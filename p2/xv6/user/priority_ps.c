#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main (int argc, char *argv[])
{
  if (argc != 2)
    exit ();
  int rc = fork ();
  if (rc < 0)
    {
      printf (1, "error\n");
    }
  else if (rc == 0)
    {
      int i, x = 0;		//child process
      printf (1, "child entry\n");
      for (i = 1; i < atoi (argv[1]); i++)
	{
	  x++;
	  if(x == 10000){
	  setpri(2);
	  printf(1, "in middle\n");
	  struct pstat proc_stat;
	  getpinfo (&proc_stat);

	  int i;
	  for (i = 0; i < NPROC; i++)
	    {
	      if (proc_stat.pid[i] == 3 || proc_stat.pid[i] == 4)
		{
		  printf (1, "proc inuse=%d\n", proc_stat.inuse[i]);
		  printf (1, "proc id=%d\n", proc_stat.pid[i]);
		  printf (1, "proc hticks=%d\n", proc_stat.hticks[i]);
		  printf (1, "proc lticks=%d\n", proc_stat.lticks[i]);
		}
	    }
	  }
	}
      printf (1, "child exiting with pid %d\n", getpid ());
	getprocs();
    }
  else				//parent process
    {
      int i, x = 0;
      printf (1, "parent entry\n");
      for (i = 1; i < atoi (argv[1]); i++)
	{
	  x++;
	  if(x == 9999)
		setpri(2);
	}
      printf (1, "parent exiting with pid %d\n", getpid ());
	getprocs();
    }


  exit ();
}
