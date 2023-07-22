#include <lib.h>

int main(int argc, char **argv) {
    if (argc != 1) {
        printf("err argc\n");
    }
    char path[1024];
    syscall_get_path(path);
	if (path[1] == 0) {
		printf("/ \n");
	}
	else {
		path[strlen(path) - 1] = 0;
		printf("%s\n", path);
	}
    return 0;
}