/**
 * @file queue.c
 * @author mustafaHTP
 * @brief circular queue that was synchronized semaphores implementation
 * @version 0.1
 * @date 2022-05-08
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_OF_THREAD 2
#define QUEUE_SIZE 20

typedef struct queue {
    int front;
    int back;
    int capacity;
    sem_t sem;
    int *data;
} queue_t;

queue_t *shared_queue;

/**
 * @brief init queue by given capacity
 *
 * @param capacity
 * @return queue_t* the new queue
 */
queue_t *init_queue(int capacity) {
    queue_t *new_queue = malloc(sizeof(queue_t));
    if (new_queue == NULL) {
        perror("malloc: ");
        exit(EXIT_FAILURE);
    }

    new_queue->front = -1;
    new_queue->back = -1;

    int r = sem_init(&new_queue->sem, 0, 1);
    if (r != 0) {
        perror("sem_init: ");
        exit(EXIT_FAILURE);
    }

    new_queue->data = malloc(sizeof(int) * capacity);
    if (new_queue->data == NULL) {
        perror("malloc: ");
        exit(EXIT_FAILURE);
    }
    new_queue->capacity = capacity;

    return new_queue;
}

void destroy_queue(queue_t *queue) {
    free(queue->data);
    sem_destroy(&queue->sem);
    free(queue);
}

int isEmpty(queue_t *queue) {
    sem_wait(&queue->sem);
    int result = queue->front == -1;
    sem_post(&queue->sem);
    return result;
}

void push(queue_t *queue, int data) {
    sem_wait(&queue->sem);

    if ((queue->front == 0 && queue->back == queue->capacity - 1) ||
        queue->back == queue->front - 1) {
        printf("queue is already full ! \n");
    }

    // queue is empty
    if (queue->front == -1) {
        queue->front = 0;
        queue->back = 0;
        queue->data[queue->back] = data;
    } else if (queue->front != 0 && queue->back == queue->capacity - 1) {
        queue->back = 0;
        queue->data[queue->back] = data;
    } else {
        ++(queue->back);
        queue->data[queue->back] = data;
    }

    sem_post(&queue->sem);
}

int pop(queue_t *queue) {
    sem_wait(&queue->sem);

    if (queue->front == -1) {
        printf("queue is already empty ! ... \n");
    }

    int popped_element = queue->data[queue->front];

    if (queue->front == queue->back) {
        queue->front = -1;
        queue->back = -1;
    } else if (queue->front == queue->capacity && queue->back == 0) {
        queue->front = 0;
    } else {
        ++(queue->front);
    }

    sem_post(&queue->sem);

    return popped_element;
}

void printQueue(queue_t *queue) {
    if (queue->front == -1) {
        printf("EMPTY... \n");
        return;
    }

    /*in normal order */
    if (queue->front <= queue->back) {
        for (int i = queue->front; i <= queue->back; i++) {
            printf("%d -> ", queue->data[i]);
        }
        puts("\n");
    } else {
        for (int i = queue->front; i < queue->capacity; i++) {
            printf("%d -> ", queue->data[i]);
        }

        for (int i = 0; i <= queue->back; i++) {
            printf("%d -> ", queue->data[i]);
        }
        puts("\n");
    }
}


void *testQueue(void *arg) {
    static int val = 0;
    for (int i = 0; i < QUEUE_SIZE / NUM_OF_THREAD; i++) {
        val += 2;
        push(shared_queue, val);
    }
    ++val;
}

int main(int argc, char const *argv[]) {
    shared_queue = init_queue(QUEUE_SIZE);
    pthread_t tids[NUM_OF_THREAD];
    for (int i = 0; i < NUM_OF_THREAD; i++) {
        pthread_create(&tids[i], NULL, testQueue, NULL);
    }

    for (int i = 0; i < NUM_OF_THREAD; i++) {
        pthread_join(tids[i], NULL);
    }

    printQueue(shared_queue);
    destroy_queue(shared_queue);

    return 0;
}
