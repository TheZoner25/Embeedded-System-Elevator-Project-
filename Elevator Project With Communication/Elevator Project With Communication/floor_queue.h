#ifndef FLOOR_QUEUE_H
#define FLOOR_QUEUE_H

#include <stdint.h>

// Function declarations
void queue_init(void);
void enqueue_floor(int floor);
int dequeue_floor(void);
int is_queue_empty(void);

#endif