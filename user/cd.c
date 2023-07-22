#include <lib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("error argc\n");
    }
    else {
        int r;
        char newpath[1024];
        get_path(argv[1], newpath);
        if((r = open(newpath, O_RDONLY)) < 0) {
            printf("%s not exist\n",newpath);
        }
        else {
            syscall_set_path(newpath);
        }
    }
}