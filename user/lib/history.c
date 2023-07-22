#include <fs.h>
#include <lib.h>

static int p;
static int fdnum;
static char* va;
static int num;
static struct Fd *fd;
static int beginva;
static char tmp[1024];
static int endva;

void init_history() {
    p = 0;
    num = 0;
    int r;
    if ((r = create(".history",  FTYPE_REG)) < 0) {
        user_panic("failed to create .history");
    }
    if ((fdnum = open(".history", O_RDWR)) < 0) {
        user_panic("filed to find .history");
    }
    fd = num2fd(fdnum);
    return;
}

void add_history(char* cmd) {
    if (*cmd == 0) {
        return;
    }
    int r;
    if ((r = write(fdnum, cmd, strlen(cmd) + 1)) < 0) {
        user_panic("filed to writ .history");
    }
    num++;
    p = num;
    beginva = (int) fd2data((int) fd);
    endva = beginva + ((struct Filefd*)fd)->f_file.f_size;
    va = (char*)(endva) - 1;
    tmp[0] = 0;
}

int next_history(char** str) {
    char *pp = *str;
    if (p == num) {
        return -1;
    }
    if (p == num - 1) {
        strcpy(*str, tmp);
        return 1;
    }
    p++;
    va++;
    while (*va != 0) {
        va++;
    }
    char* tm = va + 1;
    while (*tm != 0) {
        *pp = *tm;
        tm++;
        pp++;
    }
    *pp = 0;
    return 1;
}

int last_history(char** str) {
    char *pp = *str;
    if (p == 0) {
        return -1;
    }
    if (p == num) {
        strcpy(tmp, *str);
    }
    va--;
    p--;
    while (*va != 0) {
        va--;
    }
    char* tm = va;
    va++;
    while (*va != 0) {
        *pp = *va;
        va++;
        pp++;
    }
    va = tm;
    *pp = 0;
    return 1;
}
