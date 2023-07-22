# OS lab4实验报告

### 1.思考题

> **Thinking 4.1** 思考并回答下面的问题： 
>
> - 内核在保存现场的时候是如何避免破坏通用寄存器的？ 
> -  系统陷入内核调用后可以直接从当时的 $a0-$a3 参数寄存器中得到用户调用 msyscall 留下的信息吗？ 
> - 我们是怎么做到让 sys 开头的函数“认为”我们提供了和用户调用 msyscall 时同样 的参数的？ 
> - 内核处理系统调用的过程对 Trapframe 做了哪些更改？这种修改对应的用户态的变化是什么？

1. 调用`SAVA_ALL`函数，将寄存器的值存储在内存之中。
2. 可以，a0-a3寄存器存储了系统调用传入的前四个参数。
3. 将参数传入a0-a3寄存器以及堆栈中，堆栈指针为sp，通过sp能在内存中获取其他参数。
4. 修改EPC的值，让程序能跳转回正确的地址，将参数写入v0寄存器，相当于程序的返回值。

> **Thinking 4.2** 思考 envid2env 函数: 为什么 envid2env 中需要判断 e->env_id != envid 的情况？如果没有这步判断会发生什么情况？

- 在envid2env中，e是这样取出的。

  ```c
  e = &envs[ENVX(envid)];
  
  //宏定义如下
  #define LOG2NENV 10
  #define NENV (1 << LOG2NENV)
  #define ENVX(envid) ((envid) & (NENV - 1))
  ```

  不难看出，该操作是以envid的最后10位为索引取出从envs中取出env块，但这不能保证该进程的id与envid一样，以为进程块可以替换，需要加入额外的判断条件，确保所取进程块id和所需id相同。

> **Thinking 4.3** 思考下面的问题，并对这个问题谈谈你的理解：请回顾 kern/env.c 文件 中 mkenvid() 函数的实现，该函数不会返回 0，请结合系统调用和 IPC 部分的实现与 envid2env() 函数的行为进行解释。

- `mkenvid()`函数的实现如下

  ```c
  u_int mkenvid(struct Env *e) {
  	static u_int i = 0;
  	return ((++i) << (1 + LOG2NENV)) | (e - envs);
  }
  
  //宏定义，envs为env的数组头，e-envs为env的索引
  #define LOG2NENV 10
  ```

  可以看到， ((++i) << (1 + LOG2NENV))一定是一个非0数，因此返回值不可能为0。而在IPC和` envid2env()`函数中，envid为0都是一个判定条件，因此envid不应该为0。

> **Thinking 4.4** 关于 fork 函数的两个返回值，下面说法正确的是：
>
>  A、fork 在父进程中被调用两次，产生两个返回值 
>
> B、fork 在两个进程中分别被调用一次，产生两个不同的返回值 
>
> C、fork 只在父进程中被调用了一次，在两个进程中各产生一个返回值 
>
> D、fork 只在子进程中被调用了一次，在两个进程中各产生一个返回

C

> **Thinking 4.5** 我们并不应该对所有的用户空间页都使用 duppage 进行映射。那么究竟哪些用户空间页应该映射，哪些不应该呢？请结合 kern/env.c 中 env_init 函数进行的页面映射、include/mmu.h 里的内存布局图以及本章的后续描述进行思考。

- 0~USTACKTOP空间的页面需要共享，该处于该地址的页面为常规的用户空间，而USTACKTOP以上为异常处理栈和操作系统内核，都不应该映射。

> **Thinking 4.6** 在遍历地址空间存取页表项时你需要使用到 vpd 和 vpt 这两个指针，请参 考 user/include/lib.h 中的相关定义，思考并回答这几个问题： 
>
> - vpt 和 vpd 的作用是什么？怎样使用它们？ 
> - 从实现的角度谈一下为什么进程能够通过这种方式来存取自身的页表？ 
> - 它们是如何体现自映射设计的？ 
> - 进程能够通过这种方式来修改自己的页表项吗？

- vpt和vpd的定义如下

  ```c
  #define vpt ((volatile Pte *)UVPT)
  #define vpd ((volatile Pde *)(UVPT + (PDX(UVPT) << PGSHIFT)))
  ```

  可以看到vpt是一个固定的Ptd的指针为页表首页地址，vpd是一个固定的Pde指针为页目录首页地址，直接加上索引或者当作数组使用。

- 通过UVPT来获取vpt地址，并将页表都存储在UVPT中。

- `(UVPT + (PDX(UVPT) << PGSHIFT))) ` 就是计算自映射地址。

- 不能，这些页面的权限为只读，需要陷入内核态才能进行修改。

> **Thinking 4.7** 在 do_tlb_mod 函数中，你可能注意到了一个向异常处理栈复制 Trapframe 运行现场的过程，请思考并回答这几个问题： 
>
> -  这里实现了一个支持类似于“异常重入”的机制，而在什么时候会出现这种“异常重 入”？ 
>
> - 内核为什么需要将异常的现场 Trapframe 复制到用户空间？

- 发生中断时，又出现了另一个中断，先去执行另一个中断。
- 将Trapframe复制到用户空间，在用户态处理，可以让内核承担更少的作用，同时让异常结束后恢复原有的运行环境。

> Thinking 4.8 在用户态处理页写入异常，相比于在内核态处理有什么优势？

- 可以让内核所承担的内容更少，践行微内核的思想，加强系统的可移植性。

> **Thinking 4.9** 请思考并回答以下几个问题： 
>
> -  为什么需要将 syscall_set_tlb_mod_entry 的调用放置在 syscall_exofork 之前？ 
> -  如果放置在写时复制保护机制完成之后会有怎样的效果？

- 父子进程共享空间，在父进程运行fork时，可能出现缺页异常。
- 写入又cow保护的页面时，回触发缺页异常，将为进程额外分配映射一个物理页面，从而防止出现冲突同时节省内存空间。

### 2.难点分析

本单元内容较多，包括系统调用，IPC的实现，以及能够实现COW功能的fork函数。

#### 系统调用

系统调用调用mysyscall函数使得系统从用户态跌落内核态,从而调用内核资源完成工作。其流程如下

![屏幕截图 2023-04-27 083236](C:\Users\az\Desktop\project\os\expr\lab4\屏幕截图 2023-04-27 083236.png)

#### exercise 4.2

本次exercise中我们需要完成`do_syscall`函数，要注意的主要是arg4和arg5的读取。由于c语言对于执指针的计算和非指针变量的计算不同，要注意以防出错。

```c
arg4 = *((long*)(tf->regs[29]) + 4);
arg5 = *((long*)(tf->regs[29]) + 5);

//或者
arg4 = *((long*)(tf->regs[29] + 16));
arg5 = *((long*)(tf->regs[29] + 20));
```

#### exercise 4.3

要注意的是envid为0的情况。

```c
if (envid == 0) {
		*penv = curenv;
		return 0;
	}
```

要直接返回函数，因为分配已经完成了。进行后面的处理反而会出错。

#### IPC

IPC是微内核的重要机制之一，用以实现两个进程间的通讯。本次lab中，主要通过修改env结构体和页表联立映射来实现IPC功能。

#### Frok

frok是linux用户态构建新进程的方式，实现通过构建一个新的env块来实现，产生的子进程和父进程共享内存。此外，本次实现的frok还要支持COW功能，COW就是父子进程共享的可读写页面提供一个COW权限位；当写入带有COW权限的页面时，会触发缺页异常，让虚拟地址映射到一个新的物理页面上的，从而节省内存空间同时防止冲突。`frok()` 函数的完成是本次lab的重难点，因为无法将子进程的结构打印出来，映射的页表也不知道哪里会出现问题，因此无法判断具体是那一部分出现错误。在`frok()`函数的实现中，我就因为这一点久久查不出问题所在。下面为frok流程图。

![image-20230427201546031](C:\Users\az\AppData\Roaming\Typora\typora-user-images\image-20230427201546031.png)

#### exercise 4.10 

这个exercise要求完成`duppage()`函数，就是建立子进程指定页面的页面映射，由于对COW机制的不了解以及注释讲解得不是很清楚，导致我在`fork()`函数上耗费了一整天查找bug，最后发现是`duppage()`函数建立映射有问题，调用子进程时，出现了RI异常。具体的的实现如下：

```c
static void duppage(u_int envid, u_int vpn) {
	u_int addr;
	u_int perm;
	/* Step 1: Get the permission of the page. */
	/* Hint: Use 'vpt' to find the page table entry. */
	/* Exercise 4.10: Your code here. (1/2) */
	addr = vpn * BY2PG;
	Pte* ppte = vpt + vpn;
	perm = *ppte & 0xfff;
	/* Step 2: If the page is writable, and not shared with children, and not marked as COW yet,
	 * then map it as copy-on-write, both in the parent (0) and the child (envid). */
	/* Hint: The page should be first mapped to the child before remapped in the parent. (Why?)
	 */
	/* Exercise 4.10: Your code here. (2/2) */
	if(!(perm & PTE_D) || (perm & PTE_LIBRARY)) { 
		syscall_mem_map(0, addr, envid, addr, perm);
	} 
	else {
		perm = (perm & ~PTE_D) | PTE_COW;
		syscall_mem_map(0, addr, envid, addr, perm);
		syscall_mem_map(0, addr, 0, addr, perm);
	}
}
```

在step2中，我一开始只映射了可读写的页面而忽略了不可读的页面，导致出现有些页面没有被分配，最后出现RI异常。此外，对于共享的可读写页面，也需要取出可读的权限位`PTE_D`否则无法触发缺页异常位进程重新映射一个全新的页面。最后，要先映射子页面再重新映射父进程页面。

#### exercise 4.11 cow_entry

这个exercise有几个要注意的细节。

- 传入的虚拟地址可能没有页对齐，需要用`ROUNDDOWN(va, BY2PG)`宏将虚拟地址对齐。
- cow_entry位于用户态，需要使用`syscall_mem_alloc()`系统调用陷入内核态创建新页表。

#### exercise 4.15

- 记得检查页面是否有效，将i左移10为就是页目录索引。

  ```c
  for (i = 0; i < VPN(USTACKTOP); i++) {
  		if ((*(vpd + (i >> 10)) & PTE_V) == 0) {
  			i += 1 >> 10;
  			continue;
  		}
  		if (*(vpt + i) & PTE_V) {
  			duppage(child, i);
  		}
  	}
  ```

### 3.体会与感受

本次作业难度又是大幅度提升，首先是要完成的内容比较多，系统调用，IPC以及`fork()`函数。其次，对于`fork()`函数，debug的难度相当之大，`fork()`函数可谓是一步错步步错，但是你又很难找出是那一部分出现了问题，本次作业中，我仅仅`dugpage()`函数对于COW映射的机制不完全了解，就让我废了好大功夫来找到底是那一部分出现问题，最后排查到调度子进程直接报错RI异常，即使找出了出错的地方也不知道到底是那一部分配置错误。最后还是看指导书对COW机制的解释才发现自己的页表映射出现了错误。本次作业我有如下收获。

- 要充分了解程序运行的流程和作用的机制，而不是仅仅根据指导书和注释来完形填空。
- 要了解内核态和用户态的区别，不要出现内核态调用用户态函数，用户态调用内核地址这种情况。
- 要了解系统调用的流程以及系统调用的定义和书写，我第一次完成任务时根本找不到系统调用定义再何处。