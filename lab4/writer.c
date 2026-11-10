#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string.h>

typedef struct {
    sem_t lock;
    int ready;
    int running;
    char text[128];
} SharedData;

volatile sig_atomic_t work = 1;

void stop(int sig) {
    (void)sig;
    work = 0;
}

int main(void) {
    int fd = shm_open("/lab7_clock", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        return 1;
    }

    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        printf("Передающая программа уже запущена\n");
        close(fd);
        return 1;
    }

    if (ftruncate(fd, sizeof(SharedData)) == -1) {
        perror("ftruncate");
        close(fd);
        return 1;
    }

    SharedData *data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    data->ready = 0;
    if (sem_init(&data->lock, 1, 1) == -1) {
        perror("sem_init");
        munmap(data, sizeof(SharedData));
        close(fd);
        return 1;
    }
    data->running = 1;
    data->ready = 1;

    signal(SIGINT, stop);
    signal(SIGTERM, stop);
    printf("Передача запущена. PID: %ld\n", (long)getpid());
    fflush(stdout);

    while (work) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char clock_text[32];
        strftime(clock_text, sizeof(clock_text), "%H:%M:%S", t);

        sem_wait(&data->lock);
        snprintf(data->text, sizeof(data->text), "Время: %s, PID: %ld",
                 clock_text, (long)getpid());
        sem_post(&data->lock);

        sleep(1);
    }

    sem_wait(&data->lock);
    data->running = 0;
    sem_post(&data->lock);
    munmap(data, sizeof(SharedData));
    shm_unlink("/lab7_clock");
    close(fd);
    return 0;
}
