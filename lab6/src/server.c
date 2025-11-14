#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <pthread.h>
#include "calc.h"

//  ./server --port 20001 --tnum 4 &

void *ThreadFactorial(void *args) {
  struct FactorialArgs *fargs = (struct FactorialArgs *)args;
  uint64_t res = Factorial(fargs);
  return (void *)(uint64_t*)res;
}

int main(int argc, char **argv) {
  int tnum = -1;
  int port = -1;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {
        {"port", required_argument, 0, 0},
        {"tnum", required_argument, 0, 0},
        {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0:
      switch (option_index) {
      case 0:
        port = atoi(optarg);
        break;
      case 1:
        tnum = atoi(optarg);
        break;
      }
      break;
    }
  }

  if (port == -1 || tnum == -1) {
    fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
    return 1;
  }

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    fprintf(stderr, "Can not create server socket!");
    return 1;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons((uint16_t)port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
    fprintf(stderr, "Can not bind to socket!");
    return 1;
  }

  if (listen(server_fd, 128) < 0) {
    fprintf(stderr, "Could not listen on socket\n");
    return 1;
  }

  printf("Server listening at %d\n", port);
  
  bool server_running = true;


  while (server_running) {
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

    if (client_fd < 0) {
      fprintf(stderr, "Could not establish new connection\n");
      continue;
    }

    while (true) {
      uint64_t begin, end, mod;
      uint64_t buf[3];
      int read_bytes = recv(client_fd, buf, sizeof(buf), 0);

      if (!read_bytes)
        break;
      if (read_bytes < 0) {
        perror("recv");
        break;
      }

      begin = buf[0];
      end = buf[1];
      mod = buf[2];

      uint64_t total = 1;

      // распараллеливание
      pthread_t threads[tnum];
      struct FactorialArgs args[tnum];

      uint64_t chunk = (end - begin + 1) / tnum;
      uint64_t current = begin;

      for (int i = 0; i < tnum; i++) {
        args[i].begin = current;
        args[i].end = (i == tnum - 1) ? end : (current + chunk - 1);
        args[i].mod = mod;
        current = args[i].end + 1;

        pthread_create(&threads[i], NULL, ThreadFactorial, &args[i]);
      }

      for (int i = 0; i < tnum; i++) {
        uint64_t part = 0;
        pthread_join(threads[i], (void **)&part);
        total = MultModulo(total, part, mod);
      }

      send(client_fd, &total, sizeof(total), 0);
    }
    server_running = false;
    close(client_fd);
  }
  close(server_fd);
  return 0;
}
