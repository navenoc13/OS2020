#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <sys/types.h>

#define CHILD_CPU 1
#define PARENT_CPU 0

//One UNIT TIME
#define UNIT_T()				\
{						\
	volatile unsigned long i;		\
	for (i = 0; i < 1000000UL; i++);	\
}						\

struct process {
	char name[32];
	int t_ready;
	int t_exec;
	pid_t pid;
};

//Assing to specific core
int proc_assign_cpu(int pid, int core);

//Execute process and return pid
int proc_exec(struct process proc);

//Set low priority
int proc_block(int pid);

//Set high priority
int proc_wakeup(int pid);

#endif
