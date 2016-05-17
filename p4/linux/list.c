#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

typedef struct __node_t
{
  char *key, *val;
  struct __node_t *next;
} node_t;

// basic list structure (one used per list)
typedef struct __list_t
{
  node_t *head;
} list_t;

void
List_Init (list_t * L)
{
  L->head = NULL;
}

int List_Empty(list_t *L){
  int val = 0;
  if(L->head == NULL) 
  	val = 1;
  return val;
}

int
List_Insert(list_t * L, char *key, char *val)
{
  node_t *new = (node_t *) malloc (sizeof (node_t));
  if (new == NULL)
    {
      perror ("malloc");
      return -1;		// fail
    }
  new->key = strdup (key);
  new->val = strdup (val);
  new->next = NULL;
  if (L->head == NULL)
    L->head = new;
  else
    {
      node_t *temp = L->head;
      while (temp->next != NULL)
	temp = temp->next;
      temp->next = new;
    }
  return 0;			// success
}

node_t* List_Get(list_t *L){

  node_t *entry;

  if(L == NULL || L->head == NULL)
	entry =  NULL;
  else{
	node_t *cur = L->head;
	L->head = cur->next;
	entry = cur;
	//val = strdup(cur->key);
	//free(cur);	
  }
  return entry;
}

void
List_Print (list_t * L)
{
  node_t *temp = L->head;
  while (temp != NULL)
    {
      printf ("node val: %s\n", temp->key);
      temp = temp->next;
    }
}

int
List_Lookup (list_t * L, char *key)
{
  node_t *curr = L->head;
  while (curr)
    {
      if (strcmp (curr->key, key) == 1)
	{
	  return 0;		// success
	}
      curr = curr->next;
    }
  return -1;			// failure
}

/*
int
main (int argc, char *argv[])
{
  list_t *l = (list_t *) malloc (sizeof (list_t));
  List_Init (l);
  char *str = "hello";
  List_Insert (l, str);
  List_Print(l); 
  return 0;
}
*/
