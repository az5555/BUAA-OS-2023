# 操作系统lab6实验报告

### 思考题

> **Thinking 6.1** 示例代码中，父进程操作管道的写端，子进程操作管道的读端。如果现在想 让父进程作为“读者”，代码应当如何修改？

```c
#include <stdlib.h>
#include <unistd.h>

int fildes[2];
char buf[100];
int status;

int main(){
	status = pipe(fildes);
	if (status == -1 ) {
		printf("error\n");
	}
	switch (fork()) {
		case -1:
			break;
		case 0: 
            close(fildes[0]); 
			write(fildes[1], "Hello world\n", 12); 
			close(fildes[1]);
			exit(EXIT_SUCCESS);
		default: 
            syscall_yield();
			close(fildes[1]); 
			read(fildes[0], buf, 100); 
			printf("child-process read:%s",buf); 
			close(fildes[0]);
			exit(EXIT_SUCCESS);
		}
}
```

- 将`case 0 `  和 `default`  的内容切换注意父进程要调用`syscall_yield()` 切换进程，确保子进程先完成写入。

> **Thinking 6.2** 上面这种不同步修改 pp_ref 而导致的进程竞争问题在 user/lib/fd.c 中 的 dup 函数中也存在。请结合代码模仿上述情景，分析一下我们的 dup 函数中为什么会出 现预想之外的情况？

- `dup`函数的作用是将一个文件描述符中的的内容映射到另一个文件描述符中，可以发现，原本的`dup`函数先对文件描述符`Fd`进行映射在对相应的内容进行映射。

- 对于如下代码

  ```c
   if(fork() == 0) {
       dup(p[1], 1);
       read(p[0],buf,sizeof buf);
   }
   else {
       dup(p[0], 0);
       write(p[1],"Hello",5);
   }
  ```

  若再dup中先对文件描述符进行映射，先将`p[0]`映射再将`pipe`映射，若在两者间出现中断，就可能会出现`pageref(p[0]) = pageref(pipe)`程序会认定管道关闭，提前结束。例如，`p[0]`映射为1，此时`pipe`为2，若先映射`p[0]`，映射数加一此时有，`pageref(p[0]) == pageref(pipe)`程序在读写时会误判。

> **Thinking 6.3** 阅读上述材料并思考：为什么系统调用一定是原子操作呢？如果你觉得不是所有的系统调用都是原子操作，请给出反例。希望能结合相关代码进行分析说明。

- 在我们的MOS操作系统中，所有的系统调用都是原子操作，因为在运行系统调用时，我们在汇编代码中将外部中断进行屏蔽。

> **Thinking 6.4** 仔细阅读上面这段话，并思考下列问题 
>
> - 按照上述说法控制 pipe_close 中 fd 和 pipe unmap 的顺序，是否可以解决上述场景的进程竞争问题？给出你的分析过程。 
> -  我们只分析了 close 时的情形，在 fd.c 中有一个 dup 函数，用于复制文件描述符。 试想，如果要复制的文件描述符指向一个管道，那么是否会出现与 close 类似的问 题？请模仿上述材料写写你的理解。

- 可以解决上诉问题，因为正常情况下`pipe`的映射数是一定高于`p[0]`和`p[1]` 的，在正确的map和unmap顺序下，`pipe`映射数一定大于`p[0]`和`p[1]` ，不可能出现`pipe == p[0]`的情况。因此对管道是否关闭的误判。
- 会出现和close类似的情况，就是上面的思考题Thinking 6.2，在对管道进行映射的时候，若先对文件描述符映射，会有可能错误的判断管道是否关闭。例如，`p[0]`映射为1，此时`pipe`为2，若先映射`p[0]`，映射数加一此时有，`pageref(p[0]) == pageref(pipe)`程序在读写时会误判。

> Thinking 6.5 思考以下三个问题。 
>
> -  认真回看 Lab5 文件系统相关代码，弄清打开文件的过程。
> -  回顾 Lab1 与 Lab3，思考如何读取并加载 ELF 文件。 
> -  在 Lab1 中我们介绍了 data text bss 段及它们的含义，data 段存放初始化过的全 局变量，bss 段存放未初始化的全局变量。关于 memsize 和 filesize ，我们在 Note 1.3.4中也解释了它们的含义与特点。关于 Note 1.3.4，注意其中关于“bss 段并不在文 件中占数据”表述的含义。回顾 Lab3 并思考：elf_load_seg() 和 load_icode_mapper() 函数是如何确保加载 ELF 文件时，bss 段数据被正确加载进虚拟内存空间。bss 段 在 ELF 中并不占空间，但 ELF 加载进内存后，bss 段的数据占据了空间，并且初始 值都是 0。请回顾 elf_load_seg() 和 load_icode_mapper() 的实现，思考这一点 是如何实现的？
>
> 下面给出一些对于上述问题的提示，以便大家更好地把握加载内核进程和加载用户进程的 区别与联系，类比完成 spawn 函数。关于第一个问题，在 Lab3 中我们创建进程，并且通过 ENV_CREATE(...) 在内核态加 载了初始进程，而我们的 spawn 函数则是通过和文件系统交互，取得文件描述块，进而找 到 ELF 在“硬盘”中的位置，进而读取。 关于第二个问题，各位已经在 Lab3 中填写了 load_icode 函数，实现了 ELF 可执行 文件中读取数据并加载到内存空间，其中通过调用 elf_load_seg 函数来加载各个程序段。 在 Lab3 中我们要填写 load_icode_mapper 回调函数，在内核态下加载 ELF 数据到内存 空间；相应地，在 Lab6 中 spawn 函数也需要在用户态下使用系统调用为 ELF 数据分配空间。

- `.bss`在ELF中不占据空间，但是在载入内存时，我们会将其对于的空间全部写成0。我们对ELF的载入通过`elf_load_seg`函数是实现，其对`.bss`的载入源码如下。

  ```c
   while (i < sgsize) {
  	if ((r = map_page(data, va + i, 0, perm, NULL, MIN(bin_size - i, BY2PG))) != 0) {
  		return r;
  	}
  	i += BY2PG;
  }
  ```

  可以看到，我们将i以上`sgsize`以下的空间全部赋值为0，实现了`.bss`全部为0载入内存。

> **Thinking 6.6** 通过阅读代码空白段的注释我们知道，将标准输入或输出定向到文件，需要 我们将其 dup 到 0 或 1 号文件描述符（fd）。那么问题来了：在哪步，0 和 1 被“安排”为 标准输入和标准输出？请分析代码执行流程，给出答案。 

- 我们在`user/init.c`实现

  ```c
  // stdin should be 0, because no file descriptors are open yet
  if ((r = opencons()) != 0) {
  	user_panic("opencons: %d", r);
  }
  // stdout
  if ((r = dup(0, 1)) < 0) {
  	user_panic("dup: %d", r);
  }
  
  
  int opencons(void) {
  	int r;
  	struct Fd *fd;
  
  	if ((r = fd_alloc(&fd)) < 0) {
  		return r;
  	}
  	if ((r = syscall_mem_alloc(0, fd, PTE_D | PTE_LIBRARY)) < 0) {
  		return r;
  	}
  	fd->fd_dev_id = devcons.dev_id;
  	fd->fd_omode = O_RDWR;
  	return fd2num(fd);
  }
  ```

  

> **Thinking 6.7** 在 shell 中执行的命令分为内置命令和外部命令。在执行内置命令时 shell 不需要 fork 一个子 shell，如 Linux 系统中的 cd 命令。在执行外部命令时 shell 需要 fork 一个子 shell，然后子 shell 去执行这条命令。 据此判断，在 MOS 中我们用到的 shell 命令是内置命令还是外部命令？请思考为什么 Linux 的 cd 命令是内部命令而不是外部命令？

- 在MOS中使用的命令都是外部命令，在使用时都会`fork`一个子shell，在`spawn`中有创建子进程的行为

  ```c
  if ((child = syscall_exofork()) < 0) {
  	r = child;
  	goto err;
  }
  ```

- Linux中cd命令是直接通过调用shell内置函数来实现，而不是载入ELF文件，这样可以节省运行时间，加快反应速度。

> Thinking 6.8 在你的 shell 中输入命令 ls.b | cat.b > motd。
>
> - 请问你可以在你的 shell 中观察到几次 spawn ？分别对应哪个进程？
> - 请问你可以在你的 shell 中观察到几次进程销毁？分别对应哪个进程？

- 终端输入输出如下

  ```shell
  $ ls.b | cat.b > motd
  [00005004] pipecreate 
  [00006805] destroying 00006805
  [00006805] free env 00006805
  i am killed ... 
  [00005806] destroying 00005806
  [00005806] free env 00005806
  i am killed ... 
  [00006003] destroying 00006003
  [00006003] free env 00006003
  i am killed ... 
  [00005004] destroying 00005004
  [00005004] free env 00005004
  i am killed ... 
  ```

  - 可以看到两次`spawn`分别对应6805和5806，对于`ls.b`和`cat.b`两个命令的进程。
  - 可以看到四次进程销毁6805、5806、6003、5004分别对应`ls.b`和`cat.b`两个命令的进程，管道`fork`的子进程，`mian`中`fork`出的子进程。

### 难点分析

##### Exercise 6.1

管道的实现是本次实验的实现重点和难点之一，管道也是后面shell实现的基础。而管道实现的重点就在本次exercise，难点主要在于管道的读和写，我们的读和写是两个进程，具体的关系如下图所示。在实际情况下，我们会有一个进程读取管道，一个进程写管道，在我们的![屏幕截图 2023-05-30 152708](C:\Users\az\Desktop\project\os\expr\lab6\屏幕截图 2023-05-30 152708.png)

`pipe`结构体中，我们有一个数表述已经读取的字节的位置，一个数表述已经写入的字节位置。由于缓存区的大小有限，在读取时，

```c
struct Pipe {
	u_int p_rpos;	       // read position
	u_int p_wpos;	       // write position
	u_char p_buf[BY2PIPE]; // data buffer
};
```

我们可能还未写入所读字节，此时需要`p->p_rpos == p->p_wpos`来判断管道是否为空，在写入时，需要`p->p_wpos - p->p_rpos == BY2PIPE`来判断缓冲区是否填满。当管道为空或者管道填满时，我们通过调换进程轮询来切换进程。对于管道是否关闭的判断，我们利用`fd_ref == pipe_ref`来判断。这里可能会出现一些线程切换所导致的问题，上面思考题已经讨论。

##### Exercise 5.4 & Exercise 5.5

shell的实现很复杂，但是课程组已经帮我们写好了大部分内容；shell的启动流程如下图所示。可以看到启动过程涉及很多进程和文件。

![屏幕截图 2023-05-30 171152](C:\Users\az\Desktop\project\os\expr\lab6\屏幕截图 2023-05-30 171152.png)

我们的两个exercise只需要完成`spawn`函数和`parsecmd`函数；其中，主要难点在于`spawn`函数的实现，在这个函数中，我们要打开一个文件，并且将ELF文件载入到对内存之中，同时还涉及到线程的创建，可以说是之前几次lab知识的总结了。根据注释，前面打开文件，建立新进程，建立新堆栈的过程不难。难点在于ELF文件的载入，其中结构复杂。具体的实现如下

```c
ELF_FOREACH_PHDR_OFF (ph_off, ehdr) {
	if ((r = seek(fd, ph_off)) < 0) {
		goto err1;
	}
	if ((r = readn(fd, elfbuf, ehdr->e_phentsize)) < 0) {
		goto err1;
	}
	Elf32_Phdr *ph = (Elf32_Phdr *)elfbuf;
	if (ph->p_type == PT_LOAD) {
		void *bin;
		if ((r = read_map(fd, ph->p_offset, &bin)) < 0) {
			goto err1;
		}
		if ((r = elf_load_seg(ph, bin, spawn_mapper, &child)) < 0) {
			goto err1;
		}
	}
}

//ELF_FOREACH_PHDR_OFF 是一个宏定义，不了解其情况无法理解上述函数，其定义如下
#define ELF_FOREACH_PHDR_OFF(ph_off, ehdr)                                                         \
	(ph_off) = (ehdr)->e_phoff;                                                                \
	for (int _ph_idx = 0; _ph_idx < (ehdr)->e_phnum; ++_ph_idx, (ph_off) += (ehdr)->e_phentsize)
//可以看到，我们在宏定义中已经实现了循环遍历，因此在下面不需要再写一遍循环。

//spawn_mapper是一个函数指针，用于elf_load_seg内部函数的实现
```

对于ELF的载入，课程组已经帮我们完成了`spawn_mapper`函数，虽然完成任务不需要涉及shell的启动和实现的全流程，但是我们还是要有所了解，该函数的定义如下，可以看到，主要通过系统调用在用户态实现页表的创建与映射。

```c
static int spawn_mapper(void *data, u_long va, size_t offset, u_int perm, const void *src,
			size_t len) {
	u_int child_id = *(u_int *)data;
	try(syscall_mem_alloc(child_id, (void *)va, perm));
	if (src != NULL) {
		int r = syscall_mem_map(child_id, (void *)va, 0, (void *)UTEMP, perm | PTE_D);
		if (r) {
			syscall_mem_unmap(child_id, (void *)va);
			return r;
		}
		memcpy((void *)(UTEMP + offset), src, len);
		return syscall_mem_unmap(0, (void *)UTEMP);
	}
	return 0;
}
```

### 体会与感悟

总的来说，本次lab虽然难点较高但是任务量并不大；在本次lab中，我们会用到之前数次lab的知识，对整个操作系统也有了粗略但是全面的认识。在课程组的帮助下，我们一步一步完成了系统启动初始化，内存管理，系统调用，进程，外设，和文件系统等，虽然理解的不是那么深刻，但是也逐渐了解到了操作系统底层的一些设计。当我们完成最后一个函数，de完最后一个bug；自己一步步完成的操作系统加入shell页面时，我还是很有成就感的。

至此，操作系统的7次lab已经全部完成，在整个操作系统的课程中，我了解到了操作系统如何与硬件交涉，如何为程序编写提供运行环境，粗略了解到了makeFile和shell脚本的编写，完善了自己的c语言知识体系，也接触到了很多之前从未接触过的程序实现方式。操作系统是一个庞大且复杂的程序，我们实现的MOS操作系统也只是运行在虚拟机上的简易操作系统，而且MOS操作系统中的大部分内容也不是由我们自己完成，我们仅仅在指导书和注释的帮助下完成了一小部分。操作系统庞大且复杂，还有很多内容等待着我们挖掘。
