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

#define NITERATIONS 1000

	int
main(int argc, char *argv[])
{
	int fd;
	char buf[NITERATIONS];
	char result; //character read from file
	int n;
	int i;

	for(i = 0; i < NITERATIONS; i++){
		buf[i] = (char)(i+(int)'0');
	}


	for(i = 0; i < NITERATIONS; i++){
		if((fd = open("test_file.txt", O_CREATE | O_SMALLFILE | O_RDWR)) < 0){
			printf(1, "Failed to create the small file\n");
			test_failed();
		}
		if((n = write(fd, &buf[i], 1)) != 1){
			printf(1, "Write failed!\n");
			test_failed();
		}
		close(fd);
	}


	if((fd = open("test_file.txt", O_CREATE | O_SMALLFILE | O_RDWR)) < 0){
		printf(1, "Failed to open the small file\n");
		test_failed();
	}
	if((n = read(fd, &result, 10)) != 1){
		printf(1, "Read failed! %d\n", n);
		test_failed();
	}
	close(fd);

	if(result != buf[NITERATIONS-1]){
		printf(1, "Data mismatch.\n");
		test_failed();
	}

	test_passed();
	exit();
}

