

default: all

pre:
	rm -rf output
	mkdir output
	gcc -g -Wall -c dberror.c -o output/dberror.o
	gcc -g -Wall -c storage_mgr.c -o output/storage_mgr.o
	
test1:
	gcc -g -Wall -c test_assign1_1.c -o output/test_assign1_1.o
	gcc -g -Wall output/dberror.o output/storage_mgr.o output/test_assign1_1.o -o output/test1.o

all:
	make pre
	make test1