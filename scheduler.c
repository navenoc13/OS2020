#define _GNU_SOURCE
#include "process.h"
#include "scheduler.h"
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sched.h>

static int t_last;
static int ntime;
static int running;
static int finish_cnt;



int next_process(struct process *proc, int nproc, int policy)
{
	// Non-preemptive
	if (running != -1 && (policy == SJF || policy == FIFO))
		return running;

	int ret = -1;


	if (policy == FIFO) {
		for(int i = 0; i < nproc; i++) {
			if(proc[i].pid == -1 || proc[i].t_exec == 0)
				continue;
			if(ret == -1 || proc[i].t_ready < proc[ret].t_ready)
				ret = i;
		}
    }

	else if (policy == PSJF || policy ==  SJF) {
		for (int i = 0; i < nproc; i++) {
			if (proc[i].pid == -1 || proc[i].t_exec == 0)
				continue;
			if (ret == -1 || proc[i].t_exec < proc[ret].t_exec)
				ret = i;
		}
	}


	else if (policy == RR) {
		if (running == -1) {
			for (int i = 0; i < nproc; i++) {
				if (proc[i].pid != -1 && proc[i].t_exec > 0){
					ret = i;
					break;
				}
			}
		}
		else if ((ntime - t_last) % 500 == 0)  {
			ret = (running + 1) % nproc;
			while (proc[ret].pid == -1 || proc[ret].t_exec == 0)
				ret = (ret + 1) % nproc;
		}
		else ret = running;
	}

	return ret;
}

//Sort by ready time
int cmp(const void *a, const void *b) {
	return ((struct process *)a)->t_ready - ((struct process *)b)->t_ready;
}

int scheduling(struct process *proc, int nproc, int policy)
{
	qsort(proc, nproc, sizeof(struct process), cmp);

	for (int i = 0; i < nproc; i++)
		proc[i].pid = -1;

	proc_assign_cpu(getpid(), PARENT_CPU);
	proc_wakeup(getpid());
	
	ntime = 0;
	running = -1;
	finish_cnt = 0;
	
	while(1) {
		//Running process finish or not?
		if (running != -1 && proc[running].t_exec == 0) {
			//kill(running, SIGKILL);
			waitpid(proc[running].pid, NULL, 0);
			printf("%s %d\n", proc[running].name, proc[running].pid);
			running = -1;
			finish_cnt++;

			//All process finish
			if (finish_cnt == nproc)
				break;
		}

		//Process ready to execute?
		for (int i = 0; i < nproc; i++) {
			if (proc[i].t_ready == ntime) {	
				proc[i].pid = proc_exec(proc[i]);
				proc_block(proc[i].pid);
				
			}

		}

		//Select next process, context switch
		int next = next_process(proc, nproc, policy);
		if (next != -1) {
			if (running != next) {
				proc_wakeup(proc[next].pid);
				proc_block(proc[running].pid);
				running = next;
				t_last = ntime;
			}
		}

		//A unit time
		UNIT_T();
		if (running != -1)proc[running].t_exec--;
		ntime++;
	}

	return 0;
}
