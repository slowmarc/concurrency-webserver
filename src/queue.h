#ifndef QUEUE_H
#define QUEUE_H

#include "pthread.h"

/* Please don't access any members of this structure directly */
typedef struct {
  int head;
  int tail;

  int size;
  int max_size;
  int *nodes;

  pthread_mutex_t mutex;
} queue_t;

int queue_init(queue_t **, int);

int queue_push(queue_t *, int);

int queue_pop(queue_t *, int *);

int queue_destroy(queue_t **);

#endif
