#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

typedef struct {
    int running;
    char text[128];
} SharedData;

int main(void) {
    int fd = shm_open("/lab7_clock", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
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

    data->running = 1;
    printf("Writer PID: %ld\n", (long)getpid());

    for (int i = 0; i < 10; i++) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char clock_text[32];
        strftime(clock_text, sizeof(clock_text), "%H:%M:%S", t);

        snprintf(data->text, sizeof(data->text), "Time: %s, PID: %ld",
                 clock_text, (long)getpid());
        sleep(1);
    }

    data->running = 0;
    munmap(data, sizeof(SharedData));
    close(fd);
    shm_unlink("/lab7_clock");
    return 0;
}
