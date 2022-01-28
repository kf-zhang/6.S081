// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct page{
  struct spinlock lock;
  struct run *freelist;
};

struct page freelists[NCPU];

void
init_list(struct page* p,uint64 startpa,uint64 endpa){

    if(((uint64)startpa % PGSIZE) != 0 || (char*)startpa < end || (uint64)startpa >= PHYSTOP)
        panic("init_list");
    if(((uint64)endpa % PGSIZE) != 0 || (char*)endpa < end || (uint64)endpa >= PHYSTOP)
        panic("init_list");

    
    struct run* r;
    push_off();
    acquire(&p->lock);
    p->freelist = 0;
    for(uint64 pa = startpa;pa<endpa;pa+=PGSIZE){
        r = (struct run*)pa;
        r->next = p->freelist;
        p->freelist = r;
    }
    release(&p->lock);
    pop_off();
}

void
kinit()
{
    uint64 startpa = PGROUNDUP((uint64)end );
    uint64 endpa   = PGROUNDDOWN( (uint64)PHYSTOP );
    uint64 span = PGROUNDDOWN( (endpa -startpa)/NCPU );

    char name[6]="kmem0";
    // printf("span:%p endpa:%p startpa:%p\n",span,endpa,startpa);
    //init lock per CPU
    for(int i=0;i<NCPU;i++){
        // printf("i:%d startpa:%p endpa:%p end:%p\n",i,i*span+startpa,(i+1)*span+startpa,end);
        name[4] = '0' + i;
        initlock(&freelists[i].lock,name);
        init_list(freelists+i,startpa+i*span,startpa+(i+1)*span);
    }

}


void
idx_kfree(int idx,void *pa){
  struct run *r;
  struct page*kmem = &freelists[idx];
  r = (struct run*)pa;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP || idx < 0 || idx >= NCPU)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 0, PGSIZE);

  push_off();  
  acquire(&kmem->lock);
  r->next = kmem->freelist;
  kmem->freelist = r;
  release(&kmem->lock);
  pop_off();
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  int idx = cpuid();
  idx_kfree(idx,pa);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void*
idx_kalloc(int idx)
{
  if(idx<0||idx>=NCPU)
    return 0;

  struct run *r;
  struct page *kmem = &freelists[idx];
  push_off();
  acquire(&kmem->lock);
  r = kmem->freelist;
  if(r){
    kmem->freelist = r->next;
    release(&kmem->lock);
    pop_off();
    memset((char*)r, 0, PGSIZE);
    return (void*)r;
  }
  else{
   release(&kmem->lock); 
   pop_off();
   return 0;
  }
}


void *
kalloc(void)
{
  int idx = cpuid();
  // printf("cpuid:%d\n",idx);
  void* p = 0;
  for(int i=0;i<NCPU;i++){
    p = idx_kalloc(idx);
    if(p)
        return p;
    else{
        // printf("cpu %d out of memory\n",idx);
        idx = (idx+1)%NCPU;
      }
  }
  return 0;
}