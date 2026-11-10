#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
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

int main(void) {
    int fd = shm_open("/lab7_clock", O_RDWR, 0666);
    if (fd == -1) {
        printf("Сначала запустите передающую программу\n");
        return 1;
    }

    SharedData *data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    printf("Принимающая программа запущена. PID: %ld\n", (long)getpid());
    while (!data->ready) {
        usleep(100000);
    }

    while (1) {
        char message[128];
        int running;
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char clock_text[32];
        strftime(clock_text, sizeof(clock_text), "%H:%M:%S", t);

        sem_wait(&data->lock);
        strcpy(message, data->text);
        running = data->running;
        sem_post(&data->lock);

        if (!running) {
            printf("Передающая программа завершилась\n");
            break;
        }

        printf("Мое время: %s, мой PID: %ld | Получено: %s\n",
               clock_text, (long)getpid(), message);
        fflush(stdout);
        sleep(1);
    }

    munmap(data, sizeof(SharedData));
    close(fd);
    return 0;
}
