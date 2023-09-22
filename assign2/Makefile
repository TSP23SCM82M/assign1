# GCC https://www.geeksforgeeks.org/gcc-command-in-linux-with-examples/
# Makefile https://www.math.colostate.edu/~yzhou/computer/writemakefile.html#:~:text=The%20makefile%20is%20a%20text,separate%20rule%20in%20a%20makefile.
RUNNER = gcc
FLAG  = -g -Wall 

default: test1

test1:
	make clean
	mkdir output
# build error without link
	$(RUNNER) $(FLAG) -c dberror.c -o output/dberror.o
# build storage_mgr without link
	$(RUNNER) $(FLAG) -c storage_mgr.c -lm -o output/storage_mgr.o
# build test without link
	$(RUNNER) $(FLAG) -c test_assign1_1.c -lm -o output/test_assign1_1.o
# link
	$(RUNNER) $(FLAG) ./output/test_assign1_1.o ./output/storage_mgr.o ./output/dberror.o -lm -o output/test1.o

clean: 
	rm -rf output
