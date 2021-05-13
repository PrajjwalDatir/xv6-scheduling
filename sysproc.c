#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
}ptable;
struct processInfo
{
  int ppid;
  int psize;
  int numContextSwitches;
};

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Idea of argint taken from : https://stackoverflow.com/questions/27068394/how-to-pass-a-value-into-a-system-call-function-in-xv6

//function to set the tickets for the lottery test
int
sys_setTickets(void)
{
  int numTickets;
  if(argint(0,&numTickets) < 0){
    myproc()->tickets = 10;
  }
  else{
    myproc()->tickets = numTickets;
  }
 return 0;
}

int
sys_getNumProc(void)
{
  struct proc *p;
  int count = 0;
  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC];++p)
  {
    if(p->state != UNUSED)
      count++;
  }
  release(&ptable.lock);

  return count;
}

int
sys_getMaxPid(void)
{
  struct proc *p;
  int maximum = 0;
  acquire(&ptable.lock);

  for(p=ptable.proc;p<&ptable.proc[NPROC];++p)
  {
    if(p->state != UNUSED && p->pid > maximum)
      maximum = p->pid;
  }

  release(&ptable.lock);

  return maximum;
}

int
sys_getProcInfo(void)
{
  int pid;
  struct processInfo *info;
  struct proc *p;

  if(argint(0,&pid) < 0)
    return -1;

  if(argptr(1, (void*)&info, sizeof(info)) < 0)
    return -1;

  acquire(&ptable.lock);
  int found = 0;

  for (p = ptable.proc; p < &ptable.proc[NPROC]; ++p)
  {
    if(p->pid == pid)
    {
      info->ppid = 0;

      // check if process present with given pid
      if(p->parent != 0)
      {
        info->ppid = p->parent->pid;
      }

      info->psize = p->sz;
      info->numContextSwitches = p->contextSwitches;
      found = 1; // process with given pid found
      break;
    }
  }

  release(&ptable.lock);

  // process with given pid not found
  if(found == 0)
    return -1;

  return 0;
}

int
sys_getPriority(void)
{
  return myproc()->priority;
}

int
sys_setPriority(void)
{
  int priority;
  if(argint(0,&priority) < 0)
    return -1;

  myproc()->priority = priority;

  return 0;
}

int
sys_getTime(void) {
  struct rtcdate *d;
  if (argptr(0, (char **)&d, sizeof(struct rtcdate)) < 0)
      return -1;
  cmostime(d);
  return 0;
}

int
sys_currentProcessState(void)
{
  struct proc *p;
  //Enables interrupts on this processor.
  sti();

  // Acquire lock for ptable
  // Loop over process table and print name, state, pid of process.
  acquire(&ptable.lock);
  cprintf("\n\nname \t pid \t state \t priority\n");
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
  	if(p->state == RUNNABLE)
   	  cprintf("%s \t %d \t RUNNABLE \t %d\n", p->name, p->pid, p->priority);
  	else if(p->state == RUNNING)
   	  cprintf("%s \t %d \t RUNNING \t %d\n", p->name,p->pid, p->priority);
    else if(p->state == SLEEPING)
  	  cprintf("%s \t %d \t SLEEPING \t %d\n", p->name,p->pid, p->priority);
  }
  cprintf("\n\n");
  // Release The Lock
  release(&ptable.lock);
  return 22;
}

int
sys_changePriority(int pid, int priority)
{
	struct proc *p;
  if(priority < 0 || priority > 3){
    priority = 2;
  }
	acquire(&ptable.lock);
  // Loop over the process table and select the process
  // with given pid. Change its priority to given priority.
	for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
	  if(p->pid == pid){
			p->mlqPriority = priority;
			break;
		}
	}
	release(&ptable.lock);
	return pid;
}

int
sys_watchChildren(struct pstat *data)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  int waitTime, runTime, totalTime;

  acquire(&ptable.lock);
  for (;;)
  {
    // Scan through table looking for exited children.
    havekids = 0;
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    {
      if (p->parent != curproc)
        continue;
      havekids = 1;
      if (p->state == ZOMBIE)
      {
        // Found one.

        //Update times
        waitTime = p->endTime - p->creationTime - (p->runTime)*0.01;
        runTime = (p->runTime)*0.01;
        totalTime = p->endTime - p->creationTime;

        data->totalRunTime += runTime;
        data->totalTime += totalTime;

        // cprintf("%d  total runtime  %d, total tat  %d\n",p->pid, data->total_runtime, data->total_tat);
        cprintf("%d    %d    %d    %d    %d\n",p->pid, p->priority, waitTime, runTime, totalTime);
        //cprintf("total runtime  %d, total tat  %d\n", *total_runtime, *total_tat);

        if(p->endTime > data->maxEndTime){
          data->maxEndTime = p->endTime;
        }
        if(p->creationTime < data->minCreationTime){
          data->minCreationTime = p->creationTime;
        }

        //Clean up times
        p->creationTime = 0;
        p->endTime = 0;
        p->runTime = 0;
        p->sleepTime = 0;

        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if (!havekids || curproc->killed)
    {
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock); //DOC: wait-sleep
  }
}
