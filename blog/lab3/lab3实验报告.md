# lab3实验报告

### 思考题

> **Thinking 3.1** 请结合 MOS 中的页目录自映射应用解释代码中 e->env_pgdir[PDX(UVPT)] = PADDR(e->env_pgdir) | PTE_V 的含义。

- `PADDR(e->env_pgdir) | PTE_V` 为页目录的物理地址，并将有效位设置为1，`e->env_pgdir[PDX(UVPT)] ` 为e进程所对应的页目录的自映射地址，该代码将e进程所对应的页目录的自映射地址设置为e进程页目录对应的物理地址，并将有效位设置为1，实现了页目录地址的自映射，实现内存的初始化。

> **Thinking 3.2** elf_load_seg 以函数指针的形式，接受外部自定义的回调函数 map_page。 请你找到与之相关的 data 这一参数在此处的来源，并思考它的作用。没有这个参数可不可以？为什么？ 

- 阅读 `map_page` 指针所指向的 `load_icode_mapper` 函数。可以发现这样一行代码。

  ```c
  struct Env *env = (struct Env *)data;
  ```

  可以看到data被转化为了一个指向`Env` 结构体的指针。而data的来源也是`load_icode` 函数所传入的一个指向 `Env` 结构体的指针。

- 这个参数很有必要，确保数据写入正确的线程和对应的页目录。没有该参数，就无法将所写物理地址映射到进程的虚拟地址的联系了。

> **Thinking 3.3** 结合 elf_load_seg 的参数和实现，考虑该函数需要处理哪些页面加载的情况。 

- `va` 没有页对齐。

- `va + bin_size`没有页对齐。

- `va + sgsize` 没有页对齐。

- 需要考虑上述三种情况随机组合出现。

-  `elf_load_seg`函数疑似有些问题，应该为30注释这样。

  - ```c
    int elf_load_seg(Elf32_Phdr *ph, const void *bin, elf_mapper_t map_page, void *data) {
    	u_long va = ph->p_vaddr;
    	size_t bin_size = ph->p_filesz;
    	size_t sgsize = ph->p_memsz;
    	u_int perm = PTE_V;
    	if (ph->p_flags & PF_W) {
    		perm |= PTE_D;
    	}
    
    	int r;
    	size_t i;
    	u_long offset = va - ROUNDDOWN(va, BY2PG);
    	if (offset != 0) {
    		if ((r = map_page(data, va, offset, perm, bin, MIN(bin_size, BY2PG - offset))) !=
    		    0) {
    			return r;
    		}
    	}
    
    	/* Step 1: load all content of bin into memory. */
    	for (i = offset ? MIN(bin_size, BY2PG - offset) : 0; i < bin_size; i += BY2PG) {
    		if ((r = map_page(data, va + i, 0, perm, bin + i, MIN(bin_size - i, BY2PG))) != 0) {
    			return r;
    		}
    	}
    
    	/* Step 2: alloc pages to reach `sgsize` when `bin_size` < `sgsize`. */
    	while (i < sgsize) {
    		if ((r = map_page(data, va + i, 0, perm, NULL, MIN(bin_size - i, BY2PG))) != 0) {
    		/*if ((r = map_page(data, va + i, 0, perm, NULL, MIN(sgsize - i, BY2PG))) != 0) { */
    			return r;
    		}
    		i += BY2PG;
    	}
    	return 0;
    }
    ```

    

> **Thinking 3.4** 思考上面这一段话，并根据自己在 Lab2 中的理解，回答： 
>
> - 你认为这里的 env_tf.cp0_epc 存储的是物理地址还是虚拟地址?

- CPU所用的地址都是虚拟地址。

> **Thinking 3.5** 试找出上述 5 个异常处理函数的具体实现位置。

- 0号异常，位于genex.S文件之中。

  - ```
    NESTED(handle_int, TF_SIZE, zero)
    	mfc0    t0, CP0_CAUSE
    	mfc0    t2, CP0_STATUS
    	and     t0, t2
    	andi    t1, t0, STATUS_IM4
    	bnez    t1, timer_irq
    	// TODO: handle other irqs
    timer_irq:
    	sw      zero, (KSEG1 | DEV_RTC_ADDRESS | DEV_RTC_INTERRUPT_ACK)
    	li      a0, 0
    	j       schedule
    END(handle_int)
    ```

- 1号异常，同样位于genex.S文件之中

  - ```
    .macro BUILD_HANDLER exception handler
    NESTED(handle_\exception, TF_SIZE, zero)
    	move    a0, sp
    	jal     \handler
    	j       ret_from_exception
    END(handle_\exception)
    .endm
    
    ...
    
    BUILD_HANDLER mod do_tlb_mod
    ```

  - 而`do_tlb_mod` 函数位于`traps.c`  ，源码如下

    ```C
    void do_tlb_mod(struct Trapframe *tf) {
    	struct Trapframe tmp_tf = *tf;
    
    	if (tf->regs[29] < USTACKTOP || tf->regs[29] >= UXSTACKTOP) {
    		tf->regs[29] = UXSTACKTOP;
    	}
    	tf->regs[29] -= sizeof(struct Trapframe);
    	*(struct Trapframe *)tf->regs[29] = tmp_tf;
    
    	if (curenv->env_user_tlb_mod_entry) {
    		tf->regs[4] = tf->regs[29];
    		tf->regs[29] -= sizeof(tf->regs[4]);
    		// Hint: Set 'cp0_epc' in the context 'tf' to 'curenv->env_user_tlb_mod_entry'.
    		/* Exercise 4.11: Your code here. */
    
    	} else {
    		panic("TLB Mod but no user handler registered");
    	}
    }
    ```

- 2号异常，同样位于genex.S文件之中

  - ```
    .macro BUILD_HANDLER exception handler
    NESTED(handle_\exception, TF_SIZE, zero)
    	move    a0, sp
    	jal     \handler
    	j       ret_from_exception
    END(handle_\exception)
    .endm
    
    ...
    
    BUILD_HANDLER tlb do_tlb_refill
    ```

  - 而`do_tlb_refill` 函数位于`traps.c`  ，源码如下，就是我们之前写的Exercise 2.9。

    ```C
    Pte _do_tlb_refill(u_long va, u_int asid) {
    	Pte *pte = NULL;
    	struct Page *pp = NULL;
    	/* Hints:
    	 *  Invoke 'page_lookup' repeatedly in a loop to find the page table entry 'pte' associated
    	 *  with the virtual address 'va' in the current address space 'cur_pgdir'.
    	 *
    	 *  **While** 'page_lookup' returns 'NULL', indicating that the 'pte' could not be found,
    	 *  allocate a new page using 'passive_alloc' until 'page_lookup' succeeds.
    	 */
    
    	/* Exercise 2.9: Your code here. */
    	while(pp == NULL) {
    		pp = page_lookup(cur_pgdir, va, &pte);
    		if(pp == NULL) {
    			passive_alloc(va, cur_pgdir, asid);
    		}
    		else {
    			break;
    		}
    	}
    	return *pte;
    }
    ```

- 3号异常和2号异常的处理函数一样。

- 8号异常，同样位于genex.S文件之中

  - ```
    .macro BUILD_HANDLER exception handler
    NESTED(handle_\exception, TF_SIZE, zero)
    	move    a0, sp
    	jal     \handler
    	j       ret_from_exception
    END(handle_\exception)
    .endm
    
    ...
    
    BUILD_HANDLER sys do_syscall
    ```

  - 而`do_syscall` 函数，本次实验没有其源码。

> **Thinking 3.6** 阅读 init.c、kclock.S、env_asm.S 和 genex.S 这几个文件，并尝试说出 enable_irq 和 timer_irq 中每行汇编代码的作用。 

- `enable_irq` 

  ```
  LEAF(enable_irq)
  	li      t0, (STATUS_CU0 | STATUS_IM4 | STATUS_IEc)
  	// 将(STATUS_CU0 | STATUS_IM4 | STATUS_IEc)写入t0寄存器
  	mtc0    t0, CP0_STATUS
  	// 将t0寄存器存储的数值(STATUS_CU0 | STATUS_IM4 | STATUS_IEc)写入CP0_STATUS寄 	  //存器
  	jr      ra
  	// 返回函数
  END(enable_irq)
  ```

- `timer_irq ` 

  ```
  NESTED(handle_int, TF_SIZE, zero)
  	mfc0    t0, CP0_CAUSE
  	mfc0    t2, CP0_STATUS
  	and     t0, t2
  	andi    t1, t0, STATUS_IM4
  	bnez    t1, timer_irq
  	// TODO: handle other irqs
  timer_irq:
  	sw      zero, (KSEG1 | DEV_RTC_ADDRESS | DEV_RTC_INTERRUPT_ACK)
  	// (KSEG1 | DEV_RTC_ADDRESS | DEV_RTC_INTERRUPT_ACK)位置的一个字写为0
  	li      a0, 0
  	// 将a0寄存器写为0
  	j       schedule
  	// 跳转到schedule函数
  END(handle_int)
  ```

> **Thinking 3.7** 阅读相关代码，思考操作系统是怎么根据时钟中断切换进程的。 

- 本次实验采用了时间片轮转调度算法，每1/200秒产生一次时钟中断在处理时钟中断的过程中进行进程切换。而在内存中，存在一个循环的env链表，记载了所有的可执行进程，并且env结构体会记载每个进程的加权，当发生进程切换时，取出链表中的第一个进程作为当前进程。当几个进程切换时当一下情况时，会切换进程。
  - 参数 yield 非 0 的时候
  - 一个进程时间片用尽的时候，即静态变量 count 为 0。
  - curenv 为 NULL ，也就是操作系统第一次运行调度器的时候。
  -  当前进程的状态不再就绪，即 e->env_status != ENV_RUNNABLE 时。

- 若切换进程，则切换curenv或者将该进程进删除或者放回链表尾部。否则curenv自减一。

### 实验难点展示

难点还是在于对宏定义的理解，结构体足够熟悉，否则会出现各种各样的错误。例如Exercise 3.1的这种链表的宏定义及其用法。同时要理解到，env_free_list是普通链表而env_sched_list为循环链表。

#### Exercise 3.12

由于本章的注释和指导书写的很详细，因此我完成本次作业很顺利，但是3.12是一个例外。本次Exercise要求我们完成Schedule函数，而其要完成几乎绝大多数内容。根据注释以及指导书完成代码，但是由于指导书和注释对于yield解释较为模糊，完成时不是很能理解如何进行判定，当然犹豫了很久不知道怎么填，就没填直接提交，反而莫名其妙得分100分。在后面看到lab3的解说视频时才恍然大悟，还有就是panic部分，注释和指导书只说了要panic但是不知道要输出什么内容。

#### debug

本次实验注释和指导书都很详细，代码完成顺着指导书所写的就能顺利完成，但是debug部分较为麻烦。由于本系统较为简陋，常常出现运行卡死，没有任何报错信息的情况，我的经验是，若出现卡死无报错，大概率是指针出现问题。此外可以在测试样例调用`printk()` 函数来检查是那一部分出现错误，也可以打印出一些关键参数，看看是否复合预期。

### 思考与体会

本次实验相较于上一次lab其实难度相对降低，主要是注解和指导书都更为详细，只需要根据注释所写来完成代码，很快就能顺利完成作业。同时，经过这一次实验和这几次上机，我了解到了熟练运用c语言的重要性，在上一次上机中，我对c语言指针加法的不理解导致了上机没有完成，指针出错，常常会无法报出错误位置，而且由于上机环境问题，导致很难进行调试。而且，在本次lab3中我也有几处由于对c语言特性不熟练所导致的bug，也常常是不报错单纯卡死，令人十分困惑。在我看来，完成实验和上机，主要是要求对理论以及c语言足够熟练，课下实验的代码也对上机的完成有很好的指导作用。最后，了解一些linux指令和vim的快捷方式对于效率的提高很有帮助，熟练应用能事半功倍。