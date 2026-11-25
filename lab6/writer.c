#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_KEY 24191
#define SEM_KEY 24192

volatile sig_atomic_t running = 1;

void stop(int sig)
{
    (void)sig;
    running = 0;
}

int main(void)
{
    int shmid = shmget(SHM_KEY, 128, IPC_CREAT | 0666);
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (shmid == -1 || semid == -1) {
        perror("shmget/semget");
        return 1;
    }

    char *text = shmat(shmid, NULL, 0);
    if (text == (void *)-1) {
        perror("shmat");
        return 1;
    }
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("semctl");
        return 1;
    }

    signal(SIGINT, stop);
    printf("Writer PID: %ld\n", (long)getpid());
    fflush(stdout);

    struct sembuf lock = {0, -1, 0};
    struct sembuf unlock = {0, 1, 0};
    while (running) {
        time_t now = time(NULL);
        if (semop(semid, &lock, 1) == -1) break;
        char clock_text[32];
        strftime(clock_text, sizeof(clock_text), "%H:%M:%S", localtime(&now));
        snprintf(text, 128, "Time: %s, PID: %ld", clock_text, (long)getpid());
        semop(semid, &unlock, 1);
        sleep(3);
    }

    shmdt(text);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    return 0;
}
