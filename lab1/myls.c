#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <limits.h>

struct item {
    char name[NAME_MAX + 1];
    char full[PATH_MAX];
    struct stat st;
    char linkto[PATH_MAX];
};

int cmp(const void *a, const void *b)
{
    const struct item *x = a;
    const struct item *y = b;
    return strcmp(x->name, y->name);
}

void perms(mode_t m, char s[11])
{
    s[0] = S_ISDIR(m) ? 'd' : S_ISLNK(m) ? 'l' : S_ISCHR(m) ? 'c' : S_ISBLK(m) ? 'b' : S_ISFIFO(m) ? 'p' : S_ISSOCK(m) ? 's' : '-';
    s[1] = (m & S_IRUSR) ? 'r' : '-';
    s[2] = (m & S_IWUSR) ? 'w' : '-';
    s[3] = (m & S_IXUSR) ? 'x' : '-';
    s[4] = (m & S_IRGRP) ? 'r' : '-';
    s[5] = (m & S_IWGRP) ? 'w' : '-';
    s[6] = (m & S_IXGRP) ? 'x' : '-';
    s[7] = (m & S_IROTH) ? 'r' : '-';
    s[8] = (m & S_IWOTH) ? 'w' : '-';
    s[9] = (m & S_IXOTH) ? 'x' : '-';
    s[10] = '\0';

    if (m & S_ISUID) s[3] = (m & S_IXUSR) ? 's' : 'S';
    if (m & S_ISGID) s[6] = (m & S_IXGRP) ? 's' : 'S';
    if (m & S_ISVTX) s[9] = (m & S_IXOTH) ? 't' : 'T';
}

const char *color(mode_t m)
{
    if (S_ISDIR(m)) return "\033[1;34m";
    if (S_ISLNK(m)) return "\033[1;36m";
    if (m & (S_IXUSR | S_IXGRP | S_IXOTH)) return "\033[1;32m";
    return "";
}

void print_one(struct item *x, int longmode)
{
    const char *col = color(x->st.st_mode);

    if (!longmode) {
        printf("%s%s\033[0m  ", col, x->name);
        return;
    }

    char p[11];
    perms(x->st.st_mode, p);

    struct passwd *pw = getpwuid(x->st.st_uid);
    struct group *gr = getgrgid(x->st.st_gid);
    char t[64];
    struct tm *tm = localtime(&x->st.st_mtime);
    strftime(t, sizeof(t), "%b %e %H:%M", tm);

    printf("%s %2lu %s %s %6ld %s %s%s\033[0m",
           p,
           (unsigned long)x->st.st_nlink,
           pw ? pw->pw_name : "?",
           gr ? gr->gr_name : "?",
           (long)x->st.st_size,
           t,
           col,
           x->name);

    if (S_ISLNK(x->st.st_mode)) printf(" -> %s", x->linkto);
    printf("\n");
}

int main(int argc, char **argv)
{
    int a = 0, l = 0, c;
    while ((c = getopt(argc, argv, "la")) != -1) {
        if (c == 'l') l = 1;
        else if (c == 'a') a = 1;
        else return 1;
    }

    const char *path = optind < argc ? argv[optind] : ".";
    struct stat pst;

    if (lstat(path, &pst) != 0) {
        perror(path);
        return 1;
    }

    if (!S_ISDIR(pst.st_mode)) {
        struct item x;
        memset(&x, 0, sizeof(x));
        snprintf(x.name, sizeof(x.name), "%s", path);
        snprintf(x.full, sizeof(x.full), "%s", path);
        x.st = pst;
        if (S_ISLNK(pst.st_mode)) {
            ssize_t n = readlink(path, x.linkto, sizeof(x.linkto) - 1);
            if (n >= 0) x.linkto[n] = '\0';
        }
        print_one(&x, l);
        if (!l) printf("\n");
        return 0;
    }

    DIR *d = opendir(path);
    if (!d) {
        perror(path);
        return 1;
    }

    struct item *arr = NULL;
    int n = 0, cap = 0;
    long total = 0;
    struct dirent *e;

    while ((e = readdir(d))) {
        if (!a && e->d_name[0] == '.') continue;

        if (n == cap) {
            cap = cap ? cap * 2 : 16;
            struct item *tmp = realloc(arr, cap * sizeof(*arr));
            if (!tmp) {
                free(arr);
                closedir(d);
                return 1;
            }
            arr = tmp;
        }

        memset(&arr[n], 0, sizeof(arr[n]));
        snprintf(arr[n].name, sizeof(arr[n].name), "%s", e->d_name);
        snprintf(arr[n].full, sizeof(arr[n].full), "%s/%s", path, e->d_name);

        if (lstat(arr[n].full, &arr[n].st) != 0) continue;
        total += arr[n].st.st_blocks;

        if (S_ISLNK(arr[n].st.st_mode)) {
            ssize_t k = readlink(arr[n].full, arr[n].linkto, sizeof(arr[n].linkto) - 1);
            if (k >= 0) arr[n].linkto[k] = '\0';
        }
        n++;
    }
    closedir(d);

    qsort(arr, n, sizeof(*arr), cmp);

    if (l) printf("total %ld\n", total / 2);
    for (int i = 0; i < n; i++) print_one(&arr[i], l);
    if (!l) printf("\n");

    free(arr);
    return 0;
}
