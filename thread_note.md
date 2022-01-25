# Lab: Multithreading note

## Uthread: switching between threads

* context 
  
  模仿kernel/proc.h中 context结构体的形势,由于线程池切换是通过函数调用实现的所以只需要保存callee-saved regisers(进程切换通过thread_yield函数主动让出cpu).

```c
struct context {
  uint64 ra;
  uint64 sp;
  // callee-saved
  uint64 fp;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

struct thread {
  int        state;             /* FREE, RUNNING, RUNNABLE */
  struct context registers;
  char       stack[STACK_SIZE]; /* the thread's stack */
};
```

* context switch

context switch中除了要保存callee-saved registers,还要保存返回地址```ra```.
代码与kernel/swtch.S相同. context switch实际上就是通过替换```ra sp```来改变指令的执行流,同时为了遵循ABI保存callee-saved registers.

假设有以下两条函数调用

```shell
    thread_a() -> thread_yield() -> thread_schedule() -> thread_switch()
    thread_b() -> thread_yield() -> thread_schedule() -> thread_switch()
```

通过```thread_switch```后，```thread_a```会将```ra sp```设置为```thread_b```的的```ra sp```,然后```thread_b```经过```thread_switch```后
又将```ra sp```恢复为之前的```ra sp```,然后```thread_a```就通过```ret```返回```thread_schedule() -> thread_yield() -> thread_a()```,就从之前yield的地方继续执行。

```assemble
.text
/*
* save the old thread's registers,
* restore the new thread's registers.
*/
.globl thread_switch
thread_switch:
/* YOUR CODE HERE */
sd ra, 0(a0)
sd sp, 8(a0)
sd s0, 16(a0)
sd s1, 24(a0)
sd s2, 32(a0)
sd s3, 40(a0)
sd s4, 48(a0)
sd s5, 56(a0)
sd s6, 64(a0)
sd s7, 72(a0)
sd s8, 80(a0)
sd s9, 88(a0)
sd s10, 96(a0)
sd s11, 104(a0)

ld ra, 0(a1)
ld sp, 8(a1)
ld s0, 16(a1)
ld s1, 24(a1)
ld s2, 32(a1)
ld s3, 40(a1)
ld s4, 48(a1)
ld s5, 56(a1)
ld s6, 64(a1)
ld s7, 72(a1)
ld s8, 80(a1)
ld s9, 88(a1)
ld s10, 96(a1)
ld s11, 104(a1)

ret    /* return to ra */

```

* thread_create

由于栈是从高向低增长的，所以初始的```sp```的初始位置为

```c
t->registers.sp = (uint64)(&t->stack[STACK_SIZE-1]);
```

## barrier

pthread_cond_wait 后会重新获得锁，所以后面需要释放锁

```c
static void 
barrier()
{
  // YOUR CODE HERE
  //
  // Block until all threads have called barrier() and
  // then increment bstate.round.
  //
  pthread_mutex_lock(&bstate.barrier_mutex);
  bstate.nthread++;
  if( bstate.nthread == nthread){
    bstate.nthread=0;
    bstate.round++;

    pthread_cond_broadcast(&bstate.barrier_cond);
    pthread_mutex_unlock(&bstate.barrier_mutex);
  }
  else{
    pthread_cond_wait(&bstate.barrier_cond,&bstate.barrier_mutex);
    pthread_mutex_unlock(&bstate.barrier_mutex);
  }
  
}
```
