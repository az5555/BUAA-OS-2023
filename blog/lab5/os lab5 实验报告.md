# os lab5 实验报告

### 思考题

> **Thinking 5.1** 如果通过 kseg0 读写设备，那么对于设备的写入会缓存到 Cache 中。这是一种错误的行为，在实际编写代码的时候这么做会引发不可预知的问题。请思考：这么做这会引发什么问题？对于不同种类的设备（如我们提到的串口设备和 IDE 磁盘）的操作会有差异吗？可以从缓存的性质和缓存更新的策略来考虑。

- 如果外设更新数据时，cpu写入外设的数据只存在于Cache之中；此时，因为只有缓存块将要被新进入的数据取代时，缓存数据才会被写入内存，Cache中的对应部分数据只能在外设完成自身更新后在写入外设；在这情况下就会出现错误。
- 串口设备的读写频率远远高于IDE磁盘，因此，串口设备出错率会更高。

> **Thinking 5.2** 查找代码中的相关定义，试回答一个磁盘块中最多能存储多少个文件控制块？一个目录下最多能有多少个文件？我们的文件系统支持的单个文件最大为多大？ 

- 文件控制块`File` 结构体的定义如下：

  ```c
  struct File {
  	char f_name[MAXNAMELEN]; // filename
  	uint32_t f_size;	 // file size in bytes
  	uint32_t f_type;	 // file type
  	uint32_t f_direct[NDIRECT];
  	uint32_t f_indirect;
  
  	struct File *f_dir; // the pointer to the dir where this file is in, valid only in memory.
  	char f_pad[BY2FILE - MAXNAMELEN - (3 + NDIRECT) * 4 - sizeof(void *)];
  } __attribute__((aligned(4), packed));
  ```

- 可以看到，`File` 结构体中有这样一个`char f_pad[BY2FILE - MAXNAMELEN - (3 + NDIRECT) * 4 - sizeof(void *)];`将结构体的大小对齐为`BY2FILE`为256B，而一个磁盘块大小为4KB，因此可以存储$ 4KB / 256B = 16 $个结构体。

- 一个`File`结构体最多指向1024个磁盘块，其中可以存储$1024*16 = 16384$个文件，目录文件也使用`File`结构体，因此一个目录下最多有16384个文件。

- 一个`File`结构体最多指向1024个磁盘块，其中可以存储最大$1024*4KB = 4M$的文件。

> **Thinking 5.3** 请思考，在满足磁盘块缓存的设计的前提下，我们实验使用的内核支持的最大磁盘大小是多少？

```c
 #define DISKMAX 0x40000000
```

- 由宏定义可知，内核支持的最大磁盘大小为1GB。

> **Thinking 5.4** 在本实验中，fs/serv.h、user/include/fs.h 等文件中出现了许多宏定义， 试列举你认为较为重要的宏定义，同时进行解释，并描述其主要应用之处。

- `user/include/fs.h`

  ```c
  #define BY2BLK BY2PG //磁盘块大小
  
  struct File {
  	char f_name[MAXNAMELEN]; // 文件名
  	uint32_t f_size;	 // 文件大小
  	uint32_t f_type;	 // 文件类型
  	uint32_t f_direct[NDIRECT]; //直接指针
  	uint32_t f_indirect; // 间接指针
  
  	struct File *f_dir; // 指向目录文件控制块的指针
  	char f_pad[BY2FILE - MAXNAMELEN - (3 + NDIRECT) * 4 - sizeof(void *)];
  } __attribute__((aligned(4), packed));//文件控制块结构体
  
  
  struct Super {
  	uint32_t s_magic;   // 魔数
  	uint32_t s_nblocks; // 磁盘总块数
  	struct File s_root; // 根目录控制块
  };// 魔块
  
  ```

- `fs/serv.h` 

  ```c
  #define DISKMAP 0x10000000 //DISK起始地址
  #define DISKMAX 0x40000000 //DISK大小
  #define BY2SECT 512	 //扇区比特数
  ```

> **Thinking 5.5** 在 Lab4“系统调用与 fork”的实验中我们实现了极为重要的 fork 函数。那么 fork 前后的父子进程是否会共享文件描述符和定位指针呢？请在完成上述练习的基础上编写一个程序进行验证。 

```c
#include "lib.h"
void fork_test() {
    int fd = open("/test", O_RDWR);
    char buf[128];
    user_bzero(buf, 128);
    read(fd, buf, 10);
    debugf("buf is %s\n", buf);
    if(fork() == 0) {
        //son
        char bufSon[128];
        user_bzero(bufSon, 128);
        read(fd, bufSon, 10);
        debugf("son's buf is %s\n", bufSon);
    }else {
        char bufFa[128];
        user_bzero(bufFa, 128);
        read(fd, bufFa, 10);
        debugf("father's buf is %s\n", bufFa);
    }
} 
```

- test的文件内容为

  ```
  helloworldIamyourfatIamyourson
  ```

- 最终输出为

  ```
  helloworld
  Iamyourfat
  Iamyourson
  ```

- 可以看待子进程和父进程共享文章，因为open操作时，系统会调用`serve_open`函数，在函数中调用系统调用`ipc_send(envid, 0, o->o_ff, PTE_D | PTE_LIBRARY);`其中包括了`PTE_LIBRARY`权限位，可以将文件描述符共享，而文件重定位指针存在与文件描述符内。

> **Thinking 5.6** 请解释 File, Fd, Filefd 结构体及其各个域的作用。比如各个结构体会在哪些过程中被使用，是否对应磁盘上的物理实体还是单纯的内存数据等。说明形式自定，要 求简洁明了，可大致勾勒出文件系统数据结构与物理实体的对应关系与设计框架。

```c
struct File {
	char f_name[MAXNAMELEN]; // 文件名
	uint32_t f_size;	 // 文件大小
	uint32_t f_type;	 // 文件类型
	uint32_t f_direct[NDIRECT]; //直接指针
	uint32_t f_indirect; // 间接指针

	struct File *f_dir; // 指向目录文件控制块的指针
	char f_pad[BY2FILE - MAXNAMELEN - (3 + NDIRECT) * 4 - sizeof(void *)];
} __attribute__((aligned(4), packed));//文件控制块结构体，用于存储文件的各个信息，对文件的各种操作都会用到，在磁盘上存										 //在实体。

struct Fd {
	u_int fd_dev_id;//文件对应设备的ID
	u_int fd_offset;//设备读写的偏移
	u_int fd_omode;//允许用户进程对文件的操作权限
};//文件描述符，只存在于内存不存在于磁盘之中
//以下为fd_omode的可取值：
#define O_RDONLY    0x0000      /* 只读 */
#define O_WRONLY    0x0001      /* 只写 */
#define O_RDWR      0x0002      /* 可读写 */
#define O_ACCMODE   0x0003      /* 上述权限的掩码 */

#define O_CREAT     0x0100      /* 文件不存在时创建 */
#define O_TRUNC     0x0200      /* 长度设置为0 */
#define O_EXCL      0x0400      /* 已经存在就抛出异常 */
#define O_MKDIR     0x0800      /*创建目录*/


struct Filefd {
	struct Fd f_fd;//文件描述符
	u_int f_fileid;//文件编号
	struct File f_file;//对应的文件控制块
};//内存数据，不存在于磁盘，便于文件系统查看。
```

> **Thinking 5.7** 图5.7中有多种不同形式的箭头，请解释这些不同箭头的差别，并思考我们的操作系统是如何实现对应类型的进程间通信的。 
>
> ![屏幕截图 2023-05-19 181130](C:\Users\az\Desktop\project\os\expr\lab5\屏幕截图 2023-05-19 181130.png)

- 圆点实线黑箭头：表示创建进程。
- 实线黑箭头：表示同步消息，发送请求。
- 虚线箭头： 表示接受到请求后，返回消息。
- 我们通过IPC实现文件系统供其他线程调用，用户线程利用IPC向文件管理系统发出请求。对用户进程而言，可以调用不同的`fsipc_*`来完成不同的文件操作。

### 难点分析

##### Exercise 5.1

难点在于认识到`kseg1`区域虚拟地址和物理地址的转换，`kseg1`区域和`kseg0`区域内核虚拟地址计算方式不同，不能用之前的宏定义完成物理地址到虚拟地址的转换。此外，在写入和读取时，要保证每一个字节都在要求的范围内。同时，我们需要检查虚拟地址和物理地址，确保两者都符合条件。

##### Exercise 5.3

难点在于学会IDE外设的使用，寄存器映射如下。

| 偏移         | 效果                                                         | 数据位宽 |
| ------------ | ------------------------------------------------------------ | -------- |
| 0x0000       | 写：设置下一次读写操作时的磁盘镜像偏移的字节数               | 4字节    |
| 0x0008       | 写：设置高 32 位的偏移的字节数                               | 4 字节   |
| 0x0010       | 写：设置下一次读写操作的磁盘编号                             | 4 字节   |
| 0x0020       | 写：开始一次读写操作（写 0 表示读操作，1 表示写操作）        | 4字节    |
| 0x0030       | 读：获取上一次操作的状态返回值（读 0 表示失败，非 0 则 表示成功） | 4字节    |
| 0x4000-0x41f | 读/写：512 字节的读写缓存                                    |          |

IDE外设要先将数据写入缓存区或者磁盘先写入缓存区。知道这一机制就很容易完成了。

##### Exercise 5.5

首先要了解`Thinking 5.6`中的三种结构体以及其发挥的作用。首先可以利用`nblk = dirf->f_size / BY2BLK;`计算文件由多少磁盘块组成，根据文件控制块的指针结构得到对应的磁盘块。注意间接指针指向的位置有10个未被使用的指针，间接指针可以通过`bno = ((int *)(disk[dirf->f_indirect].data))[i];`得到磁盘块，然后遍历每一磁盘块中的每一个文件控制块，找到一个未被使用的文件控制块，实现如下。

![屏幕截图 2023-05-20 010318](C:\Users\az\Desktop\project\os\expr\lab5\屏幕截图 2023-05-20 010318.png)

```c
for (struct File *f = blk; f < blk + FILE2BLK; ++f) {
			if (f->f_name[0] == '\0') {
				return f;
			}
		}
```

##### Exercise 5.6

调用系统调用`syscall_mem_alloc`和`syscall_mem_unmap`完成虚拟地址的映射。

##### Exercise 5.8

这个exercise按照注释完成难度不大，但是我发现了如下代码，对此感到疑惑。

```c
struct Fd *fd;
ffd = (struct Filefd*) fd;
```

可以发现，这里直接将一个指向`Fd`结构体的指针转变为了一个指向`Filefd`结构体的指针，这个操作是否可行？在查阅`Fd`结构体时发现

```c
struct Filefd {
	struct Fd f_fd;//文件描述符
	u_int f_fileid;//文件编号
	struct File f_file;//对应的文件控制块
}
```

`Filefd`结构体中包括了一个`Fd`结构体，而且位置为第一个，因此这个操作是可行的。

### 体会与感悟

- 本次lab相比于上一次的多线程难度降低了很多，无论是调试还是代码的编写都简单了很多。但是我发现，虽然实验中要求我们完成的代码量不多，但是依旧有很多代码值得我们去研究，要充分理解文件系统中几个结构体的作用和定义，充分理解外设的输入和输出。
- 这次代码的实现会用到之前所写的系统调用例如`syscall_mem_alloc`和`syscall_mem_unmap`以及之前写好的IPC，这要求我们对之前知识掌握的足够充分，体现了知识的关联性。
- 本次代码也用到了很多已经写好的函数，结构体，宏定义例如`Dev`结构体，可以自己去阅读一下，会有所收获。
