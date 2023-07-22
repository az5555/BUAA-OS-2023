#include <lib.h>

void tree(char* path, int depth, char *name) {
    for (int i = 0; i < depth - 1; i++) {
        printf("│   ");
    }
    if (depth != 0) {
        printf("├── ");
    }
    int fd,n;
    struct File f;
	printf("\033[34m");
    printf("%s\n", name);
	printf("\033[0m");
    if ((fd = open(path, O_RDONLY)) < 0) {
		user_panic("open %s: %d", path, fd);
	}
    while ((n = readn(fd, &f, sizeof f)) == sizeof f) {
		if (f.f_name[0]) {
			if (f.f_type == FTYPE_DIR) {
                char newpath[MAXNAMELEN];
                strcpy(newpath, path);
                int l1 = strlen(path);
                int l2 = strlen(f.f_name);
                if (path[l1 - 1] == '/') {
                    l1--;
                }
                newpath[l1] = '/';
                for (int i = 0; i < l2; i++) {
                    newpath[l1 + i + 1] = f.f_name[i];
                }
                newpath[l1 + l2 + 1] = 0;
                tree(newpath, depth + 1, f.f_name);
            }
            else {
                for (int i = 0; i < depth; i++) {
                    printf("│   ");
                }
                printf("├── %s\n", f.f_name);
            }
		}
	}
    close(fd);
}

int main(int argc, char **argv) {
    if (argc == 1) {
        char path[1024];
        syscall_get_path(path);
		tree(path, 0, ".");
        printf("\n");
	} else {
		for (int i = 1; i < argc; i++) {
            char newpath[1024];
			get_path(argv[i], newpath);
			tree(newpath, 0, argv[i]);
            printf("\n");
		}
	}
	return 0;
}
