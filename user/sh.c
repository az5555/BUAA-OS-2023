#include <args.h>
#include <lib.h>

#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"

/* Overview:
 *   Parse the next token from the string at s.
 *
 * Post-Condition:
 *   Set '*p1' to the beginning of the token and '*p2' to just past the token.
 *   Return:
 *     - 0 if the end of string is reached.
 *     - '<' for < (stdin redirection).
 *     - '>' for > (stdout redirection).
 *     - '|' for | (pipe).
 *     - 'w' for a word (command, argument, or file name).
 *
 *   The buffer is modified to turn the spaces after words into zero bytes ('\0'), so that the
 *   returned token is a null-terminated string.
 */
int _gettoken(char *s, char **p1, char **p2) {
	*p1 = 0;
	*p2 = 0;
	if (s == 0) {
		return 0;
	}

	while (strchr(WHITESPACE, *s)) {
		*s++ = 0;
	}
	if (*s == 0) {
		return 0;
	}

	if (*s == '"') {
		*s = 0;
		s++;
		*p1 = s;
		while (*s != 0 && *s != '"') {
			s++;
		}
		if (*s == 0) {
			return 0;
		}
		else {
			*s = 0;
			s++;
			*p2 = s;
			return 'w';
		}

	}

	if (strchr(SYMBOLS, *s)) {
		int t = *s;
		*p1 = s;
		*s++ = 0;
		*p2 = s;
		return t;
	}

	*p1 = s;
	while (*s && !strchr(WHITESPACE SYMBOLS, *s)) {
		s++;
	}
	*p2 = s;
	return 'w';
}

int gettoken(char *s, char **p1) {
	static int c, nc;
	static char *np1, *np2;

	if (s) {
		nc = _gettoken(s, &np1, &np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2);
	return c;
}

#define MAXARGS 128

int parsecmd(char **argv, int *rightpipe) {
	int argc = 0;
	while (1) {
		char *t;
		int fd, r;
		int c = gettoken(0, &t);
		switch (c) {
		case 0:
			return argc;
		case 'w':
			if (argc >= MAXARGS) {
				debugf("too many arguments\n");
				exit();
			}
			argv[argc++] = t;
			break;
		case '<':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: < not followed by word\n");
				exit();
			}
			// Open 't' for reading, dup it onto fd 0, and then close the original fd.
			/* Exercise 6.5: Your code here. (1/3) */
			if ((r = open(t, O_RDONLY)) < 0) {
				char newpath[1024];
				get_path(t, newpath);
				if ((r = create(newpath, FTYPE_REG)) < 0) {
					user_panic("< Open err");
				}
				r = open(newpath, O_WRONLY);
			}
			fd = r;
			dup(fd, 0);
			close(fd);
			//user_panic("< redirection not implemented");

			break;
		case '>':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: > not followed by word\n");
				exit();
			}
			// Open 't' for writing, dup it onto fd 1, and then close the original fd.
			/* Exercise 6.5: Your code here. (2/3) */
			if ((r = open(t, O_WRONLY)) < 0) {
				char newpath[1024];
				get_path(t, newpath);
				if ((r = create(newpath, FTYPE_REG)) < 0) {
					user_panic("> Open err");
				}
				r = open(newpath, O_WRONLY);
			}
			fd = r;
			dup(fd, 1);
			close(fd);
			//user_panic("> redirection not implemented");

			break;
		case '|':;
			/*
			 * First, allocate a pipe.
			 * Then fork, set '*rightpipe' to the returned child envid or zero.
			 * The child runs the right side of the pipe:
			 * - dup the read end of the pipe onto 0
			 * - close the read end of the pipe
			 * - close the write end of the pipe
			 * - and 'return parsecmd(argv, rightpipe)' again, to parse the rest of the
			 *   command line.
			 * The parent runs the left side of the pipe:
			 * - dup the write end of the pipe onto 1
			 * - close the write end of the pipe
			 * - close the read end of the pipe
			 * - and 'return argc', to execute the left of the pipeline.
			 */
			int p[2];
			/* Exercise 6.5: Your code here. (3/3) */
			pipe(p);
			if ((*rightpipe = fork()) == 0) {
                    dup(p[0], 0);
                    close(p[0]);
                    close(p[1]);
                    return parsecmd(argv, rightpipe);
                } else {
                    dup(p[1], 1);
                    close(p[1]);
                    close(p[0]);
                    return argc;
                }
			user_panic("| not implemented");

			break;
		case ';':
			if ((*rightpipe = fork()) == 0) {
                    return argc;
                }
			else {
					wait(*rightpipe);
                    return parsecmd(argv, rightpipe);
                }
			break;
		case '&':
			if ((*rightpipe = fork()) == 0) {
                    return argc;
                }
			else {
                    return parsecmd(argv, rightpipe);
                }
		}
	}

	return argc;
}

void runcmd(char *s) {
	gettoken(s, 0);

	char *argv[MAXARGS];
	int rightpipe = 0;
	int argc = parsecmd(argv, &rightpipe);
	if (argc == 0) {
		return;
	}
	argv[argc] = 0;

	int child = spawn(argv[0], argv);
	close_all();
	if (child >= 0) {
		wait(child);
	} else {
		debugf("spawn %s: %d\n", argv[0], child);
	}
	if (rightpipe) {
		wait(rightpipe);
	}
	exit();
}

void readline(char *buf, u_int n) {
	int r;
	char c1;
	char c2;
	char c3;
	int num = 0;
	for (int i = 0; i < n;) {
		if ((r = read(0, &c1, 1)) != 1) {
			if (r < 0) {
				debugf("read error: %d\n", r);
			}
			exit();
		}
		if (c1 == '\b' || c1 == 0x7f) {
			for (int j = i - 1; j < num - 1; j++) {
				buf[j] = buf[j + 1];
			}
			if (i > 0) {
				i -= 1;
				buf[--num] = 0;
				printf("\b");
				for(int j = i; j < num; j++) {
					printf("%c", buf[j]);
				}
				printf(" ");
				for (int j = 0; j <= num - i; j++) {
					printf("\b");
				}
			}		
		}
		else if (c1 == 27) {
			if ((r = read(0, &c2, 1)) < 0) {
				debugf("read err");
				exit();
			}
			if ((r = read(0, &c3, 1)) < 0) {
				debugf("read err");
				exit();
			}
			if (c2 == '[' && c3 == 'D') {
				//left
				if (i > 0) {
					i--;
				}
				else {
					printf(" ");
				}
			}
			if (c2 == '[' && c3 == 'C') {
				//right
				if (i < num) {
					i++;
				}
				else {
					printf("\b");
				}
			}
			if (c2 == '[' && c3 == 'B') {
				//down
				if (next_history(&buf) > 0) {
					if (i != 0) {
						for (int j = 0; j <= num - i; j++) {
							printf(" ");
						}
						for (int j = 0; j <= num; j++) {
							printf("\b \b");
						}
					}
					else {
						printf(" \b");
					}
					printf("%s",buf);
					i = num = strlen(buf);
				}
			}
			if (c2 == '[' && c3 == 'A') {
				//up
				printf("\x1b[B");
				if (last_history(&buf) > 0) {
					if (i != 0) {
						for (int j = 0; j <= num - i; j++) {
							printf(" ");
						}
						for (int j = 0; j <= num; j++) {
							printf("\b \b");
						}
					}
					else {
						printf(" \b");
					}
					printf("%s",buf);
					i = num = strlen(buf);
				}
			}
		}
		else if (c1 == '\r' || c1 == '\n') {
			buf[num] = 0;
			add_history(buf);
			return;
		}
		else {
			for (int j = num; j > i - 1; j--) {
				buf[j] = buf[j - 1];
			}
			num++;
			buf[i++] = c1;
			for(int j = i; j < num; j++) {
				printf("%c", buf[j]);
			}
			buf[num] = 0;
			for (int j = 0; j < num - i; j++) {
				printf("\b");
			}
		}
	}
	debugf("line too long\n");
	while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n') {
		;
	}
	buf[0] = 0;
}

char buf[1024];

void usage(void) {
	debugf("usage: sh [-dix] [command-file]\n");
	exit();
}

int main(int argc, char **argv) {
	int r;
	int interactive = iscons(0);
	int echocmds = 0;
	init_history();
	debugf("........^z96212234321259uv8+++3767!annoooov19521463677777666652^++^-8375776666666666\n");
	debugf("...+!34i*--+++-++++-++---+-8-+iu--+++++++++++---.-a8224666666648-..35667676666666666\n");
	debugf("o..+..--++++++++++++++++++++++-8^++++++++++++++++++++--z52466663z+466787666666666666\n");
	debugf("z937u^..-+++++++++++++++++++++++^++++++++++++++++++++++++-*7246587556676766666657766\n");
	debugf("-+-.!o.-+++++++++++++++++++++++++++++++++++-++++++++++++++++-n3266666666666666666564\n");
	debugf(".n5n..++++++++++++++++++++++++++++++++++++++z+-++++++++++++++-+^82666666666666656661\n");
	debugf("n...++++++--*^++++++++++++++++++++++++++++++-*9^-++++++++++++++++-83656666666666663!\n");
	debugf("--+++++++-zz--++++++++++++++++++++++++++++++++-v9-+++++++++++++++++^726666666766643.\n");
	debugf("++++++--zi-++++++++++++-o^++++++++++++++++++++++-7o++++++++++++++++++o246666666653+.\n");
	debugf("++++++^5*-^++++++++++++z+++++++++++++++++++++++++-ui-++++++++++++++++++7366666747^..\n");
	debugf("++++-v6++++++++++++-+-u^+++++++++++++++++++++++++++o7-+++++++++++++++++-a267643*++*7\n");
	debugf("++++a9+++++++++++++-+ao++++++++++++++++++++++++++++-^7-++++++++++++++++++*243664n++-\n");
	debugf("++-v8+++++++++++++++*1-++++++++++++++++++++++++++++++^9-+++++++++++++++++++25663u+++\n");
	debugf("+-*5-++++++++++++--.7--++++++++++++++++++++++++++++++-na-+++++++++++++++++++34a+++++\n");
	debugf("+-3--++++++++++++++un++++++++++++++++++++++++++++++++++1^+++++++++++++++++++o3++++++\n");
	debugf("-8a-++++++++++++++-5-++++++++++o+++++++++++++++++++++++^a++++++++++a++-n*++++19+++++\n");
	debugf("*4-+++++++++++++++zv-++++++++++8*-++++++++++++++++++++++z+-+++++++-3^-i6-++++-6o++++\n");
	debugf("4a+++++++++++++++.7++++++++++-7o9+++++++++++++++++++++++*a--++++++^+77o-+++++-n8-+++\n");
	debugf("2+++++++++++++++++!++++++++-+18.7o+++++++++++++++++++++++^9-++++++++++++++++-+-8*+++\n");
	debugf("9+++++++++++++++-ov+++++++++v5-.z8+++++++++++++++++++++^v.z6+++++++++++++++++++vz+++\n");
	debugf("v-+++++++++++++++v^+++++zv^n3-..-4^++++++++++++++++++++oz..u7i*+++++++++++++++++u+++\n");
	debugf("+-+++++++++++++++z++++++++i28512233668u+++++++++++++o6321241i7-++++++++++++++++-v+++\n");
	debugf("-++++++++++++++++^+++++--76-......4o++-++++++++++++++-o7-....!7-++++++++++++++++*^++\n");
	debugf("+++++++++++++++++++++++n3v...-^***v4-+++++++++++++++++5*..-...!7++++++++++++++++*n++\n");
	debugf("++++++++++++++++++++-*31z77io+*na1u3u++++++++++++++--781zno*z9767+++++++++++++++*v++\n");
	debugf("++++++++++++++++++-*437n+i37a^-.....3^++++++++++++++69v---.-*i4ui5-^++++++++++++*a-+\n");
	debugf("+++++++++++++++++o73vn48^...+ono..--a6-+^++++++++--3z-.^u1*-..-.z52-++++++++++++^i-+\n");
	debugf("+++++++++++++++++va15^...z361--n95^.-91-uz+++++++v3*.841+-a85+-..^n2++++++++++++*!-+\n");
	debugf("+++++++++++++++++o^^.---571iiii!ii3o--6!16*++++o41.-861iuuiii3o....-5-++++++++++*9-+\n");
	debugf("+++++++++++++++++^a---.i51iiiiii1i!3-.+.-n3+.v21-..+3iii11111!5+...z*+++++++++++o9-+\n");
	debugf(".-++++++++++++++++7---.961iiii1i1i13^.--..845*.....^2i1iii1ii13o---3++++++++++++n!-+\n");
	debugf("...-+++++++++++++-3+.--z41iiiiiiii13*..............^21ii1iiii13o-.i!+++++++++++-u6^+\n");
	debugf("25-.+++++++++++++.6v--.-4!11ii111186-............-..581111ii178---3^+++++++++++-1a5i\n");
	debugf("972.-+++++++++++++i9.---+48i111iu85+--.............--481111138+--*u-+++++++++++-9n.-\n");
	debugf("996u.-+++++++++++^o2------n445537+..................-.o5334a-+--.no++++++++++++-6*..\n");
	debugf("8873+.++++++++++++-2+---------------................----.--------^2-++++++++++++4+.^\n");
	debugf("79976.-+++++++++++.4v------------------......---.-.--------------.2^+++++++++++o7-.^\n");
	debugf("98862-.+++++++++++-!9------------------....v1uauv----------------u5-++++++++++-ai..^\n");
	debugf("98811o.-++++++++++-n2------------------..-zi!!ao+---------------12^++++++++++--8v.++\n");
	debugf("99248!..+++++++++++-3o---------------.....-...-.....-..-----.-v3u+++++++++++-+^5+.8^\n");
	debugf("935^a5..+++++++++++-7i------------..................-.----.n44u9+++++++++++++-z9..-4\n");
	debugf("1a^+o3-.+++++++++++^v3v*-..-------.................-.-z7338^++^4++++++++++++++6o...+\n");
	debugf("++^+*4-.++++++++++++-41zzu532459un+--....--+oa!53134!avvvzvu28^3*++++++++++++*7-....\n");
	debugf("++^+o4-ni.++++++++++-u5n9ui68888887777778866nvvvnvvvzvvvvvvzzva2z-++++++++++-8n.....\n");
	debugf("++++v6..u6++++++++++++3un6z.!46778877778887a6vvvvnvnvvvvvvvzzv52a-++++++++++*7.....8\n");
	debugf("+++-iv.-+^38++++++++++a3nv576v+n63478999935+367aovvvvvvzzzzva2n8i-++++n+++--6^...12v\n");
	debugf("+++.5+.-+++^46*-+++++^-68zz388855uo.^*^n*.+4788944avzvzzzzv48++!!-++++v2^+-uz.^35i!1\n");
	debugf("+-+*z.-+-+n5ii62u++++++^3uv94888888654446687888886739nzzzz2o-+-i9+++-ua-9+^7-.1!9637\n");
	debugf("---5-.++-87!i!!!!34v-+++n1zv368878888777887777788778738zzvzzi56!8-+-5n..oz8+-4258i!!\n");
	debugf("++i*.++-79!1!6ii!i!!43v++n1va278888888878877777787877773avvvvvv7!.!2^9.*.1^.i9i9999!\n");
	debugf("-na..+-i75458ii6iii!ii555*^2v9397778878888788888778788864zzvzzv543!*-5-^o-.!!!!9!!!9\n");
	debugf("u1..+-u5i!i111i9651u98vznz6u6n558788888777888888888878872zvzzzuavz4--5n.u9!9!9!9!!!9\n");
	debugf("*.-++o41i111iiiiii9!6vzvvvvvziz27888877888888877777778863zvzzzzvz!i.-15-o99!!!9!99!9\n");
	debugf(".^---4!i11iiiii1i1i4avzvvvvvzvv93888778888888888888887848vzvzvvvz4^++o3^+!!!!!!!!!!9\n");
	debugf("6a++88iiiiiiii1iiiu131vvvzzvvzzz477877777778888888888762zvvvvzvv9i-++^3z.i!!i9!!9999\n");
	debugf("*7+a6iiiiiiiii1auuu1i66zzvzvvvvvu2777777787788887888882ivvzzzzza4^--+-4i.a9!!!!!!ii!\n");
	debugf("-8a31iiiiiiiiii11111i1i2!vvvvzvzn5477788888888778888869nvvvvvzv76n--++31.z9!!!!!!!!!\n");
	debugf("+^21iiiiiiii1iii11i!1ii9!31zvvvvvz288888888888888888867vvvvvzvu416*++n5-.97i!!!!!!!!\n");
	debugf("+.78iiiii!iiiiii1i1i11i6i181avvvvv658878887778888887866vvvzvvv4!ii3^-8u...^638i!!!!!\n");
	debugf("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	debugf("::                                                         ::\n");
	debugf("::                  Welcom To SOS Shell                    ::\n");
	debugf("::                                                         ::\n");
	debugf(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	ARGBEGIN {
	case 'i':
		interactive = 1;
		break;
	case 'x':
		echocmds = 1;
		break;
	default:
		usage();
	}
	ARGEND

	if (argc > 1) {
		usage();
	}
	if (argc == 1) {
		close(0);
		if ((r = open(argv[1], O_RDONLY)) < 0) {
			user_panic("open %s: %d", argv[1], r);
		}
		user_assert(r == 0);
	}
	for (;;) {
		if (interactive) {
			char path[1024];
			syscall_get_path(path);
			if (path[1] == 0) {
				printf("\n$ ");
			}
			else {
				path[strlen(path) - 1] = 0;
				printf("\033[34m");
				printf("\n%s ", path);
				printf("\033[0m");
				printf("$ ");
			}
		}
		readline(buf, sizeof buf);

		if (buf[0] == '#') {
			continue;
		}
		if (echocmds) {
			printf("# %s\n", buf);
		}
		if ((r = fork()) < 0) {
			user_panic("fork: %d", r);
		}
		if (r == 0) {
			runcmd(buf);
			exit();
		} else {
			wait(r);
		}
	}
	return 0;
}
