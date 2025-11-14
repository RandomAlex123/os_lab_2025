#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include "calc.h"

//  ./client --k 4 --mod 10 --servers servers.txt 

struct Server {
  char ip[255];
  int port;
};

struct Task {
  uint64_t begin, end, mod;
  struct Server server;
  uint64_t result;
};

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  *val = i;
  return true;
}

void *RunTask(void *arg) {
  struct Task *t = (struct Task *)arg;

  struct hostent *hostname = gethostbyname(t->server.ip);
  if (!hostname) {
    perror("gethostbyname");
    t->result = 1;
    return NULL;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(t->server.port);
  server_addr.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

  int sck = socket(AF_INET, SOCK_STREAM, 0);
  connect(sck, (struct sockaddr *)&server_addr, sizeof(server_addr));

  uint64_t buf[3] = {t->begin, t->end, t->mod};
  send(sck, buf, sizeof(buf), 0);

  recv(sck, &t->result, sizeof(t->result), 0);
  close(sck);

  return NULL;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers_path[255] = {'\0'};

  while (true) {
    int current_optind = optind ? optind : 1;
    static struct option options[] = {
        {"k", required_argument, 0, 0},
        {"mod", required_argument, 0, 0},
        {"servers", required_argument, 0, 0},
        {0, 0, 0, 0}};
    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);
    if (c == -1)
      break;
    if (c == 0) {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        break;
      case 2:
        memcpy(servers_path, optarg, strlen(optarg));
        break;
      }
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers_path)) {
    printf("Usage: --k N --mod M --servers file\n");
    return 1;
  }

  // читаем список серверов
  FILE *f = fopen(servers_path, "r");
  if (!f) {
    perror("servers file");
    return 1;
  }

  struct Server servers[128];
  int servers_num = 0;

  while (!feof(f)) {
    char line[255];
    if (!fgets(line, sizeof(line), f))
      break;

    char *colon = strchr(line, ':');
    if (!colon)
      continue;

    *colon = '\0';
    strcpy(servers[servers_num].ip, line);
    servers[servers_num].port = atoi(colon + 1);
    servers_num++;
  }
  fclose(f);

  // разбиваем факториал по серверам
  pthread_t threads[servers_num];
  struct Task tasks[servers_num];

  uint64_t chunk = k / servers_num;
  uint64_t current = 1;

  for (int i = 0; i < servers_num; i++) {
    tasks[i].begin = current;
    tasks[i].end = (i == servers_num - 1) ? k : (current + chunk - 1);
    current = tasks[i].end + 1;

    tasks[i].mod = mod;
    tasks[i].server = servers[i];
    tasks[i].result = 1;

    pthread_create(&threads[i], NULL, RunTask, &tasks[i]);
  }

  uint64_t total = 1;

  for (int i = 0; i < servers_num; i++) {
    pthread_join(threads[i], NULL);
    total = MultModulo(total, tasks[i].result, mod);
  }

  printf("Final answer: %llu\n", total);

  return 0;
}
