#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "param.h"

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

int
main(int argc, char *argv[])
{
  char *ptr;
  int i;
  
  ptr = shmem_access(3);
  if (ptr == NULL) {
    test_failed();
  }
  
  char arr[4] = "tmp";
  for (i = 0; i < 4; i++) {
    *(ptr+i) = arr[i];
  }
  
  //argstr
  int fd = open(ptr, O_WRONLY|O_CREATE);
  if (fd == -1) {
    printf(1, "open system call failed to take a string from within a shared page\n");
    test_failed();
  }
  
  //argptr
  int n = write(fd, ptr, 10);
  if (n == -1) {
    printf(1, "write system call failed to take a pointer from within a shared page\n");
    test_failed();
  }
  
  //making sure invalid strings are still caught
  int fd2 = open((char *)(USERTOP/2), O_WRONLY|O_CREATE);
  if (fd2 != -1) {
    printf(1, "open system call successfully accepted an invalid string\n");
    test_failed();
  }
  
  //making sure invalid pointers are still caught
  n = write(fd, (char *)(USERTOP/2), 10);
  if (n != -1) {
    printf(1, "write system call successfully accepted an invalid pointer\n");
    test_failed();
  }
  
  //making sure edge case is checked
  printf(1, "%d\n", ptr);
  n = write(fd, (char *)(ptr + 4094), 3);
  if (n != -1) {
    printf(1, "write system call successfully accepted an overflowing pointer in a shared page\n");
    test_failed();
  }
  
  test_passed();
  exit();
}

