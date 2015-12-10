/*
 * bounded/bounded.c --
 *
 * Bounded buffer (producer / consumer) problem with pthreads
 * and condition variables.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_SIZE 12

typedef struct buffer {
    unsigned int    count;
    unsigned int    data[BUFFER_SIZE];
    int             in;
    int             out;
    pthread_mutex_t mutex;
    pthread_cond_t  empty;
    pthread_cond_t  full;
    sem_t empty_flag_semaphore;
    sem_t full_flag_semaphore;
} buffer_t;

static buffer_t shared_buffer = {
    .count = 0,
    .in    = 0,
    .out   = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .empty = PTHREAD_COND_INITIALIZER,
    .full  = PTHREAD_COND_INITIALIZER
};

static int
next()
{
    static unsigned int cnt = 0;
    return ++cnt;
}

static void
check(unsigned int num)
{
    static unsigned int cnt = 0;
    if (num != ++cnt) {
        fprintf(stderr, "oops: expected %u but got %u\n", cnt, num);
    }
}

static void*
producer(void *data)
{
    buffer_t *buffer = (buffer_t *) data;
    int pvalue;
        // write proper lock mechansim
	sem_wait(&(buffer->full_flag_semaphore));
	sem_getvalue(&(buffer->full_flag_semaphore),&pvalue);
	printf("\nfull_flag_sema is %d\n", pvalue);
	pthread_mutex_lock(&(buffer->mutex));
        buffer->data[buffer->in] = next();
        buffer->in = (buffer->in + 1) % BUFFER_SIZE;
        buffer->count++;
	printf("Producer:%d\n", buffer->in);
	sleep(1);
	pthread_mutex_unlock(&(buffer->mutex));
	sem_post(&(buffer->empty_flag_semaphore));
	sem_getvalue(&(buffer->full_flag_semaphore),&pvalue);
        printf("\nafter full_flag_sema is %d\n", pvalue);

    return NULL;
}

static void*
consumer(void *data)
{
    buffer_t *buffer = (buffer_t *) data;
    int cvalue;
        sem_getvalue(&(buffer->empty_flag_semaphore),&cvalue);
        printf("\nempty_flag_sema is %d\n", cvalue);

	sem_wait(&(buffer->empty_flag_semaphore));
	sem_getvalue(&(buffer->empty_flag_semaphore),&cvalue);
        printf("\nempty_flag_sema is %d\n", cvalue);
	pthread_mutex_lock(&(buffer->mutex));
        check(buffer->data[buffer->out]);
        buffer->out = (buffer->out + 1) % BUFFER_SIZE;
        buffer->count--;
	printf("Consumer:%d\n", buffer->out);
	sleep(1);
	pthread_mutex_unlock(&(buffer->mutex));
	sem_post(&(buffer->full_flag_semaphore));
	sem_getvalue(&(buffer->empty_flag_semaphore),&cvalue);
        printf("\nafter empty_flag_sema is %d\n", cvalue);

    return NULL;
}

static int
run(int nc, int np)
{
    int i, n = nc + np;
    pthread_t thread[n];

    sem_init(&(shared_buffer.full_flag_semaphore),0,BUFFER_SIZE-shared_buffer.count);
    sem_init(&(shared_buffer.empty_flag_semaphore),0,shared_buffer.count);

    for (i = 0; i < n; i++) {
        if (pthread_create(&thread[i], NULL,
                           i < nc ? consumer : producer, &shared_buffer)) {
            fprintf(stderr, "thread creation failed\n");
            return EXIT_FAILURE;
        }
    }

    for (i = 0; i < n; i++) {
        if (thread[i]) pthread_join(thread[i], NULL);
    }

    return EXIT_SUCCESS;
}

int
main(int argc, char **argv)
{
    int c, nc = 1, np = 1;
    const char *usage
        = "Usage: bounded [-c consumers] [-p producers] [-h]\n";
   
    while ((c = getopt(argc, argv, "c:p:h")) >= 0) {
        switch (c) {
        case 'c':
            if ((nc = atoi(optarg)) <= 0) {
                fprintf(stderr, "number of consumers must be > 0\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'p':
            if ((np = atoi(optarg)) <= 0) {
                fprintf(stderr, "number of producers must be > 0\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'h':
            printf(usage);
            exit(EXIT_SUCCESS);
        }
    }

    return run(nc, np);
}
