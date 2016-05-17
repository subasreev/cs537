#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

	void
test_failed()
{
	printf(1, "TEST FAILED\n");
	exit();
}

	void
test_passed()
{
	printf(1, "TEST PASSED\n");
	exit();
}

#define NBLOCKS (NDIRECT+1)
#define SIZE NBLOCKS*4

	int
main(int argc, char *argv[])
{
	int fd, i;
	char buf[SIZE];
	char buf2[SIZE];
	char tmp;
	struct stat st;

	for(i = 0; i < SIZE; i++){
		buf[i] = (char)(i+(int)'0');
	}
	memset(buf2, 0, SIZE);


	for(i = 0; i < SIZE+5; i++){
		printf(1,"%d iteration\n",i);
		if((fd = open("test_file.txt", O_CREATE | O_SMALLFILE | O_RDWR)) < 0){
			printf(1, "Failed to create the small file\n");
			test_failed();
		}

		while(read(fd, &tmp, 1) == 1) 
		{
		} //go to end of file
		int ret_val;
		if((ret_val=write(fd, &buf[i], 1)) != 1){
			break;
		}
		close(fd);
	}


	if((fd = open("test_file.txt", O_CREATE | O_SMALLFILE | O_RDWR)) < 0){
		printf(1, "Failed to open the small file\n");
		test_failed();
	}

	if(fstat(fd, &st) < 0){
		printf(1, "Failed to get stat on the small file\n");
		test_failed();
	}

	if(st.size != SIZE){
		printf(1, "Invalid file size %d\n", st.size);
		test_failed();
	}

	if(read(fd, buf2, SIZE) != SIZE){
		printf(1, "Read failed!\n");
		test_failed();
	}
	close(fd);

	for(i = 0; i < SIZE; i++){
		if(buf[i] != buf2[i]){
			printf(1, "Data mismatch %c %c\n", buf[i], buf2[i]);
			test_failed();
		}
	}

	test_passed();
	exit();
}
