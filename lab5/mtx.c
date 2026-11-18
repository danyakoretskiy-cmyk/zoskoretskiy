#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>

#define READERS 10
#define RECORDS 10

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char shared_array[128] = "Record 0";

void *writer_thread(void *arg) {
    (void)arg;
    for (int i = 1; i <= RECORDS; i++) {
        pthread_mutex_lock(&mutex);
        snprintf(shared_array, sizeof(shared_array), "Record %d", i);
        pthread_mutex_unlock(&mutex);
        usleep(200000);
    }
    return NULL;
}

void *reader_thread(void *arg) {
    int id = *(int *)arg;
    char copy[128];
    int number;

    do {
        pthread_mutex_lock(&mutex);
        strcpy(copy, shared_array);
        sscanf(copy, "Record %d", &number);
        pthread_mutex_unlock(&mutex);

        printf("Reader %d | TID: %ld | %s\n", id, syscall(SYS_gettid), copy);
        usleep(100000);
    } while (number < RECORDS);

    return NULL;
}

int main(void) {
    pthread_t writer, readers[READERS];
    int ids[READERS];

    for (int i = 0; i < READERS; i++) {
        ids[i] = i + 1;
        if (pthread_create(&readers[i], NULL, reader_thread, &ids[i]) != 0) {
            fprintf(stderr, "Error creating reader\n");
            return 1;
        }
    }

    if (pthread_create(&writer, NULL, writer_thread, NULL) != 0) {
        fprintf(stderr, "Error creating writer\n");
        return 1;
    }

    pthread_join(writer, NULL);
    for (int i = 0; i < READERS; i++)
        pthread_join(readers[i], NULL);

    pthread_mutex_destroy(&mutex);
    return 0;
}
