#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/syscall.h>

char text[100] = "Record 0";
sem_t sem;

void *writer(void *arg)
{
    (void)arg;
    for (int i = 1; i <= 10; i++) {
        sem_wait(&sem);
        snprintf(text, sizeof(text), "Record %d", i);
        sem_post(&sem);
        sleep(1);
    }
    return NULL;
}

void *reader(void *arg)
{
    (void)arg;
    for (int i = 0; i < 10; i++) {
        sem_wait(&sem);
        printf("TID: %ld | %s\n", syscall(SYS_gettid), text);
        sem_post(&sem);
        sleep(1);
    }
    return NULL;
}

int main(void)
{
    pthread_t w, r;
    if (sem_init(&sem, 0, 1) == -1) {
        perror("sem_init");
        return 1;
    }
    if (pthread_create(&w, NULL, writer, NULL) != 0 ||
        pthread_create(&r, NULL, reader, NULL) != 0) {
        fprintf(stderr, "pthread_create error\n");
        return 1;
    }
    pthread_join(w, NULL);
    pthread_join(r, NULL);
    sem_destroy(&sem);
    return 0;
}
