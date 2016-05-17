#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define size (60)

int
main(int argc, char *argv[])
{
	int fd;
       	int i;
	char buf[size];

	
	fd = open("foobar.txt", O_CREATE|O_RDWR);
	char str[size];

	for(i=0;i<15;i++)
		str[i] = 'a';
	for(;i<30;i++)
		str[i]='b';
	for(;i<size-1;i++)
		str[i]='c';
	str[i] = '\0';

	int rc = write(fd, str, size);
	printf(1,"1st write %d\n", rc);

	rc = write(fd, str, size*2);
	printf(1,"2nd write %d\n",rc);
	close(fd);
	

	fd = open("foobar.txt", O_SMALLFILE|O_RDONLY);
	if(fd < 0)
		printf(1,"file error\n");
	int read_bytes = read(fd,buf,size);
	printf(1,"bytes read=%d\n",read_bytes);
	close(fd);
	for(i =0;i<size;i++)
		printf(1,"%x",buf[i]);	
	printf(1,"string=%s\n",buf);
	printf(1,"successful\n");
	exit();
}
