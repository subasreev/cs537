#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<fcntl.h>


	int
main (int argc, char *argv[])
{

	int rc,  j, len;
	char *path = strdup("/bin"); //getenv ("PATH");
	char command_line[129];
	char *params[128], *paths[128];
	char error_message[30] = "An error has occurred\n";

	//invalid arguments to shell program
	if (argv[1] != NULL)
	{
		write (STDERR_FILENO, error_message, strlen (error_message));
		exit (1);
	}

	//parse paths in PATH variable
	char *token = strtok (path, ":");
	for (j=0; token != NULL;)
	{
		paths[j++] = strdup (token);
		token = strtok (NULL, ":");
	}
	paths[j++] = NULL;

	while (1)
	{
		//print shell prompt
		write(STDOUT_FILENO, "whoosh> ", strlen("whoosh> "));
		//get user typed command from console
		fgets (command_line, 130, stdin);

		len = strlen (command_line);
		if (command_line[len - 1] != '\n')
		{
			write (STDERR_FILENO, error_message, strlen (error_message));
			int c;
			while ((c = getchar ()) != '\n' && c != EOF);
			continue;
		}
		command_line[len - 1] = '\0';

		//parse command line arguments
		token = strtok (command_line, " ");	//space as delimiter
		if (token == NULL)	//empty command
			continue;

		for(j=0; token!=NULL;)
		{
			params[j++] = strdup (token);
			token = strtok (NULL, " ");
		}
		params[j++] = NULL;

		//check and branch for built-in commands like cd
		//what behavior is expected when cd/exit/pwd with extra arguments?
		
		int ret_code;
		if (strcmp (params[0], "cd") == 0)
		{
			if (params[1] == NULL)
			{
				params[1] = getenv ("HOME");
				params[2] = NULL;
			}
			ret_code = chdir (params[1]);
			if (ret_code == -1)
			{
				write (STDERR_FILENO, error_message, strlen (error_message));
				continue;
			}
		}
		else if (strcmp (params[0], "exit") == 0)
			exit (0);
		else if (strcmp (params[0], "pwd") == 0)
		{
			char cwd[256]; //limit max number of chars in dir name to be 256 
			if (getcwd (cwd, sizeof (cwd)) != NULL){
				write(STDOUT_FILENO, strcat(cwd,"\n"), strlen(cwd)+1);
			}
		}
		else if (strcmp (params[0], "path") == 0)	//how abt path command with invalid folders?
		{
			for(j=1;params[j] != NULL;j++)
			{
				paths[j - 1] = params[j];
			}
			paths[j - 1] = NULL;
		}
		else
		{
			//find first path that contains executable for command in params[0]
			int found = -1;
			struct stat fileStat;
			char *file = NULL, *dir = NULL;
			for(j=0; paths[j] != NULL && found == -1;j++)
			{
				file =
					(char *) malloc (sizeof (char) *
							(strlen (paths[j]) + strlen (params[0]) +
							 5));
				file[0] = '\0';
				strcat (file, paths[j]);
				strcat (file, "/");
				dir = strdup(file);
				strcat (file, params[0]);
				found = stat (file, &fileStat);
			}

			if (found < 0) // executable for user program not found in path
			{
				write (STDERR_FILENO, error_message, strlen (error_message));
			}
			else
			{
				params[0] = strdup(file); //full path to user program
				rc = fork ();          //create child process for executing user program
				if (rc == 0)
				{		//child process           
					int k, code, fd=2;
					char *outfile, *errfile;

					for(k=0; params[k] != NULL; k++)
					{
						if (strcmp (params[k], ">") == 0)
						{	//output redirection with more than 1 output file or missing output file
							if (params[k + 1] == NULL || params[k + 2] != NULL)
							{
								write (STDERR_FILENO, error_message,
										strlen (error_message));
								exit (1);

							}
							//valid output file specified by user; truncate .out and .err files
							outfile = (char *)malloc(sizeof(char) *(strlen(params[k+1])+5));
							errfile = (char *)malloc(sizeof(char) *(strlen(params[k+1])+5));
							strcpy(outfile, params[k+1]);
							strcat(outfile, ".out");
							strcpy(errfile, params[k+1]);
							strcat(errfile, ".err");

							code = open (outfile, O_CREAT | O_WRONLY | O_TRUNC,
									S_IRUSR | S_IWUSR);
							//outfile open was unsuccessful
							if (code == -1)
							{
								write (STDERR_FILENO, error_message,
										strlen (error_message));
								exit (1);
							}
							else {
								//close stdout only if open() on outfile was successful
								close (STDOUT_FILENO);
								//close and reopen outfile so that it gets file descriptor of stdout
								close(code);
								code = open (outfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
								//close stderr and open errfile to reroute program error
								close(STDERR_FILENO);
								fd = open(errfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
							}

							break;
						}
					}
					//make kth element to be NULL position after redirection is removed from params
					params[k] = NULL;
					execv(params[0], params);
					write (fd, error_message,
							strlen (error_message));
					exit (1);
				}
				else if (rc > 0) //parent process
				{
					wait (NULL);
				}
				else
				{		//fork not successful
					write (STDERR_FILENO, error_message,
							strlen (error_message));
				}
			}
		}
	}
	return 0;
}
