# 操作系统lab2实验报告

### 思考题

> **Thinking 2.1** 请根据上述说明，回答问题：在编写的 C 程序中，指针变量中存储的地址是虚拟地址，还是物理地址？MIPS 汇编程序中 lw 和 sw 使用的是虚拟地址，还是物理地址？

- 指针地址是虚拟地址，MIPS中lw，sw指令也都是虚拟地址。

> **Thinking 2.2** 请思考下述两个问题： 
>
> - 从可重用性的角度，阐述用宏来实现链表的好处。 
> -  查看实验环境中的 /usr/include/sys/queue.h，了解其中单向链表与循环链表的实现，比较它们与本实验中使用的双向链表，分析三者在插入与删除操作上的性能差异。

- 用宏来实现链表可以适配不同的链表名称，不同的成员变量，不同的数据类型。在调用这些宏的时候有强大的适配性，对各种各样不同的链表都能适用，可移植性也更强。此外，宏是直接对字符串进行替换，这样更具有灵活性，也能一定程度上提高性能。

- 实验环境中的queue.h中的单向链表和循环链表链表（这里截取了插入和删除的宏定义）。

  ```c
  //单向链表
  #define SIMPLEQ_INSERT_AFTER(head, listelm, elm, field) do {            \
           if (((elm)->field.sqe_next = (listelm)->field.sqe_next) == NULL)\
                   (head)->sqh_last = &(elm)->field.sqe_next;              \
          (listelm)->field.sqe_next = (elm);                              \
  } while (/*CONSTCOND*/0)
  
  #define STAILQ_REMOVE(head, elm, type, field) do {                      \
          if ((head)->stqh_first == (elm)) {                              \
                  STAILQ_REMOVE_HEAD((head), field);                      \
          } else {                                                        \
                  struct type *curelm = (head)->stqh_first;               \
                  while (curelm->field.stqe_next != (elm))                        \
                          curelm = curelm->field.stqe_next;               \
                  if ((curelm->field.stqe_next =                          \
                          curelm->field.stqe_next->field.stqe_next) == NULL) \
                              (head)->stqh_last = &(curelm)->field.stqe_next; \
          }                                                               \
  } while (/*CONSTCOND*/0)
  //循环链表
  #define CIRCLEQ_INSERT_AFTER(head, listelm, elm, field) do {            \
          (elm)->field.cqe_next = (listelm)->field.cqe_next;              \
          (elm)->field.cqe_prev = (listelm);                              \
          if ((listelm)->field.cqe_next == (void *)(head))                \
                  (head)->cqh_last = (elm);                               \
          else                                                            \
                  (listelm)->field.cqe_next->field.cqe_prev = (elm);      \
          (listelm)->field.cqe_next = (elm);                              \
  } while (/*CONSTCOND*/0)
  
  #define CIRCLEQ_INSERT_BEFORE(head, listelm, elm, field) do {           \
          (elm)->field.cqe_next = (listelm);                              \
          (elm)->field.cqe_prev = (listelm)->field.cqe_prev;              \
          if ((listelm)->field.cqe_prev == (void *)(head))                \
                  (head)->cqh_first = (elm);                              \
          else                                                            \
                  (listelm)->field.cqe_prev->field.cqe_next = (elm);      \
          (listelm)->field.cqe_prev = (elm);                              \
  } while (/*CONSTCOND*/0)
      
  #define CIRCLEQ_REMOVE(head, elm, field) do {                           \
          if ((elm)->field.cqe_next == (void *)(head))                    \
                  (head)->cqh_last = (elm)->field.cqe_prev;               \
          else                                                            \
                  (elm)->field.cqe_next->field.cqe_prev =                 \
                      (elm)->field.cqe_prev;                              \
          if ((elm)->field.cqe_prev == (void *)(head))                    \
                  (head)->cqh_first = (elm)->field.cqe_next;              \
          else                                                            \
                  (elm)->field.cqe_prev->field.cqe_next =                 \
                      (elm)->field.cqe_next;                              \
  } while (/*CONSTCOND*/0)
  ```

- 本次实验中的queue.h中的双向链表（同上）。

  ```c
  #define LIST_INSERT_AFTER(listelm, elm, field)                                             \
  	do {                                                                                   \
  		LIST_NEXT((elm), field) = LIST_NEXT((listelm), field);                             \
  		if (LIST_NEXT((elm), field) != NULL) {                                             \
  			LIST_NEXT((elm), field)->field.le_prev = &LIST_NEXT((elm), field);  		   \
  		}                                                                                  \
  		LIST_NEXT((listelm), field) = (elm);                                               \
  		(elm)->field.le_prev = &LIST_NEXT((listelm), field);                               \
  	} while (0)
  
  #define LIST_INSERT_BEFORE(listelm, elm, field)                                            \
  	do {                                                                                   \
  		(elm)->field.le_prev = (listelm)->field.le_prev;                                   \
  		LIST_NEXT((elm), field) = (listelm);                                               \
  		*(listelm)->field.le_prev = (elm);                                                 \
  		(listelm)->field.le_prev = &LIST_NEXT((elm), field);                               \
  	} while (0)
  
  #define LIST_REMOVE(elm, field)                                                            \
  	do {                                                                                   \
  		if (LIST_NEXT((elm), field) != NULL)                                               \
  			LIST_NEXT((elm), field)->field.le_prev = (elm)->field.le_prev;            	   \
  		*(elm)->field.le_prev = LIST_NEXT((elm), field);                                   \
  	} while (0)
  ```

- 可以看到，单向链表插入最为简单，运行效率也最快，但是不能向前方插入元素，删除需要遍历整个链表速度较慢；双向链表效率次之，因为要修改四个元素，但可以向前插入元素，删除速度很快；循环链表也是一个双向链表，也可以前后插入，而且效率与双向链表相近，但是删除效略低，但是循环链表可以一直去下一个元素，有着特殊作用。

- 速度比较：

| 链表类型         | 插入速度 | 删除速度 |
| ---------------- | -------- | -------- |
| 单向链表         | O(1)     | O(n)     |
| 双向链表         | O(1)     | O(1)     |
| 循环量表（双向） | O(1)     | O(1)     |



> **Thinking 2.3** 请阅读 include/queue.h 以及 include/pmap.h, 将 Page_list 的结构梳理清楚，选择正确的展开结构。
>
> ```c
> A:
> struct Page_list{
> 	struct {
> 		struct {
> 			struct Page *le_next;
> 			struct Page **le_prev;
> 		}* pp_link;
> 		u_short pp_ref;
> 	}* lh_first;
> }
> ```
>
> ```c
> B：
> struct Page_list{
> 	struct {
> 		struct {
> 			struct Page *le_next;
> 			struct Page **le_prev;
> 		} pp_link;
> 		u_short pp_ref;
> 	} lh_first;
> }
> ```
>
> ```c
> C:
> struct Page_list{
> 	struct {
> 		struct {
> 			struct Page *le_next;
> 			struct Page **le_prev;
> 		} pp_link;
> 		u_short pp_ref;
> 	}* lh_first;
> }
> ```

- 答案为c,阅读上述文件逐步带入即可。

> **Thinking 2.4** 请思考下面两个问题： 
>
> - 请阅读上面有关 R3000-TLB 的描述，从虚拟内存的实现角度，阐述 ASID 的必要性。
> - 请阅读《IDT R30xx Family Software Reference Manual》的 Chapter 6，结合 ASID 段的位数，说明 R3000 中可容纳不同的地址空间的最大数量。

- 程序中可能存在多个进程，在不同的内存空间中，同一虚拟地址可能映射到不同的物理地址，利用ASID可以判断虚拟地址到底映射到哪一个物理地址，用来标识进程，并为进程提供地址空间保护。
- ASID为EntryHi Register的6-11位，一共有六位地址，地址空间的最大数量位2^6^，一共64位。

> **Thinking 2.5** 请回答下述三个问题：
>
> - tlb_invalidate 和 tlb_out 的调用关系？ 
> - 请用一句话概括 tlb_invalidate 的作用。
> - 逐行解释 tlb_out 中的汇编代码。

- `tlb_invalidate` 调用 `tlb_out` 。

- 删除特定虚拟地址对应的TLB中的旧表项。

-  `tlb_out` 的源码和注释

  ```
  LEAF(tlb_out)
  .set noreorder
  	mfc0    t0, CP0_ENTRYHI 
  	/* 将CP0_ENTRYHI寄存器中的数据写到t0寄存器中，读取CP0_ENTRYHI寄存器。*/
  	mtc0    a0, CP0_ENTRYHI
  	/* 将a0寄存器中的数据写到CP0_ENTRYHI寄存器中。*/
  	nop
  	/* 位tlbp指令预留的nop，防止数据冲突。*/
  	/* Step 1: Use 'tlbp' to probe TLB entry */
  	tlbp
  	/*查询参数的地址是否存在于TLB中，查找TLB中的对应表项，将索引写入Index寄存器，如果查找失败，则Index的最高位置1*/
  	nop
  	/* 位tlbp指令预留的nop，防止数据冲突。*/
  	/* Step 2: Fetch the probe result from CP0.Index */
  	mfc0    t1, CP0_INDEX
  	/* 将CP0_INDEX寄存器中的数据写到t1寄存器中，读取CP0_INDEX寄存器。*/
  .set reorder
  	bltz    t1, NO_SUCH_ENTRY
  	/* 当t1寄存器小于0，即CP0_INDEX寄存器最高位为一，之前查找失败，就跳转到NO_SUCH_ENRRY函数。
  .set noreorder
  	mtc0    zero, CP0_ENTRYHI
  	/* 将CP0_ENTRYHI寄存器清零。*/
  	mtc0    zero, CP0_ENTRYLO0
  	/* 将CP0_ENTRYLO0寄存器清零。*/
  	nop
  	/* Step 3: Use 'tlbwi' to write CP0.EntryHi/Lo into TLB at CP0.Index  */
  	tlbwi
  	/*将CP0.EntryHi/Lo写入CP0.Index处的TLB*/
  .set reorder
  
  NO_SUCH_ENTRY:
  	mtc0    t0, CP0_ENTRYHI
  	/* 将CP0_ENTRYHI寄存器中的数据写到t0寄存器中，读取CP0_ENTRYHI寄存器。*/
  	j       ra
  	/*返回函数*/
  END(tlb_out)
  ```

  

> **Thinking 2.6** 任选下述二者之一回答： 
>
> -  简单了解并叙述 X86 体系结构中的内存管理机制，比较 X86 和 MIPS 在内存管理上的区别。 
> -  简单了解并叙述 RISC-V 中的内存管理机制，比较 RISC-V 与 MIPS 在内存管理上的区别。

- X86架构内存管理机制主要为两部分：分段机制和分页机制。
  - 分段机制：分段机制将内存分为以基地址和长度描述的块，为不同程序划分了不同的代码区域、数据区域、栈区域，从而避免了同一个处理器上运行的多个程序互相影响。
  - 分页机制：将线性地址和物理地址联系起来，机制与MIPS架构的分页机制类似。
- 差别主要体现在TLB不命中的处理上。
  - MIPS会触发TLB Refill 异常，内核的 tlb_refill_handler 会以 pgd_current 为当前进程的 PGD 基址，索引获得转换失败的虚址对应的 PTE，并将其填入 TLB。
  - 而 X86 在 TLB 不命中时，是由硬件 MMU 以 CR3 为当前进程的 PGD 基址，索引获得 PFN 后，直接输出 PA。同时 MMU 会填充 TLB 以加快下次转换的速度。

### 难点分析

#### exercise 2.1

难点主要在于一页的大小为4KB，这点老师上课其实讲过，但如果上课没听讲就很难得知这一点了。

#### exercise 2.2

难点主要在于理解链表结构，以及宏定义的使用某种程度上让代码更加难以理解。链表虽然之前在数据结构课程上学习过而且较为简单，但是本次的链表指向前一个元素的指针是一个指向前一个指针指向后一个指针的指针。其关系图如下。理解了之一结构就不难做出来。此外还要考虑插入元素为最后一个元素的情况。![屏幕截图 2023-03-20 165323](C:\Users\az\Desktop\project\os\expr\第三次实验报告\屏幕截图 2023-03-20 165323.png)

#### exercise 2.3

~~噩梦开始~~，从这一次exercise开始，后面会有大量宏定义和函数，需要我们仔细阅读 `qmap.h`,`env.h` 以及`mmu.h` 文件，了解对应宏定义的作用。还要仔细阅读注释，同时还要理解虚拟地址和物理地址的区别（**在用户态使用的大多为虚拟地址**），难度相比于之前的练习提高很多。这里列出一些宏定义以及宏定义的作用。在了解这些宏定义以及`freemem`为虚拟地址后，就能很顺利写出这道练习了。

| 宏定义或者函数名称 | 效果解释                 |
| ------------------ | ------------------------ |
| BY2PG              | 4096即一页所含字节       |
| PADDR              | 输入物理地址获得虚拟地址 |
| KADDR              | 输入虚拟地址获得物理地址 |
| page2kva           | 获取页的虚拟地址         |
| page2pa            | 获取页的物理地址         |
| PTE_ADDR           | 获取一级页表上PTBase(PA) |
| va2pa              | 将虚拟地址转化为物理地址 |

#### exercise 2.4

熟练运用已经写好的宏定义，注意 `memset` 使用的也是虚拟地址，要进行转化。注意错误的返回值为`-E_NO_MEM`。

#### exercise 2.6 & exercise 2.7

这两个关联性较强，我放在一起讲。要完成这两个函数放在一起讲解。可以说，这两道题目是本次实验的最难点，这次练习要使用到有效位，利用有效位宏定义与页表取与可以判断有效位是否有效，同时利用好或运算修改权限位。对照流程图和注解就能完成这两次作业。

![屏幕截图 2023-03-20 220819](C:\Users\az\Desktop\project\os\expr\第三次实验报告\屏幕截图 2023-03-20 220819.png)

在这两次作业中，我出现了一个很难以发现的bug，起因是walk函数，要传入一个链表的指针的指针，我在函数外定义了一个链表的指针的指针，并将这个指针传入walk函数，在进行检测的时候，因为我传入的是一个空指针，对其指向的值赋值时出现了直接卡死的情况。后来通过一步步调试发现是一个指针赋值的时候卡死，这让我百思不得其解，我将指针输出，发现结果很正常，如何我再将指针所指向的值输出，发现都是0。问题正在于此，传入的参数是一个指针的指针，都指向了0x0这个地址，我对这个地址进行了修改，因此发生的卡死现象。通过这一件事，加深了我对指针的畏惧，在使用指针事，一定要明白指针所指向的对象，防止出现错误。

### 体会与感悟

本次lab2实验和前面的实验难度差距很大，可以说我在这一次实验上所画的精力比前两次作业加起来还要多。本次作业主要在于MIPS架构的内存管理机制，其中我对TLB重填的机制，对虚拟地址如何使用还是不太理解。此外，本次实验由于大量的宏定义和函数，加大了理解和完成代码的难度，这也是难度大的原因之一。同时，本次实验也让我加深了对c语言，特别是指针的理解，对指针的处理一定要慎之又慎。
