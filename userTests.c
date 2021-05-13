//Timer program that uses waitx syscall
#include "types.h"
#include "stat.h"
#include "user.h"

//Dummy tasks to consume time
void cputask(){
    float increment = 2, num = 0, limit = 900000000;
    for (float j = 0; j < limit; j+=increment){
        num += 179.326 * 48.5;
    }
}

void iotask(){
    int increment = 2, num = 0, limit = 200;
    for (int j = 0; j < limit; j+=increment){
        num = num + j;
        if(j%10 == 0)
            sleep(1);
    }
}

void mlqTest(){
    int count = 20;
    int pid = 0;
    int mlqPriority = 0;
    for (int i = 0; i < count; i++){
        pid = fork();
        if(pid == 0){
            mlqPriority = i%3 + 1;
            pid = getpid();
            changePriority(pid,mlqPriority);
            if(i%3 == 0)
                iotask();
            else
                cputask();
            exit();
        }
        else{
            continue;
        }
    }
    struct pstat data;
    data.maxEndTime = 0, data.minCreationTime = 666666;
    printf(1,"PID  Prio  WT  RT  TAT\n");
    for (int j = 0; j < count; j++){
        //int waittime,runtime;
        int status;
        status = watchChildren(&data);
        status += 0;
    }

    int total_time = data.maxEndTime - data.minCreationTime;
    int avg_tat = data.totalTime / count;
    int throughput = (count * 1000) / total_time;
    int cpu_util = (data.totalRunTime * 100)/total_time;
    printf(1,"\n\n Multi Level Queue\n");
    printf(1,"Avg Total Time  %d sec,  Throughput  %d process/msec,  CPU Utilization  %d %%\n", avg_tat ,throughput , cpu_util);
}


int main(int argc, char* argv[])
{
    mlqTest();
    exit();
}
