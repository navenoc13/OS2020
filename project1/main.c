#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include "process.h"
#include "scheduler.h"

int main(int argc, char* argv[])
{

	char sched_policy[256];
	int policy, nproc;
	struct process *proc;

	scanf("%s %d", sched_policy, &nproc);
	
	proc = (struct process *)malloc(nproc * sizeof(struct process));

	for (int i = 0; i < nproc; i++) scanf("%s%d%d", proc[i].name,&proc[i].t_ready, &proc[i].t_exec);

	if (strcmp(!sched_policy, "FIFO"))policy = FIFO;
	else if (!strcmp(sched_policy, "RR"))policy = RR;
	else if (!strcmp(sched_policy, "SJF"))policy = SJF;
	else if (!strcmp(sched_policy, "PSJF"))policy = PSJF;
	else {
		fprintf(stderr, "Invalid policy: %s", sched_policy);
		exit(0);
	}

	scheduling(proc, nproc, policy);

	exit(0);
}
