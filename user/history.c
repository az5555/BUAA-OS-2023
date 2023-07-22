#include <lib.h>

int main() {
    int fdnum;
    if ((fdnum = open(".history", 0)) < 0) {
        user_panic("filed to find .history");
    }
    struct Fd *fd = num2fd(fdnum);
    char *begin = (char*) fd2data((int) fd);
    char *endva = begin + ((struct Filefd*)fd)->f_file.f_size;
    while (begin < endva) {
        if (*begin != 0) {
            printf("%c", *begin);
        }
        else {
            printf("\n");
        }
        begin++;
    }
    close(fdnum);
}