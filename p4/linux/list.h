#ifndef __LIST_H
#define __LIST_H

typedef struct __node_t
{
  char *key, *val;
  struct __node_t *next;
} node_t;

typedef struct __list_t
{
  node_t *head;
} list_t;

int List_Empty(list_t *L);
node_t* List_Get(list_t *L);
void List_Print(list_t *L);
void List_Init(list_t *L);
int List_Insert(list_t *L, char *key, char*val);
#endif

