#ifndef __HASHTABLE_H
#define __HASHTABLE_H

typedef struct _lst_t_ {
	    char *string;
	        struct _lst_t_ *next;
} lst_t;

typedef struct _hash_table_t_ {
	    int size;       /* the size of the table */
	        lst_t **table; /* the table elements */
} hash_table_t;

hash_table_t *create_hash_table(int size);
unsigned int hash(hash_table_t *hashtable, char *str);
lst_t *lookup_string(hash_table_t *hashtable, char *str);
int add_string(hash_table_t *hashtable, char *str);
lst_t *lookup_string(hash_table_t *hashtable, char *str);
int add_string(hash_table_t *hashtable, char *str);

#endif 

