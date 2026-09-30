#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s pattern [file ...]\n", argv[0]);
        return 2;
    }

    regex_t re;
    int r = regcomp(&re, argv[1], 0);
    if (r != 0) {
        char err[256];
        regerror(r, &re, err, sizeof(err));
        fprintf(stderr, "%s\n", err);
        return 2;
    }

    int found = 0;
    int error = 0;
    int count = argc - 2;

    if (count == 0) {
        char *line = NULL;
        size_t size = 0;
        while (getline(&line, &size, stdin) != -1) {
            if (regexec(&re, line, 0, NULL, 0) == 0) {
                fputs(line, stdout);
                found = 1;
            }
        }
        free(line);
    } else {
        for (int i = 2; i < argc; i++) {
            FILE *f;
            const char *name = argv[i];

            if (strcmp(argv[i], "-") == 0) f = stdin;
            else f = fopen(argv[i], "r");

            if (!f) {
                perror(argv[i]);
                error = 1;
                continue;
            }

            char *line = NULL;
            size_t size = 0;
            while (getline(&line, &size, f) != -1) {
                if (regexec(&re, line, 0, NULL, 0) == 0) {
                    if (count > 1) {
                        if (strcmp(argv[i], "-") == 0) printf("(standard input):");
                        else printf("%s:", name);
                    }
                    fputs(line, stdout);
                    found = 1;
                }
            }
            free(line);
            if (f != stdin) fclose(f);
        }
    }

    regfree(&re);
    if (error) return 2;
    return found ? 0 : 1;
}
