# GCC https://www.geeksforgeeks.org/gcc-command-in-linux-with-examples/
# Makefile https://www.math.colostate.edu/~yzhou/computer/writemakefile.html#:~:text=The%20makefile%20is%20a%20text,separate%20rule%20in%20a%20makefile.
CC = gcc
CFLAGS  = -g -Wall 

default: test1

test1:
	$(RM) -rf output
	mkdir output
	$(CC) $(CFLAGS) -c dberror.c -o output/dberror.o
	$(CC) $(CFLAGS) -c storage_mgr.c -lm -o output/storage_mgr.o
	$(CC) $(CFLAGS) -c test_assign1_1.c -lm -o output/test_assign1_1.o
	$(CC) $(CFLAGS) ./output/test_assign1_1.o ./output/storage_mgr.o ./output/dberror.o -lm -o output/test1.o

clean: 
	$(RM) -rf output
