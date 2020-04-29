#define _GNU_SOURCE
#include "process.h"
#include <sched.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h>

int proc_assign_cpu(int pid, int core){
	if (core > sizeof(cpu_set_t)) {
		fprintf(stderr, "Core index error.");
		return -1;
	}

	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(core, &mask);
		
	if (sched_setaffinity(pid, sizeof(mask), &mask) < 0) {
		perror("sched_setaffinity");
		exit(1);
	}

	return 0;
}

int proc_exec(struct process proc){
	int pid = fork();
	int this_pid;

	if (pid < 0) {
		perror("fork");
		return -1;
	}

	if (pid == 0) {
		unsigned long s_sec, s_nsec, e_sec, e_nsec;

		this_pid = getpid();
		syscall(333, &s_sec, &s_nsec, &e_sec, &e_nsec, 1,&this_pid);
		for (int i = 0; i < proc.t_exec; i++) {
			UNIT_T();
		}

		syscall(333, &s_sec, &s_nsec, &e_sec, &e_nsec,0, &this_pid);
		exit(0);
	}
	
	//Prevent a child interrupted by Parent, assign to another core
	proc_assign_cpu(pid, CHILD_CPU);

	return pid;
}

int proc_block(int pid){
	struct sched_param param;
	param.sched_priority = 0;

	int ret = sched_setscheduler(pid, SCHED_IDLE, &param);
	
	if (ret < 0) {
		perror("sched_setscheduler");
		return -1;
	}

	return ret;
}

int proc_wakeup(int pid){
	struct sched_param param;
	param.sched_priority = 0;

	int ret = sched_setscheduler(pid, SCHED_OTHER, &param);
	
	if (ret < 0) {
		perror("sched_setscheduler");
		return -1;
	}

	return ret;
}
