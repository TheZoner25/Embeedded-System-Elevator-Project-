#include "floor_queue.h"

#define MAX_QUEUE 10

static int queue[MAX_QUEUE];
static int front = 0;
static int rear = -1;
static int count = 0;

void queue_init(void) {
    front = 0;
    rear = -1;
    count = 0;
}

void enqueue_floor(int floor) {
    if (count < MAX_QUEUE) {
        rear = (rear + 1) % MAX_QUEUE;
        queue[rear] = floor;
        count++;
    }
}

int dequeue_floor(void) {
    if (count == 0) return -1;

    int floor = queue[front];
    front = (front + 1) % MAX_QUEUE;
    count--;

    return floor;
}

int is_queue_empty(void) {
    return (count == 0);
}