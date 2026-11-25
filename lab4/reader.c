#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>


typedef struct {
    int running;
    char text[128];
} SharedData;

int main(void) {
    int fd = shm_open("/lab7_clock", O_RDWR, 0666);
    if (fd == -1) {
        printf("Start writer first\n");
        return 1;
    }

    SharedData *data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    printf("Reader PID: %ld\n", (long)getpid());

    while (data->running) {
        printf("Received: %s\n", data->text);
        sleep(1);
    }

    munmap(data, sizeof(SharedData));
    close(fd);
    return 0;
}
