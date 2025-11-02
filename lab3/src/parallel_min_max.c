#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>    // NEW

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

pid_t *child_pids = NULL; // NEW
int pnum_global = 0;      // NEW

// Обработчик сигнала SIGALRM — убивает все дочерние процессы
void KillChildren(int signum) { // NEW
  printf("Timeout reached! Killing child processes...\n");
  for (int i = 0; i < pnum_global; i++) {
    if (child_pids[i] > 0) {
      kill(child_pids[i], SIGKILL);
    }
  }
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;
  int timeout = 0; // NEW

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {
        {"seed", required_argument, 0, 0},
        {"array_size", required_argument, 0, 0},
        {"pnum", required_argument, 0, 0},
        {"by_files", no_argument, 0, 'f'},
        {"timeout", required_argument, 0, 0}, // NEW
        {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              fprintf(stderr, "seed must be positive\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              fprintf(stderr, "array_size must be positive\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
              fprintf(stderr, "pnum must be positive\n");
              return 1;
            }
            break;
          case 3:
            with_files = true;
            break;
          case 4: // timeout
            timeout = atoi(optarg);
            if (timeout <= 0) {
              fprintf(stderr, "timeout must be positive\n");
              return 1;
            }
            break;
          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;
      default:
        break;
    }
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed num --array_size num --pnum num [--timeout num]\n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  child_pids = calloc(pnum, sizeof(pid_t)); // NEW
  pnum_global = pnum;                       // NEW

  int **pipes = NULL;
  if (!with_files) {
    pipes = malloc(sizeof(int *) * pnum);
    for (int i = 0; i < pnum; i++) {
      pipes[i] = malloc(sizeof(int) * 2);
      if (pipe(pipes[i]) == -1) {
        perror("pipe");
        return 1;
      }
    }
  }

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid == 0) {
      // CHILD
      long long start = i * array_size / pnum;
      long long end = (i + 1) * array_size / pnum;

      struct MinMax sub_minmax = GetMinMax(array, start, end);

      if (with_files) {
        char filename[256];
        snprintf(filename, sizeof(filename), "result_%d.txt", i);
        FILE *f = fopen(filename, "w");
        fprintf(f, "%d %d\n", sub_minmax.min, sub_minmax.max);
        fclose(f);
      } else {
        close(pipes[i][0]);
        write(pipes[i][1], &sub_minmax.min, sizeof(int));
        write(pipes[i][1], &sub_minmax.max, sizeof(int));
        close(pipes[i][1]);
      }
      free(array);
      exit(0);
    } else if (child_pid > 0) {
      // PARENT
      child_pids[i] = child_pid; // NEW
      active_child_processes++;
    } else {
      perror("fork");
      return 1;
    }
  }

  // Установка таймера и обработчика, если задан timeout
  if (timeout > 0) { // NEW
    signal(SIGALRM, KillChildren);
    alarm(timeout);
  }

  // Неблокирующее ожидание завершения детей
  while (active_child_processes > 0) { // NEW
    pid_t pid = waitpid(-1, NULL, WNOHANG);
    if (pid > 0) {
      active_child_processes--;
    } else {
      usleep(100000); // немного подождать (0.1 сек)
    }
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      char filename[256];
      snprintf(filename, sizeof(filename), "result_%d.txt", i);
      FILE *f = fopen(filename, "r");
      if (f && fscanf(f, "%d %d", &min, &max) == 2) fclose(f);
    } else {
      close(pipes[i][1]);
      read(pipes[i][0], &min, sizeof(int));
      read(pipes[i][0], &max, sizeof(int));
      close(pipes[i][0]);
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);

  free(array);
  free(child_pids); // NEW
  if (pipes) {
    for (int i = 0; i < pnum; i++) free(pipes[i]);
    free(pipes);
  }

  return 0;
}
