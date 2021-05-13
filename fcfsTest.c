#include "types.h"
#include "stat.h"
#include "user.h"


void cpuburst() {
    unsigned x = 0;
    unsigned y = 0;

    while (x < 100000) {
        y = 0;
        while (y < (10000)) {
            y++;
        }
        x++;
    }
}

void ioburst(){
    sleep(10);
}

int
main(int argc, char *argv[])
{
    int proc1, proc2;

    if ((proc1 = fork()) == 0) {
        //int pid1 = getpid();
        cpuburst();
        exit();
    }
    else if ((proc2 = fork()) == 0) {
        //int pid2 = getpid();
        ioburst();
        exit();
    }

    wait();
    wait();
    exit();
}