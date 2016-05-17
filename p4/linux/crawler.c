#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include "list.h"
#include "hashtable.h"
#include<regex.h>
#include<limits.h>


int QUEUE_SIZE;
list_t *queue_pages;
char **queue_links;

int fill_ptr = 0;
int use_ptr = 0;
int count = 0;
int sum_queue_lengths = 0; //keep track of end condition

pthread_mutex_t glock, lock1, lock2;	//glock for sum_queue_lengths, lock1 for bounded queue, lock2 for unbounded queue
pthread_cond_t c, fill, empty, fill_unbounded;
hash_table_t *urls;

void
put (char *value)
{
  queue_links[fill_ptr] = strdup (value);
  fill_ptr = (fill_ptr + 1) % QUEUE_SIZE;
  count++;
}

char *
get ()
{
  char *tmp = queue_links[use_ptr];
  use_ptr = (use_ptr + 1) % QUEUE_SIZE;
  count--;
  //q1_ctr--;
  return tmp;
}

int
areQueuesEmpty ()
{
  int val;
  pthread_mutex_lock (&glock);
  val = (sum_queue_lengths == 0) ? 1 : 0;
  pthread_mutex_unlock (&glock);
  //printf("checking for end condition %d\n",val);
  return val;
}


void
thr_join ()
{
  //printf("in thread join\n");
  pthread_mutex_lock (&glock);
  int done = (sum_queue_lengths == 0) ? 1 : 0;
  while (done == 0){
    pthread_cond_wait (&c, &glock);
    done = (sum_queue_lengths == 0) ? 1 : 0;
  }
  //printf("exiting thread join\n");
  pthread_mutex_unlock (&glock);
}

void
thr_exit ()
{
  //printf("in thread exit\n");
  pthread_mutex_lock (&glock);
  int done = (sum_queue_lengths == 0) ? 1 : 0;
  if (done == 1){ 
    //printf("signaling main crawler thread\n");
    pthread_cond_signal (&c);
  }
  pthread_mutex_unlock (&glock);
  //printf("signaled main thread\n");
}

void *parse (void * context)
{
  void (*_edge_fn) (char *from, char *to) = context;
  char *url, *page;
  node_t *entry;

  while (!areQueuesEmpty ())
    {
      //critical section: dequeue page from shared unbounded queue acting as consumer
      //printf("in parser\n");

      pthread_mutex_lock(&lock2);
      while (List_Empty(queue_pages))
	pthread_cond_wait (&fill_unbounded, &lock2);
      entry = List_Get (queue_pages);
      pthread_mutex_unlock(&lock2);

      url = entry->key;
      page = entry->val; 
      //printf("parsing %s\n",url);


      //core logic for parsing a page
      char *save_ptr1, *token;
      char *delim = " \n\t";
      regex_t regex;
      char *expr = "^link:.*";
      int reti = regcomp (&regex, expr, 0);
      if (reti)
	{
	  //printf (stderr, "Could not compile regex\n");
	  exit (1);
	}

      char *str1 = page;
      int has_atleast_a_link = 0;
      for (;; str1 = NULL)
	{
	  token = strtok_r (str1, delim, &save_ptr1);
	  if (token != NULL)
	    {
	      if (!(regexec (&regex, token, 0, NULL, 0)) && strlen(token)>5)
		{
		  //printf ("%s matched: %s len=%d\n", url,token,	  (int) strlen (token));
		  char *addr = strndup (token + 5, (int) strlen (token) - 5);

		  //critical section: insert into shared bounded queue acting as producer
		  pthread_mutex_lock (&lock1);
		  while (count == QUEUE_SIZE)
		    pthread_cond_wait (&empty, &lock1);

                  //check if url is not already visited
		  pthread_mutex_unlock (&lock1); 

		  _edge_fn(url,addr);

		  pthread_mutex_lock (&lock1);
                  if(lookup_string(urls, addr)==NULL){
			has_atleast_a_link = 1;
			add_string(urls, addr);
			put(addr);
			pthread_mutex_lock(&glock);
                        sum_queue_lengths++;
			pthread_mutex_unlock(&glock);
			//_edge_fn(url,addr);
		  	pthread_cond_signal (&fill);
                  }
		  pthread_mutex_unlock (&lock1);

		}

	    }
	  else
	    break;
	}
        pthread_mutex_lock(&glock);
	sum_queue_lengths--; 
        pthread_mutex_unlock(&glock);
      /*
      if (has_atleast_a_link == 0)
	{
	  pthread_mutex_lock (&glock);
	  sum_queue_lengths--;
	  pthread_mutex_unlock (&glock);
	}
      */
    }
  thr_exit ();
  //printf("parser thread exiting\n");
  return NULL;
}

void *
download (void *context)	//char *(*_fetch_fn) (char *url))
{
  char *(*_fetch_fn) (char *url) = context;
  char *url;
  //critical section: dequeue url from shared bounded queue acting as consumer
  while (!areQueuesEmpty ())
    {
      pthread_mutex_lock (&lock1);
      while (count == 0)
	pthread_cond_wait (&fill, &lock1);
      //pthread_mutex_lock(&glock);
      url = get ();
      //pthread_mutex_unlock(&glock);
      pthread_cond_signal (&empty);
      pthread_mutex_unlock (&lock1);
      //printf("downloading %s que len=%d\n",url,sum_queue_lengths);
      char *page = _fetch_fn (url);
      if (page == NULL)
	{			//fetch was unsuccessful
          //printf("download %s was unsuccessful\n",url);
      	  pthread_mutex_lock(&glock);
	  sum_queue_lengths--;
	  pthread_mutex_unlock(&glock);	   
	}
      else
	{
	  //printf("downloading %s completed\n",url);
	  //add to unbounded queue
	  pthread_mutex_lock (&lock2);
	  List_Insert (queue_pages, url, page); //List_Insert is thread-safe as it has a lock
	  //printf("inserted %s in queue of pages\n",url);
	  pthread_cond_signal (&fill_unbounded);	//signal when not empty
	  pthread_mutex_unlock (&lock2);
	}
    }
    //printf("downloader exiting\n");
  thr_exit ();
  return NULL;
}

int
crawl (char *start_url,
       int download_workers,
       int parse_workers,
       int queue_size,
       char *(*_fetch_fn) (char *url),
       void (*_edge_fn) (char *from, char *to))
{

  QUEUE_SIZE = queue_size;
  queue_links = (char **) malloc (sizeof (char *) * queue_size);
  queue_pages = (list_t *) malloc (sizeof (list_t));
  List_Init (queue_pages);
  urls = create_hash_table(65535); //hash set for unique urls

  //add start_url to bounded queue
  //printf("start url = %s\n", start_url);
  put (start_url);
  add_string(urls, start_url);

/*
  char *seed_url = NULL;
  lst_t *l = lookup_string(urls, start_url);  
  if(l!=NULL)
	  seed_url = l->string;
  if(seed_url != NULL)
	printf("seed url=%s\n",seed_url);
  else
	printf("not found\n");
*/
  //set sum_queue_lengths to 1
  sum_queue_lengths = 1;
 

  //create threads
  pthread_t downloaders[download_workers], parsers[parse_workers];
  int i;
  //printf("creating threads\n");

  for (i = 0; i < download_workers; i++)
    pthread_create (&downloaders[i], NULL, download, _fetch_fn);

  for (i = 0; i < parse_workers; i++)
    pthread_create (&parsers[i], NULL, parse, _edge_fn);

  //printf("created threads\n");
  thr_join ();
  
  printf ("crawler completed successfully\n");
  return 0;
}
