#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

volatile sig_atomic_t got_int = 0;
volatile sig_atomic_t got_term = 0;

void exit_message()
{
    printf("Exiting process %d\n", getpid());
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
    int i;
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

    setbuf(stdout, NULL);

    pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Child process\n");
        printf("PID = %d\n", getpid());
        printf("PPID = %d\n", getppid());

        for (i = 0; i < 5; i++) {
            sleep(1);

            if (got_int != 0) {
                printf("SIGINT received: %d\n", got_int);
                got_int = 0;
            }

            if (got_term != 0) {
                printf("SIGTERM received: %d\n", got_term);
                got_term = 0;
            }
        }

        return 5;
    }
    else {
        printf("Parent process\n");
        printf("PID = %d\n", getpid());
        printf("PPID = %d\n", getppid());

        printf("Waiting for child %d\n", pid);

        while (waitpid(pid, &status, 0) == -1) {
            if (errno == EINTR) {
                if (got_int != 0) {
                    printf("SIGINT received: %d\n", got_int);
                    got_int = 0;
                }

                if (got_term != 0) {
                    printf("SIGTERM received: %d\n", got_term);
                    got_term = 0;
                }

                continue;
            }

            perror("waitpid");
            return 1;
        }

        if (got_int != 0)
            printf("SIGINT received: %d\n", got_int);

        if (got_term != 0)
            printf("SIGTERM received: %d\n", got_term);

        if (WIFEXITED(status)) {
            printf("Child exit code: %d\n",
                   WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
            printf("Child killed by signal: %d\n",
                   WTERMSIG(status));
        }
    }

    return 0;
}
