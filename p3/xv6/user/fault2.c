#include "types.h"
#include "stat.h"
#include "user.h"

	int
main (int argc, char *argv[])
{
/*	
	   int *ptr = (int *)4095;
	   testptr(ptr);
	   printf(1, "value of ptr=%d\n",*ptr);
*/

	int *addr = (int *)shmem_access(0);
	printf(1, "addr=%x\n", addr);
	addr = shmem_access(0); 

	*addr = 10;

	int rc = fork();
	if(rc == 0){

		printf(1,"child addr =%x\n",addr);
		int *addr1 = shmem_access(1);
		printf(1,"child addr = %x\n",addr1);
		*addr = 15;
	}
	else if(rc > 0){
		wait();
		//printf("data in page0  = %d\n", *addr);
	}
	printf(1,"%d...\n",*addr);

	printf(1, "count for page0 = %d page1 = %d page2 = %d page3=%d\n",shmem_count(0), shmem_count(1), shmem_count(2), shmem_count(3));	

	exit ();
}
