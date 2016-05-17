#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
   int num_procs = getprocs();
   printf(1,"number of procs=%d\n",num_procs); 
   exit();
}
