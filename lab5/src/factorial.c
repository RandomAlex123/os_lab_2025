#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

long long k = 0;
int pnum = 0;
long long mod = 0;

pthread_mutex_t mutex;
long long result = 1;

void* thread_factorial(void* arg) {
    long long start = ((long long*)arg)[0];
    long long end   = ((long long*)arg)[1];
    free(arg);

    long long local_result = 1;

    for (long long i = start; i <= end; i++) {
        local_result = (local_result * i) % mod;
    }

    // Добавляем результат в общий факториал
    pthread_mutex_lock(&mutex);
    result = (result * local_result) % mod;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 5) {
        printf("Usage: %s -k <num> --pnum=<threads> --mod=<mod>\n", argv[0]);
        return 1;
    }

    // Парсим аргументы
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-k") == 0) {
            k = atoll(argv[++i]);
        } else if (strncmp(argv[i], "--pnum=", 7) == 0) {
            pnum = atoi(argv[i] + 7);
        } else if (strncmp(argv[i], "--mod=", 6) == 0) {
            mod = atoll(argv[i] + 6);
        }
    }

    if (k <= 0 || pnum <= 0 || mod <= 0) {
        printf("Invalid arguments.\n");
        return 1;
    }

    pthread_t threads[pnum];
    pthread_mutex_init(&mutex, NULL);

    long long chunk = k / pnum;

    for (int i = 0; i < pnum; i++) {
        long long* range = malloc(2 * sizeof(long long));

        range[0] = i * chunk + 1;
        range[1] = (i == pnum - 1) ? k : (i + 1) * chunk;

        pthread_create(&threads[i], NULL, thread_factorial, range);
    }

    for (int i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);

    printf("Result: %lld\n", result);
    return 0;
}
