

default: all

pre:
	gcc -g -Wall -c dberror.c -o output/dberror.o
	gcc -g -Wall -c storage_mgr.c -o output/storage_mgr.o
	gcc -g -Wall -c buffer_mgr_stat.c -o output/buffer_mgr_stat.o
	gcc -g -Wall -c buffer_mgr.c -o output/buffer_mgr.o
	gcc -g -Wall -c record_mgr.c -o output/record_mgr.o
	gcc -g -Wall -c expr.c -o output/expr.o
	gcc -g -Wall -c rm_serializer.c -o output/rm_serializer.o
	
test1:
	gcc -Wall -c test_assign1_1.c -o output/test_assign1_1.o
	gcc -Wall output/dberror.o output/storage_mgr.o output/buffer_mgr_stat.o output/buffer_mgr.o output/test_assign1_1.o -o output/test1.o
test2:
	gcc -Wall -c test_assign2_1.c -o output/test_assign2_1.o
	gcc -Wall output/dberror.o output/storage_mgr.o output/buffer_mgr_stat.o output/buffer_mgr.o output/test_assign2_1.o -o output/test2.o
test2_2: # LRU_K
	gcc -g -Wall -c test_assign2_2.c -o output/test_assign2_2.o
	gcc -g -Wall output/dberror.o output/storage_mgr.o output/buffer_mgr_stat.o output/buffer_mgr.o output/test_assign2_2.o -o output/test2_2.o
test2_3: # LFU
	gcc -g -Wall -c test_assign2_3_LFU.c -o output/test_assign2_3.o
	gcc -g -Wall output/dberror.o output/storage_mgr.o output/buffer_mgr_stat.o output/buffer_mgr.o output/test_assign2_3.o -o output/test2_3.o
test2_4: # CLOCK
	gcc -g -Wall -c test_assign2_4_CLOCK.c -o output/test_assign2_4.o
	gcc -g -Wall output/dberror.o output/storage_mgr.o output/buffer_mgr_stat.o output/buffer_mgr.o output/test_assign2_4.o -o output/test2_4.o
test3:
	gcc -g -Wall -c test_assign3_1.c -o output/test_assign3_1.o
	gcc -g -Wall output/dberror.o output/expr.o output/record_mgr.o output/rm_serializer.o output/storage_mgr.o output/buffer_mgr_stat.o output/buffer_mgr.o output/test_assign3_1.o -o output/test3.o

clean:
	rm -rf output
	mkdir output

all:
	make clean
	make pre
	make test1
	make test2
	make test2_2
	make test2_3
	make test2_4
	make test3

run:
	./output/test1.o
	./output/test2.o
	./output/test2_2.o
	./output/test3.o
	./output/test4.o