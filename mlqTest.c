#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  int mlqPriority, pid;
  if(argc != 3){
    printf(2,"Bad Arguments\nUsage: $nice pid mlqPriority\n");
    exit();
  }
  // Convert strings to int
  pid = atoi(argv[1]);
  mlqPriority = atoi(argv[2]);
  // Priority should be between 0 to 50
  if (mlqPriority < 0 || mlqPriority > 3){
    printf(2,"Invalid mlqPriority (1/ 2/ 3)!\n");
    exit();
  }
  // Call changePriority internally to change the mlqPriority of a process
  changePriority(pid, mlqPriority);
  exit();
}