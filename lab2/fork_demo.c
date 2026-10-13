#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

volatile sig_atomic_t got_int = 0;
volatile sig_atomic_t got_term = 0;

void exit_message(void)
{
    printf("Process %d: atexit works, exiting\n", (int)getpid());
}

void int_handler(int sig)
{
    got_int = sig;
}

void term_handler(int sig)
{
    got_term = sig;
}

void check_signals(void)
{
    if (got_int) {
        printf("Process %d received SIGINT (%d)\n", (int)getpid(), got_int);
        got_int = 0;
    }
    if (got_term) {
        printf("Process %d received SIGTERM (%d)\n", (int)getpid(), got_term);
        got_term = 0;
    }
}

int main(void)
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

    setbuf(stdout, NULL);
    pid = fork();

    if (pid == -1) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        printf("Child: PID = %d, PPID = %d\n", (int)getpid(), (int)getppid());
        printf("Child is working for 5 seconds\n");
        for (int i = 0; i < 5; i++) {
            sleep(1);
            check_signals();
        }
        return 5;
    } else {
        printf("Parent: PID = %d, PPID = %d\n", (int)getpid(), (int)getppid());
        printf("Parent is waiting for child %d\n", (int)pid);
        while (waitpid(pid, &status, 0) == -1) {
            if (errno == EINTR) {
                check_signals();
                continue;
            }
            perror("waitpid");
            return 1;
        }
        check_signals();
        if (WIFEXITED(status))
            printf("Child exit code: %d\n", WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("Child killed by signal: %d\n", WTERMSIG(status));
    }

    return 0;
}
