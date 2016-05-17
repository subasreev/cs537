

Author: Subasree Venkatsubhramaniyen

------------------------------------
Part 1a: Sorting Program 
File: fastsort.c

Description:

The code reads in a filename and an optional argument (word column to sort the file) from command line and prints the sorted lines as output. Valid format to execute the program is as below:
./fastsort [ -3] filename

In the above example, 3 represents the word column by which the file is to be sorted. Lines with < 3 words are sorted by last word. If the optional argument is missing, then by default the file
is sprted by the first word in each line.  

Code:

The program processes the lines in the input file by storing in a struct with key and value being the the word column and the line respectively. To store the lines in a static data structure, max number
of lines is required, else we can use linked list. However, usage of linked list slows the program due to random memory access. Alos, predefined sorting algo has be effficient on linked list. Hence, the 
program can rather read the file twice, (once to count number of lines) and another time to store the lines in array of structure. However, stat/fstat function is used to find out the number of bytes in 
the file and allocate array of structures accordingly.  strtok() function is used to tokenize the line and obtain the key by which 
lines are to be sorted.

Appropriate error message are printed on stderr due to various exceptions. To speed up the program execution, few optimizations have been included, namely custom quickSort (cache-friendly 3-way radix quick sort) function
has been defined. Also, prefix operators are used intead of postfix operators. Unnecessary if conditions and c functions like strlen have been avoided. 


----------------------------------
Part 1b: System call addition

The idea is to add a system call getprocs() in xv6 kernel. The system call is required to return the number of processes in the system. The problem ssttement requiresintroduction of code in various files in
xv6 code base. Below are some code changes:

user/usys.S        - Code to invoke the new system call 			- SYSCALL(getprocs)
include/syscall.h  - Code to define the new system call 			- #define SYS_getprocs 22
kernel/syscall.c   - Code to add function pointer for the new system call	- [SYS_getprocs] sys_getprocs
kernel/proc.c	   - Code to define the body of the new system call routine     - Code to increment number of processes with state = UNUSED and return NPROC - counter
kernel/sysfunc.h   - Code to add system call handler				- int sys_getprocs(void);
user/user.h        - Code to add the new system call in user exectabe commands  - int getprocs(void);
user/getprocs.c    - Code to test the newly added system call                   - Code to invoke getprocs()

 
 

    

 


