

default: all

pre:
	rm -rf output
	mkdir output
	gcc -g -Wall -c dberror.c -o output/dberror.o
	gcc -g -Wall -c storage_mgr.c -o output/storage_mgr.o
	gcc -g -Wall -c buffer_mgr_stat.c -o output/buffer_mgr_stat.o
	gcc -g -Wall -c buffer_mgr.c -o output/buffer_mgr.o
	
test1:
	gcc -g -Wall -c test_assign2_1.c -o output/test_assign2_1.o
	gcc -g -Wall output/dberror.o output/storage_mgr.o output/buffer_mgr_stat.o output/buffer_mgr.o output/test_assign2_1.o -o output/test2.o

all:
	make pre
	make test1