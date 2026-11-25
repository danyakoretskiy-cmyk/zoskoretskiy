#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_KEY 24191
#define SEM_KEY 24192

int main(void)
{
    int shmid = shmget(SHM_KEY, 128, 0666);
    int semid = semget(SEM_KEY, 1, 0666);
    if (shmid == -1 || semid == -1) {
        printf("Start writer first\n");
        return 1;
    }

    char *text = shmat(shmid, NULL, 0);
    if (text == (void *)-1) {
        perror("shmat");
        return 1;
    }

    struct sembuf lock = {0, -1, 0};
    struct sembuf unlock = {0, 1, 0};
    while (1) {
        char message[128];
        if (semop(semid, &lock, 1) == -1) break;
        snprintf(message, sizeof(message), "%s", text);
        if (semop(semid, &unlock, 1) == -1) break;

        time_t now = time(NULL);
        printf("Reader time: %.8s | PID: %ld | %s\n", ctime(&now) + 11,
               (long)getpid(), message);
        fflush(stdout);
        sleep(3);
    }

    shmdt(text);
    return 0;
}
