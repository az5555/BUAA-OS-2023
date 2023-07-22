# 思考题

> ![image-20230313190610427](C:\Users\az\AppData\Roaming\Typora\typora-user-images\image-20230313190610427.png)

使用 `man objdump` 命令，可以查询向 objdump 传入的参数的含义。

编写一个简单的c语言程序test.c

```c
#include<stdio.h>
int main() {
	int a = 1;
	int b = 2;
	return a+b;
}
```



先使用 `gcc -E test.c > E.txt` 将test.c程序进行预处理，并将结果导入E.txt文件中。

```c
/* 文件太长，这里只展示前5行 */
# 0 "test.c"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
```

然后使用 `gcc -c test.c  test.o` 将test.c编译生成test.o文件。

```
gcc: warning: test.o: linker input file unused because linking not done
```

这里会警告输出没有连接。

随后用 `objdump -DS test.o > o.txt` 命令将text.o反编译，并将结果输入到o.txt文件中。其中 -D表示反编译，-S表示将源码和反编译码一同输出。

```
test.o：     文件格式 elf64-x86-64


Disassembly of section .text:

0000000000000000 <main>:
   0:	f3 0f 1e fa          	endbr64 
   4:	55                   	push   %rbp
   5:	48 89 e5             	mov    %rsp,%rbp
   8:	c7 45 f8 01 00 00 00 	movl   $0x1,-0x8(%rbp)
   f:	c7 45 fc 02 00 00 00 	movl   $0x2,-0x4(%rbp)
  16:	8b 55 f8             	mov    -0x8(%rbp),%edx
  19:	8b 45 fc             	mov    -0x4(%rbp),%eax
  1c:	01 d0                	add    %edx,%eax
  1e:	5d                   	pop    %rbp
  1f:	c3                   	ret    

Disassembly of section .comment:

0000000000000000 <.comment>:
   0:	00 47 43             	add    %al,0x43(%rdi)
   3:	43 3a 20             	rex.XB cmp (%r8),%spl
   6:	28 55 62             	sub    %dl,0x62(%rbp)
   9:	75 6e                	jne    79 <main+0x79>
   b:	74 75                	je     82 <main+0x82>
   d:	20 31                	and    %dh,(%rcx)
   f:	31 2e                	xor    %ebp,(%rsi)
  11:	33 2e                	xor    (%rsi),%ebp
  13:	30 2d 31 75 62 75    	xor    %ch,0x75627531(%rip)        # 7562754a <main+0x7562754a>
  19:	6e                   	outsb  %ds:(%rsi),(%dx)
  1a:	74 75                	je     91 <main+0x91>
  1c:	31 7e 32             	xor    %edi,0x32(%rsi)
  1f:	32 2e                	xor    (%rsi),%ch
  21:	30 34 29             	xor    %dh,(%rcx,%rbp,1)
  24:	20 31                	and    %dh,(%rcx)
  26:	31 2e                	xor    %ebp,(%rsi)
  28:	33 2e                	xor    (%rsi),%ebp
  2a:	30 00                	xor    %al,(%rax)

Disassembly of section .note.gnu.property:

0000000000000000 <.note.gnu.property>:
   0:	04 00                	add    $0x0,%al
   2:	00 00                	add    %al,(%rax)
   4:	10 00                	adc    %al,(%rax)
   6:	00 00                	add    %al,(%rax)
   8:	05 00 00 00 47       	add    $0x47000000,%eax
   d:	4e 55                	rex.WRX push %rbp
   f:	00 02                	add    %al,(%rdx)
  11:	00 00                	add    %al,(%rax)
  13:	c0 04 00 00          	rolb   $0x0,(%rax,%rax,1)
  17:	00 03                	add    %al,(%rbx)
  19:	00 00                	add    %al,(%rax)
  1b:	00 00                	add    %al,(%rax)
  1d:	00 00                	add    %al,(%rax)
	...

Disassembly of section .eh_frame:

0000000000000000 <.eh_frame>:
   0:	14 00                	adc    $0x0,%al
   2:	00 00                	add    %al,(%rax)
   4:	00 00                	add    %al,(%rax)
   6:	00 00                	add    %al,(%rax)
   8:	01 7a 52             	add    %edi,0x52(%rdx)
   b:	00 01                	add    %al,(%rcx)
   d:	78 10                	js     1f <.eh_frame+0x1f>
   f:	01 1b                	add    %ebx,(%rbx)
  11:	0c 07                	or     $0x7,%al
  13:	08 90 01 00 00 1c    	or     %dl,0x1c000001(%rax)
  19:	00 00                	add    %al,(%rax)
  1b:	00 1c 00             	add    %bl,(%rax,%rax,1)
  1e:	00 00                	add    %al,(%rax)
  20:	00 00                	add    %al,(%rax)
  22:	00 00                	add    %al,(%rax)
  24:	20 00                	and    %al,(%rax)
  26:	00 00                	add    %al,(%rax)
  28:	00 45 0e             	add    %al,0xe(%rbp)
  2b:	10 86 02 43 0d 06    	adc    %al,0x60d4302(%rsi)
  31:	57                   	push   %rdi
  32:	0c 07                	or     $0x7,%al
  34:	08 00                	or     %al,(%rax)
	...
```

然后使用 `gcc -o test.o test.c` 随后用 `objdump -DS test.o > o.txt` 命令将text.o反编译，并将结果输入到o.txt文件中。其中 -D表示反编译，-S表示将源码和反编译码一同输出。

```
/*文件太长，这里只展示前面部分*/
test.o：     文件格式 elf64-x86-64


Disassembly of section .interp:

0000000000000318 <.interp>:
 318:	2f                   	(bad)  
 319:	6c                   	insb   (%dx),%es:(%rdi)
 31a:	69 62 36 34 2f 6c 64 	imul   $0x646c2f34,0x36(%rdx),%esp
 321:	2d 6c 69 6e 75       	sub    $0x756e696c,%eax
 326:	78 2d                	js     355 <__abi_tag-0x37>
 328:	78 38                	js     362 <__abi_tag-0x2a>
 32a:	36 2d 36 34 2e 73    	ss sub $0x732e3436,%eax
 330:	6f                   	outsl  %ds:(%rsi),(%dx)
 331:	2e 32 00             	cs xor (%rax),%al

Disassembly of section .note.gnu.property:

0000000000000338 <.note.gnu.property>:
 338:	04 00                	add    $0x0,%al
 33a:	00 00                	add    %al,(%rax)
 33c:	20 00                	and    %al,(%rax)
 33e:	00 00                	add    %al,(%rax)
 340:	05 00 00 00 47       	add    $0x47000000,%eax
 345:	4e 55                	rex.WRX push %rbp
 347:	00 02                	add    %al,(%rdx)
 349:	00 00                	add    %al,(%rax)
 34b:	c0 04 00 00          	rolb   $0x0,(%rax,%rax,1)
 34f:	00 03                	add    %al,(%rbx)
 351:	00 00                	add    %al,(%rax)
 353:	00 00                	add    %al,(%rax)
 355:	00 00                	add    %al,(%rax)
 357:	00 02                	add    %al,(%rdx)
 359:	80 00 c0             	addb   $0xc0,(%rax)
 35c:	04 00                	add    $0x0,%al
 35e:	00 00                	add    %al,(%rax)
 360:	01 00                	add    %eax,(%rax)
 362:	00 00                	add    %al,(%rax)
 364:	00 00                	add    %al,(%rax)
	...
```

> ![image-20230313190646214](C:\Users\az\AppData\Roaming\Typora\typora-user-images\image-20230313190646214.png)

先前往target文件夹，输入命令 `../tools/readelf/readelf mos` 可以在中断看到输出

```
0:0x0
1:0x80010000
2:0x800124c0
3:0x800124d8
4:0x800124f0
5:0x0
6:0x0
7:0x0
8:0x0
9:0x0
10:0x0
11:0x0
12:0x0
13:0x0
14:0x0
15:0x0
16:0x0
```

前往readelf文件夹，输入 `readelf -h hello` 可在终端看到。

```
ELF 头：
  Magic：   7f 45 4c 46 01 01 01 03 00 00 00 00 00 00 00 00 
  类别:                              ELF32
  数据:                              2 补码，小端序 (little endian)
  Version:                           1 (current)
  OS/ABI:                            UNIX - GNU
  ABI 版本:                          0
  类型:                              EXEC (可执行文件)
  系统架构:                          Intel 80386
  版本:                              0x1
  入口点地址：               0x8049600
  程序头起点：          52 (bytes into file)
  Start of section headers:          746252 (bytes into file)
  标志：             0x0
  Size of this header:               52 (bytes)
  Size of program headers:           32 (bytes)
  Number of program headers:         8
  Size of section headers:           40 (bytes)
  Number of section headers:         35
  Section header string table index: 34
```

输入  `readelf -h readelf` 可得

```
ELF 头：
  Magic：   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  类别:                              ELF64
  数据:                              2 补码，小端序 (little endian)
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI 版本:                          0
  类型:                              DYN (Position-Independent Executable file)
  系统架构:                          Advanced Micro Devices X86-64
  版本:                              0x1
  入口点地址：               0x1180
  程序头起点：          64 (bytes into file)
  Start of section headers:          14488 (bytes into file)
  标志：             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         13
  Size of section headers:           64 (bytes)
  Number of section headers:         31
  Section header string table index: 30
```

readelf不能读取readelf因为两者的类别有所不同，一个是ELF32,另一个是ELF64。

![image-20230313190705783](C:\Users\az\AppData\Roaming\Typora\typora-user-images\image-20230313190705783.png)



因为我们加载程序时，不是真的在MIPS硬件上运行，而是通过GXemul虚拟机，支持直接加载ELF格式的内核，提供了bootloader的引导，在运行第一行代码前，我们已经有了一个正常运行的环境和内存，因此可以直接按照内存布局图放置。

# 实验难点展示

本次实验比较简单，主要考察c语言代码的理解和阅读能力。

#### Exercise 1.1

> 主要考察对指针的理解，在ELF文件头中，有所有的段表头和节表头的信息。理解结构体内的信息并且熟练c语言指针的用法和ELF的结构的话很容易做出来。
>
> ![image-20230313215349629](C:\Users\az\AppData\Roaming\Typora\typora-user-images\image-20230313215349629.png)

#### Exercise 1.3

> 该任务要完成_start函数，完成该mips文件的时候要注意这里的寄存器前不用加$和之前计算机组成原理中的mips程序有所不同。

#### Exercise 1.4

> 该练习给lab1最为复杂的一个练习要补全`vpringmf()`函数，要注意10进制数输入的正负，并且有由于第二个参数为 `unsigned long` 记得将负数转化为正数，要发现这一点，要理解各个函数的实现方式，否则容易出错。记得初始化各个参数。

# 体会和感悟

相比与lab0，lab1所需要动手完成的部分主要为我们较为熟悉的c语言，因此完成得更加顺利。但是lab1中需要我们理解的内容更为庞大，我对于内核，启动这些东西依旧不够了解。

#### 启动

由于GXemul仿真器的简化，我们可以直接按照内存布局图将内核载入内存的正确位置。随后我们便开始MOS的搭建，并且完成了_start函数，将栈指针放在正确的位置并跳转到mian函数。相较与现实中的操作系统启动流程，这只是很小的一部分，而且也有所简化。

#### printk

在该次实验中，我们也被要求完善printk函数，这里我解开了困扰我好久的问题——如何实现printf函数，在这里我学习到了一种全新的c语言语法va_list，深感c语言的奥妙，同时我了解到，printf函数效果其实是向一个特定地址写入一些数据来实现的。

#### 总结

总的来说，lab1完善了我的c语言知识，让我看到了很多以前没见过的实现程序的方式，了解到了c语言底层的实现，让我大开眼界。也了解到了.lds这种全新的文件格式，同时加深了对操作系统启动的理解。