#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

	int
main(int argc, char *argv[])
{
	struct pstat proc_stat;

	getpinfo(&proc_stat);

	int i;
	for(i=0;i<NPROC;i++){
		if(proc_stat.inuse[i]==1){
			printf(1, "proc inuse=%d\n",proc_stat.inuse[i]);
			printf(1,"proc id=%d\n",proc_stat.pid[i]);	
			printf(1,"proc hticks=%d\n",proc_stat.hticks[i]);
			printf(1,"proc lticks=%d\n",proc_stat.lticks[i]);
		}
	}
	exit();
}
