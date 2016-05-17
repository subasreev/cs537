.PHONY: all
all : libcrawler.so file_tester web_tester concurrent_threads

file_tester : file_tester.c libcrawler.so
	gcc -g -L. -lcrawler -lpthread file_tester.c list.c -Wall -Werror -o file_tester

web_tester : web_tester.c cs537.c libcrawler.so
	gcc -g -L. -lcrawler -lpthread web_tester.c cs537.c list.c -Wall -Werror -o web_tester

libcrawler.so : crawler.c
	gcc -g -fpic -c list.c -Wall -Werror -o list.o
	gcc -g -fpic -c hashtable.c -Wall -Werror -o hashtable.o
	gcc -g -fpic -c crawler.c -Wall -Werror -o crawler.o 
	gcc -g -shared -o libcrawler.so crawler.o list.o hashtable.o

.PHONY: clean
clean :
	rm -f file_tester web_tester libcrawler.so *.o *~
