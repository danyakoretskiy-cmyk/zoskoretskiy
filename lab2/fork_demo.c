#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int got_int = 0;
int got_term = 0;

void exit_message()
{
    printf("Process exiting\n");
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

    atexit(exit_message);

    signal(SIGINT, int_handler);

    sa.sa_handler = term_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);

    pid = fork();

    if (pid == 0) {
        printf("Child PID = %d\n", getpid());

        sleep(5);

        if (got_int)
            printf("Received SIGINT\n");

        if (got_term)
            printf("Received SIGTERM\n");

        return 5;
    }
    else {
        printf("Parent PID = %d\n", getpid());

        wait(&status);

        printf("Child exit code = %d\n", status);
    }

    return 0;
}
