#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>

mode_t apply_one(mode_t mode, const char *s, mode_t mask)
{
    int u = 0, g = 0, o = 0;
    int who = 0;
    int i = 0;

    while (s[i] == 'u' || s[i] == 'g' || s[i] == 'o' || s[i] == 'a') {
        who = 1;
        if (s[i] == 'u') u = 1;
        if (s[i] == 'g') g = 1;
        if (s[i] == 'o') o = 1;
        if (s[i] == 'a') u = g = o = 1;
        i++;
    }

    if (!who) u = g = o = 1;

    char op = s[i++];
    if (op != '+' && op != '-' && op != '=') return (mode_t)-1;

    mode_t bits = 0;
    int hasx = S_ISDIR(mode) || (mode & (S_IXUSR | S_IXGRP | S_IXOTH));

    for (; s[i]; i++) {
        char c = s[i];
        if (c == 'r') {
            if (u) bits |= S_IRUSR;
            if (g) bits |= S_IRGRP;
            if (o) bits |= S_IROTH;
        } else if (c == 'w') {
            if (u) bits |= S_IWUSR;
            if (g) bits |= S_IWGRP;
            if (o) bits |= S_IWOTH;
        } else if (c == 'x' || (c == 'X' && hasx)) {
            if (u) bits |= S_IXUSR;
            if (g) bits |= S_IXGRP;
            if (o) bits |= S_IXOTH;
        } else if (c == 's') {
            if (u) bits |= S_ISUID;
            if (g) bits |= S_ISGID;
        } else if (c == 't') {
            bits |= S_ISVTX;
        } else {
            return (mode_t)-1;
        }
    }

    if (!who && op != '=') bits &= ~mask;

    if (op == '+') mode |= bits;
    else if (op == '-') mode &= ~bits;
    else {
        mode_t clear = 0;
        if (u) clear |= S_IRWXU | S_ISUID;
        if (g) clear |= S_IRWXG | S_ISGID;
        if (o) clear |= S_IRWXO | S_ISVTX;
        mode = (mode & ~clear) | bits;
    }

    return mode;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "usage: %s mode file...\n", argv[0]);
        return 1;
    }

    char *end;
    errno = 0;
    long num = strtol(argv[1], &end, 8);
    int numeric = (*argv[1] && *end == '\0' && errno == 0 && num >= 0 && num <= 07777);

    mode_t oldmask = umask(0);
    umask(oldmask);
    int status = 0;

    for (int f = 2; f < argc; f++) {
        struct stat st;
        if (stat(argv[f], &st) != 0) {
            perror(argv[f]);
            status = 1;
            continue;
        }

        mode_t m = numeric ? (mode_t)num : st.st_mode;

        if (!numeric) {
            char *copy = strdup(argv[1]);
            if (!copy) return 1;

            char *save = NULL;
            char *part = strtok_r(copy, ",", &save);
            int bad = 0;

            while (part) {
                mode_t x = apply_one(m, part, oldmask);
                if (x == (mode_t)-1) {
                    bad = 1;
                    break;
                }
                m = x;
                part = strtok_r(NULL, ",", &save);
            }

            free(copy);
            if (bad) {
                fprintf(stderr, "bad mode: %s\n", argv[1]);
                status = 1;
                continue;
            }
        }

        if (chmod(argv[f], m) != 0) {
            perror(argv[f]);
            status = 1;
        }
    }

    return status;
}
