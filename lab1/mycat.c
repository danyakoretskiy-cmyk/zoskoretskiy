#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int n = 0, b = 0, E = 0;
    int c;

    while ((c = getopt(argc, argv, "nbE")) != -1) {
        if (c == 'n') n = 1;
        else if (c == 'b') b = 1;
        else if (c == 'E') E = 1;
        else return 1;
    }

    if (b) n = 0;

    int line = 1;
    int files = argc - optind;

    if (files == 0) {
        char buf[4096];
        while (fgets(buf, sizeof(buf), stdin)) {
            int blank = (buf[0] == '\n' && buf[1] == '\0');
            if (b && !blank) printf("%6d\t", line++);
            else if (n) printf("%6d\t", line++);

            if (E) {
                char *p = strchr(buf, '\n');
                if (p) {
                    *p = '\0';
                    printf("%s$\n", buf);
                } else {
                    printf("%s", buf);
                }
            } else {
                printf("%s", buf);
            }
        }
        return 0;
    }

    for (int i = optind; i < argc; i++) {
        FILE *f;
        if (strcmp(argv[i], "-") == 0) f = stdin;
        else f = fopen(argv[i], "r");

        if (!f) {
            perror(argv[i]);
            continue;
        }

        char buf[4096];
        while (fgets(buf, sizeof(buf), f)) {
            int blank = (buf[0] == '\n' && buf[1] == '\0');
            if (b && !blank) printf("%6d\t", line++);
            else if (n) printf("%6d\t", line++);

            if (E) {
                char *p = strchr(buf, '\n');
                if (p) {
                    *p = '\0';
                    printf("%s$\n", buf);
                } else {
                    printf("%s", buf);
                }
            } else {
                printf("%s", buf);
            }
        }

        if (f != stdin) fclose(f);
    }

    return 0;
}
