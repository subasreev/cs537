#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

//structure that defines a record  key: word in inputted column and value being the entrire line
struct char_map{
	char *key;
	char *value;
};
typedef struct char_map c_map;
#define malloc_error "malloc failed\n"
#define bad_command_error "Error: Bad command line parameters\n"
#define line_long_error "Line too long\n"
#define file_error "Error: Cannot open file"

//three-way quick sort 
void quickSort(c_map *a, int lo, int hi, int d) {
		if (hi <= lo)
			return;
		int lt = lo, gt = hi;
		int v = a[lo].key[d]; //get the dth character from the key
		int i = lo + 1;
		while (i <= gt) {
		
			int t = a[i].key[d];
			if (t < v) {
				//swap the record in the structure 
				c_map temp = a[lt];
				a[lt] = a[i];
				a[i] = temp;
				++lt; //prefix operators seem to be faster than postfix
				++i;
			} else if (t > v) {
				c_map temp = a[i];
				a[i] = a[gt];
				a[gt] = temp;
				--gt;
			} else
				++i;
		}
		quickSort(a, lo, lt - 1, d);
		if (v >= 0)
			quickSort(a, lt, gt, d + 1);
		quickSort(a, gt + 1, hi, d);
}	

int
main(int argc, char *argv[])
{
    // arguments
    char *inFile = "";
    char  *col_param = "";
    int column_num = 1;	 //default word column for sort operation

     // input params - expects exactly 2/3 parameters
    if(argc < 2 || argc > 3){
	fprintf(stderr, bad_command_error);
	exit(1);
    }

    if(argc == 2){
	inFile = argv[1];  //strdup(argv[1]);
    }
    else {
        int isValid = 0;
	char *pend;
	inFile =  argv[2];  //strdup(argv[2]);
        col_param = argv[1]; //strdup(argv[1]);

 	// check for format < -number >
	if(strlen(col_param) >=2 && col_param[0] == '-'){ 
		column_num = strtol(col_param+1, &pend, 10);
		if(*pend == '\0') {
			isValid = 1;
		}
        } 
	if(isValid == 0){
		 fprintf(stderr, bad_command_error);
        	 exit(1);
	}
		
    }	

    // open and create output file
    FILE *fp = fopen(inFile, "r");
    if (fp == NULL) {
	fprintf(stderr, "%s %s\n", file_error, inFile);	
	exit(1);
    }

    struct stat fileStat;
    if(stat(inFile, &fileStat) < 0)    {
        exit(1);
    }

    int fsize = fileStat.st_size;
    c_map *lines = (c_map *) malloc(fsize * 2);
    if(lines == NULL)
	fprintf(stderr, malloc_error);

	
    //read the file contents and store the contents in a struct

    char *line = (char *) malloc(130 * sizeof(char));
    if(line == NULL)
	fprintf(stderr, malloc_error);	

    int i = 0, j;	

    while(1){
	//int read_ctr;
        if(fgets(line, 129 , fp) == NULL) 
		break;

	int line_len = strlen(line);
	if(line[line_len - 1] != '\n'){
		fprintf(stderr, line_long_error);
		exit(1);
	}
      
        //remove the newline character at the end of the line
        line[line_len - 1] = '\0';
        lines[i].value = strdup(line); 

        char *token = strtok(line, " ");
	char *last_token = (char *)malloc(sizeof(char) * (line_len + 1 ));
        if(last_token == NULL)
		fprintf(stderr, malloc_error);
	int ctr = 1;
	if(token == NULL)
		ctr = 0; 

        //tokenize the line to get the appropriate word as the key
	while(token != NULL && ctr < column_num){
		strcpy(last_token, token);	
		token = strtok(NULL, " ");
		++ctr;
        }
	if(token!=NULL){
		strcpy(last_token, token);
	}

	//assign key to the record
        lines[i].key = strdup(last_token);
   	++i; 
    }   	
    
    fclose(fp);	
    free(line);	

    //sort lines by the key
    quickSort(lines, 0, i-1, 0);

    //print sorted output to console
    for(j = 0; j < i; ++j){
        printf("%s\n", lines[j].value);
    }


    return 0;
}


