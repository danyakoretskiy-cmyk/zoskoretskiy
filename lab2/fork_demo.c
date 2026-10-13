#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

volatile sig_atomic_t got_int = 0;
volatile sig_atomic_t got_term = 0;

void exit_message()
{
    printf("Process %d exiting\n", getpid());
}

void int_handler(int sig)
{
    got_int = sig;
}

void term_handler(int sig)
{
    got_term = sig;
}

int main()
{
    pid_t pid;
    int status;
    struct sigaction sa;

    if (atexit(exit_message) != 0) {
        printf("atexit error\n");
        return 1;
    }

    if (signal(SIGINT, int_handler) == SIG_ERR) {
        perror("signal");
        return 1;
    }

    sa.sa_handler = term_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    pid = fork();

    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Child PID = %d, PPID = %d\n",
               getpid(), getppid());

        sleep(5);

        if (got_int)
            printf("Child received SIGINT (%d)\n", got_int);

        if (got_term)
            printf("Child received SIGTERM (%d)\n", got_term);

        return 5;
    }
    else {
        printf("Parent PID = %d, PPID = %d\n",
               getpid(), getppid());

        printf("Waiting for child\n");

        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Child exit code = %d\n",
                   WEXITSTATUS(status));
        }
    }

    return 0;
}
