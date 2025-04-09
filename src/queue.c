#include "queue.h"

#include <stdio.h>
#include <stdlib.h>

int queue_init(queue_t **queue, int max_size) {
  if (*queue) {
    fprintf(stderr, "Queue has already been initialized!\n");
    return EXIT_FAILURE;
  }
  if (max_size <= 0) {
    fprintf(stderr, "Can't initialize a queue with a non positive max size!\n");
    return EXIT_FAILURE;
  }

  *queue = malloc(sizeof(queue_t));
  if(!(*queue)) {
    fprintf(stderr, "Failed to allocate memory for the queue!\n");
    return EXIT_FAILURE;
  }

  (*queue) -> head = -1;
  (*queue) -> tail = -1;

  (*queue) -> size = 0;
  (*queue) -> max_size = max_size;

  (*queue) -> nodes = malloc(max_size * sizeof(int));
  if (!((*queue) -> nodes)) {
    fprintf(stderr, "Failed to allocate memory for queue's node array\n");
    free(*queue);

    return EXIT_FAILURE;
  }

  if (pthread_mutex_init(&((*queue) -> mutex), NULL) != EXIT_SUCCESS) {
    fprintf(stderr, "Failed to initialize queue's mutex!\n");
    free(*queue);
    free((*queue) -> nodes);

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int queue_push(queue_t *queue, int fd) {
  if (!queue) {
    fprintf(stderr, "Can't push into an uninitialized queue!\n");
    return EXIT_FAILURE;
  }

  pthread_mutex_lock(&(queue -> mutex));
  if (queue -> size == queue -> max_size) {
    fprintf(stderr, "Can't push into a full queue!\n");
    pthread_mutex_unlock(&(queue -> mutex));

    return EXIT_FAILURE;
  }

  if (queue -> head == -1) queue -> head = 0;
  queue -> tail = (queue -> tail + 1) % queue -> max_size;
  queue -> nodes[queue -> tail] = fd;
  queue -> size = queue -> size + 1;

  pthread_mutex_unlock(&(queue -> mutex));
  return EXIT_SUCCESS;
}

int queue_pop(queue_t *queue, int *fd) {
  if (!queue) {
    fprintf(stderr, "Can't pop out of an uninitialized queue!\n");
    return EXIT_FAILURE;
  }

  pthread_mutex_lock(&(queue -> mutex));
  if (queue -> size == 0) {
    fprintf(stderr, "Can't pop out of an empty queue!\n");
    pthread_mutex_unlock(&(queue -> mutex));

    return EXIT_FAILURE;
  }

  if (fd) *fd = queue -> nodes[queue -> head];
  queue -> head = (queue -> head + 1) % queue -> max_size;
  queue -> size = queue -> size - 1;

  pthread_mutex_unlock(&(queue -> mutex));
  return EXIT_SUCCESS;
}

int queue_destroy(queue_t **queue) {
  if (!(*queue)) {
    fprintf(stderr, "Queue has already been destroyed!\n");
    return EXIT_FAILURE;
  }

  pthread_mutex_destroy(&((*queue) -> mutex));
  free((*queue) -> nodes);
  free(*queue);
  *queue = NULL;

  return EXIT_SUCCESS;
}
