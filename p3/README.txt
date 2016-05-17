Author: Subasree Venkatsubhramaniyen

-----------------------------------

PART1: xv6 - NULL pointer dereference 
The program is intended to allocate virtual pages starting from page 1 i.e not allocating page 0 cause page 0 to be invalidated.
Hence, access to page 0 would trap into the kernel, killing the process

Implementation: 
changed the below files:
kernel/vm.c - chnaged copyuvm() to copy address space from page1 instead of page0
kernel/exec.c - changed code to load program from page1 by skipping first page
kernel/syscall.c - incorporated condition checks to handle invalid addresses that refer to page0
user/makefile.mk - changed code to load program from address 0x1000 (decimal 4096) which is the address of 2nd page
----------------------------------

PART2: xv6 - Shared Pages 
Requirement is to add 2 new system calls, namely shmem_access(page_number) and shmem_count(page_number)

This causes introduction of new attributes namely is_shared(array of 4 elements to track whether process is sharing page i) and
shared_pg_beg(beginning of shared address page) to the header file proc.h

proc.c
in allocproc() function, is_shared is cleared to 0 and shared_pg_beg is set to USERTOP (as nothing is being shared) are set as default when a process is created.
fork() and growproc() functions - changed code to manage shared pages
new system calls shmem_access() and shmem_count() - logic to update variables for shared pages

syscall.c - modified conditions and checks to handle arguments to system calls 

vm.c
A new variable shared_ps_cnt to track the count of processes sharing pages 1-4 is created
added 4 new functions:
shmem_init() - invoked once upon boottime to grab 4 physical pages
mapva2pa() - helper function to map logical address to one of the 4 physical pages based on the argument page_number
update_shmem_ps_cnt() - takes an page_number and offset as arguments and updates the shared_ps_nt variable with the offset (increment/decrement count)
get_shmem_ps_cnt() - returns the value at page_number index of the shared_ps_cnt array


