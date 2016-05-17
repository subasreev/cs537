Author: Subasree Venkatsubhramaniyen

-----------------------------------

PART1: Shell
The program is intended to mimic a simple shell that can execute user programs (like ls, cat etc) and built-in commands namely pwd, cd, path, exit.
The program takes no command line arguments. Upon execution, it prompts for user input command.

Implementation: 
2-process approach to handle the shell.
Parent process handles the shell part i.e gets user input, parses the command and classifies as built-in/user program.
If it is not a built-in command, then it looks for the exeutable for the input user program in the paths given by PATH variable (initially set to /bin)
If no executable is found, then an aerror message is displayed. If found, then child process is created to execute it.

A child process is created using fork() system call to execute the user program
Handles input redirection to write the output of the user program to output file appended by .out mentioned in the user command.
Error messages are redirected to .err appended to user input file.

----------------------------------

PART2: xv6
Requirement is to add 2 new system calls, namely setpriority() and getpinfo()
Scheduler to implement priority scheduling

This causes introduction of new attributes namely priority, inuse, lticks, hticks to the header file proc.h

in allocproc() function, priority=1 and inuse=1 are set as default when a process is created.

Also, proc.c requires changes in scheduler() function, where any process of priority=2 in a ready state is required to be scheduled.

If there exists only a running process which is of priority=2, then it is scheduled.
Else, in the absence of priority=2 process, a ready process with priority=1 is scheduled.
This continues for every clock tick.
Whenever, priority=2 process is scheduled, hticks of that process is incremented, else lticks is incremented.

In getpinfo() system call, the argument structure paramters are filled simply by accessing the process attributes by iterating through the process table.


