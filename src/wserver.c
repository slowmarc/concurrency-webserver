#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include "queue.h"
#include "request.h"
#include "io_helper.h"

#define CONSUMERS 10

char default_root[] = ".";

sem_t is_empty;
sem_t is_full;
queue_t *queue;

void *consume(void *argument);

//
// ./wserver [-d <basedir>] [-p <portnum>] 
// 
//
int main(int argc, char *argv[]) {
  int c;
  char *root_dir = default_root;
  int port = 10000;

  while ((c = getopt(argc, argv, "d:p:")) != -1) {
    switch (c) {
      case 'd':
        root_dir = optarg;
        break;
      case 'p':
        port = atoi(optarg);
        break;
      default:
        fprintf(stderr, "usage: wserver [-d basedir] [-p port]\n");
        exit(EXIT_FAILURE);
    }
  }

  // run out of this directory
  chdir_or_die(root_dir);


  sem_init(&is_empty, 0, CONSUMERS);
  sem_init(&is_full, 0, 0);
  queue_init(&queue, CONSUMERS);

  pthread_t consumers[CONSUMERS];
  for (int index = 0; index < CONSUMERS; index++) {
    pthread_create(&consumers[index], NULL, consume, NULL);
  }

  // now, get to work
  int listen_fd = open_listen_fd_or_die(port);
  while (true) {
    struct sockaddr_in client_addr;
    int client_len = sizeof(client_addr);
    int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);

    sem_wait(&is_empty);
    queue_push(queue, conn_fd);
    sem_post(&is_full);
  }

  for (int index = 0; index < CONSUMERS; index++) {
    pthread_join(consumers[index], NULL);
  }

  sem_destroy(&is_empty);
  sem_destroy(&is_full);
  queue_destroy(&queue);

  return EXIT_SUCCESS;
}

void *consume(void *arugment) {
  while (true) {
    sem_wait(&is_full);

    int conn_fd;
    queue_pop(queue, &conn_fd);

    request_handle(conn_fd);
    close_or_die(conn_fd);

    sem_post(&is_empty);
  }

  pthread_exit(EXIT_SUCCESS);
}
