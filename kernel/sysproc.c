#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"


uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  struct proc *p = myproc();
  unsigned int abits=0;

  uint64 addr;
  argaddr(0, &addr); //Đọc địa chỉ base từ tham số đầu tiên
  
  int num;
  argint(1,&num); //Đọc số trang từ tham số thứ hai

  uint64 dest;
  argaddr(2, &dest); // Đọc địa chỉ mask từ tham số thứ ba
    
  //Duyệt qua từng trang
  for(int i=0;i<num;i++){
    uint64 query_addr = addr + i * PGSIZE ; // Địa chỉ ảo của trang thứ i

    //để tìm PTE tương ứng
    //với địa chỉ ảo query_addr trong Page Table của tiến trình hiện tại.
    pte_t * pte=walk(p->pagetable, query_addr, 0);  

    // Nếu PTE không tồn tại (NULL) HOẶC PTE chưa hợp lệ, bỏ qua.
    if (pte == 0 || (*pte & PTE_V) == 0) {
        continue; 
    }

    //kiểm tra xem bit PTE_A trong PTE có đang được đặt là 1 hay không
    //nếu có, trang này đã được truy cập
    if(*pte&PTE_A)
    {
      abits=abits|(1<<i); //Ghi nhận thông tin trang i đã được truy cập vào biến abits
      *pte=(*pte)&(~PTE_A); //Xóa bit truy cập PTE_A trong PTE để đánh dấu rằng trang đã được truy cập.
    }
  }

  //Sao chép nội dung của bitmask abits (được lưu trữ trong kernel space) sang địa chỉ user space là dest.
  if(copyout(p->pagetable, dest, (char*)&abits, sizeof(abits)) < 0)
    return -1;

  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
