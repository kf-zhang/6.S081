# Lab net

## prepare

[video](https://www.bilibili.com/video/BV1QA411F7ij?p=20)有对net的讲解，对于理解```kernel/net.h```以及网络协议栈有着比较好的作用.
[E1000 mannual](https://pdos.csail.mit.edu/6.828/2021/readings/8254x_GBe_SDM.pdf)结合代码中E1000的相关宏定义可以参考各个宏定义的作用.
[xv6 book](https://pdos.csail.mit.edu/6.828/2021/xv6/book-riscv-rev2.pdf)中concole的代码结构与E1000基本一致，可以参考.

## e1000_transmit

### transmit init

```c
  memset(tx_ring, 0, sizeof(tx_ring));
  for (i = 0; i < TX_RING_SIZE; i++) {
    tx_ring[i].status = E1000_TXD_STAT_DD;
    tx_mbufs[i] = 0;
  }
  regs[E1000_TDBAL] = (uint64) tx_ring;
  if(sizeof(tx_ring) % 128 != 0)
    panic("e1000");
  regs[E1000_TDLEN] = sizeof(tx_ring);
  regs[E1000_TDH] = regs[E1000_TDT] = 0;
```

### 相关宏定义

* E1000_TDT E1000_TDH E1000_TDBAL E1000_TDLEN
[E1000 mannual](https://pdos.csail.mit.edu/6.828/2021/readings/8254x_GBe_SDM.pdf) 3.4

* E1000_TXD_STAT_DD
[E1000 mannual](https://pdos.csail.mit.edu/6.828/2021/readings/8254x_GBe_SDM.pdf) 3.3.7.2

### transmit 实现

```c
int
e1000_transmit(struct mbuf *m)
{
  //
  // Your code here.
  //
  // the mbuf contains an ethernet frame; program it into
  // the TX descriptor ring so that the e1000 sends it. Stash
  // a pointer so that it can be freed after sending.
  //

  acquire(&e1000_lock);

  uint64 idx = regs[E1000_TDT];
  struct tx_desc* entry = tx_ring+idx;
  if( !(entry->status &  E1000_TXD_STAT_DD) ){
    release(&e1000_lock);
    return -1;
  }
  if(tx_mbufs[idx])
    mbuffree(tx_mbufs[idx]);
  
  entry->addr = (uint64)m->head;
  entry->length = m->len;
  entry->cmd = E1000_TXD_CMD_EOP | E1000_TXD_CMD_RS;
  tx_mbufs[idx] = m;//记录指针用于未来free

  regs[E1000_TDT]=(idx+1)%TX_RING_SIZE;
  __sync_synchronize();//防止乱序执行导致的问题
  release(&e1000_lock);

  return 0;

}
```

## e1000_recv

### recv init

```c
// [E1000 14.4] Receive initialization
  memset(rx_ring, 0, sizeof(rx_ring));
  for (i = 0; i < RX_RING_SIZE; i++) {
    rx_mbufs[i] = mbufalloc(0);
    if (!rx_mbufs[i])
      panic("e1000");
    rx_ring[i].addr = (uint64) rx_mbufs[i]->head;
  }
  regs[E1000_RDBAL] = (uint64) rx_ring;
  if(sizeof(rx_ring) % 128 != 0)
    panic("e1000");
  regs[E1000_RDH] = 0;
  regs[E1000_RDT] = RX_RING_SIZE - 1;
  regs[E1000_RDLEN] = sizeof(rx_ring);
```