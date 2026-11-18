#include <stdio.h>
#include <string.h>
#include <pthread.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>
#endif

#define READER_COUNT 10
#define MAX_RECORDS 10

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char data[80] = "Record 0";
int stopped = 0;

void delay_ms(int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    struct timespec ts = {ms / 1000, (ms % 1000) * 1000000L};
    nanosleep(&ts, NULL);
#endif
}

unsigned long get_thread_id(void)
{
#ifdef _WIN32
    return (unsigned long)GetCurrentThreadId();
#else
    return (unsigned long)syscall(SYS_gettid);
#endif
}

void *writer(void *arg)
{
    (void)arg;
    for (int i = 1; i <= MAX_RECORDS; i++) {
        pthread_mutex_lock(&mutex);
        snprintf(data, sizeof(data), "Record %d", i);
        pthread_mutex_unlock(&mutex);
        delay_ms(200);
    }
    pthread_mutex_lock(&mutex);
    stopped = 1;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *reader(void *arg)
{
    int number = *(int *)arg;
    int done;
    char copy[80];
    unsigned long tid = get_thread_id();

    do {
        pthread_mutex_lock(&mutex);
        strcpy(copy, data);
        done = stopped;
        pthread_mutex_unlock(&mutex);
        printf("Reader %2d | TID: %lu | %s\n", number, tid, copy);
        delay_ms(250);
    } while (!done);
    return NULL;
}

int main(void)
{
    pthread_t writer_id;
    pthread_t reader_ids[READER_COUNT];
    int numbers[READER_COUNT];
    int count = 0;
    int rc = pthread_create(&writer_id, NULL, writer, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(rc));
        return 1;
    }

    for (int i = 0; i < READER_COUNT; i++) {
        numbers[i] = i + 1;
        rc = pthread_create(&reader_ids[i], NULL, reader, &numbers[i]);
        if (rc != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(rc));
            break;
        }
        count++;
    }

    pthread_join(writer_id, NULL);
    for (int i = 0; i < count; i++)
        pthread_join(reader_ids[i], NULL);
    pthread_mutex_destroy(&mutex);
    return count == READER_COUNT ? 0 : 1;
}
