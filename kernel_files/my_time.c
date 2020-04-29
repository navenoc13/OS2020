#include <linux/ktime.h>
#include <linux/kernel.h>
#include <linux/timekeeping.h>
#include <linux/linkage.h>

asmlinkage int sys_my_time(unsigned long *s_sec, unsigned long *s_nsec, 
							unsigned long *e_sec,unsigned long *e_nsec,
							int BE, int *pid
						) {
  struct timespec t;
  //Record Kernel Time (beginning and end)
  getnstimeofday(&t);

//if Beginning
  if (BE) {
    *s_sec = t.tv_sec;
    *s_nsec = t.tv_nsec;
  } 
  else {
    *e_sec = t.tv_sec;
    *e_nsec = t.tv_nsec;
    printk("[Project1] %d %lu.%09lu %lu.%09lu\n", *pid, *s_sec, *s_nsec, *e_sec, *e_nsec);
  }
  return 0;
}
