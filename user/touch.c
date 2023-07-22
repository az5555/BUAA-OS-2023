#include <lib.h>

int main(int argc, char **argv) {
    int r;
    if (argc != 2) {
        printf("error argc\n");
        return -1;
    }
    char newpath[1024];
	get_path(argv[1], newpath);
    int fd = open(newpath, O_RDONLY);
    if (fd < 0) {
        if ((r = create(newpath, FTYPE_REG)) < 0) {
            printf("can't creat a file\n");
        }
    }
    else {
        printf("file exits\n");
    }
    close(fd);
}

