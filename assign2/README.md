# assign1

## Notable

- We have completed FIFO, LRU, CLOCK, LRU_K, LFU algorithm and relevant test cases have been written.
- We've passed all test cases included test_assign1_1, test_assign2_1, test_assign2_2, test_assign2_3_LFU, test_assign2_4_CLOCK.

## How to run this project

First, you can change the variable "RUNNER" and "FLAG" for some build requirements. For example, use another compiler or build for better debugger. When we want to build for debugger, we need add flag for debugger.

The build method are based Makefile. There are the commands that you can use.

1) make

This is run the default command, which is "all".

2) make all

This command is used to build all test files, and the output will generated in the "output" file.

3) make clean

You may don't need to run this directly.
This command is used to remove the "output" directory and all of it's subfiles. It is really useful when you want to make sure your workplace is clean. For example, you want to push your code to remote repository.

4 make run

After build, you can use this to run all the tests.

5) ./output/test2.o

After build, you can use this command to run this project.

./output/test1.o for assign1 test cases.
./output/test2.o for assign2 test cases.
./output/test2_2.o for assign2_2 test cases.
./output/test3.o for assign2 LFU test cases.
./output/test4.o for assign2 CLOCK test cases.



**Replacement Strategies**

**For bonus credit :** 
*Clock*

CLOCK determines whether the desired page is already present in the buffer pool.

If it's there, it lets us know about it and raises the pin count, which shows that it's in use.

If not, it searches for a free space in the pool to use.
If it locates an empty space, it fills it with the desired page from the disk.

CLOCK uses a reference bit to keep track of whether or not each page in the pool has recently been accessed.

It clears the reference bit and searches for the next page if it needs to replace a page and discovers one that has a reference bit set, indicating that it has recently been used.

Until it finds a page that doesn't have the reference bit set, indicating that it hasn't been used recently, it continues doing this and replaces that page with the new one.

*First-In-First-Out (FIFO)*

Like a queue, FIFO operates. It determines whether the desired page is already present in the buffer pool, which is temporary memory for pages.

If the page is already in the buffer pool, it is returned to us, and the number of "pins" (which represents how frequently this page is accessed) increases.

It searches for an open space in the buffer pool if the page is not already there.

If it comes to an empty area, it fills it up by reading the desired page from the disk and inserting it there.

Before inserting the new page, it writes the altered page back to the disk if the place it's utilizing was previously holding a separate page that was changed (dirty).

*Least Recently Used (LRU)*

LRU is a technique that swaps out the buffer pool page that hasn't been used in the longest.

Imagine that the buffer pool's pages are organized from most recently used at one end to least recently used at the other, in order of when they were last used.

Prior to inserting the new page, LRU selects the page that had the least recently utilized end and writes it to the disk.


*initBufferPool*

A new memory area, which we refer to as a buffer pool, is created up by initBufferPool with a predetermined number of page slots.

At first, no new pages are created and all the slots are null. It's similar to clearing up parking spots in a lot for automobiles but there aren't any cars there yet.

This process uses dynamic memory allocation to build the buffer pool.

*shutdownBufferPool*

shutdownBufferPool is responsible for closing down and cleaning up a buffer pool.

It effectively destroys the parking lot we previously created by releasing any resources and memory that the buffer pool was utilizing.
This function resets the buffer manager variables after releasing resources, making it ready for potential future use.

*forceFlushPool*

When a page in the buffer pool has been modified (i.e., has a "dirty" mark), forceFlushPool makes sure that it is written back to the disk.
It's similar to saving any document edits you make before turning off your computer.

To ensure that everything is securely kept on the disk, this function entails opening an existing file on the disk, writing the altered pages back to that file, and then closing the file.

*markDirty*

If a page is marked as "dirty," it has been edited, and markDirty checks to see if that is the case.

It searches the buffer pool for the frame containing this page.

When it locates the frame, it flags the page as "dirty," signifying that alterations have been performed.

If the page cannot be located, an error is reported.

It returns RC_OK, which denotes success if everything goes according to plan.

*pinPage*

pinPage is a method used to make sure a specific page (identified by pageNum) is available for use in the buffer pool.

It applies logic based on LRU (Least Recently Used) and FIFO (First-In-First-Out) strategies to decide which page should be removed from the buffer pool if it's full or needs to make space.

*unpinPage*

The "pin" (a marker indicating the page is being used) can be removed from a page using the unpinPage function.
It makes sure the inputs are proper by first checking them.

The frame in the buffer pool that matches the given page is then sought after.

If the "fixed count" (a count of the number of times the page has been pinned) is more than 0, the fixed count is decreased.

It returns RC_OK after removing the pin successfully.

*forcePage*

To ensure that changes are saved, the forcePage method writes the contents of a page back to the disk (the page file).

The current page should be written back to the disk if the page->pageNum matches the pageNum.

The page is marked as not "dirty" after authoring it, indicating that it has not been edited since.

Indicating that the page was successfully written back to the file on disk, it returns RC_OK.

*getFrameContents*

Accessing and reviewing the contents of a particular page frame is possible with the help of the getFrameContents function.

It can be used to set a pointer that enables you to traverse the frame and access the value or information kept within.

*getDirtyFlags*

The getDirtyFlags function produces an array of boolean values when you call it.

A page frame in the buffer pool is represented by each boolean value in the array.

The page saved in the "i"-th frame is designated as "dirty," indicating that it has been updated, if the boolean at index "i" is TRUE.

*getFixCounts*

An array of integers is returned by the getFixCounts function.

A page frame in the buffer pool is represented by each integer in the array.

The fixed count of the page contained in the "i"-th frame is represented by the integer at index "i."

How frequently the page has been pinned or is currently being used is shown by the fixed count.

*getNumReadIO*

You can find out how many pages have been read from the disk since a buffer pool was originally created by using the getNumReadIO method.

*getNumWriteIO*

It gives how many pages have been written to the page file (saved to the disk) since the buffer pool was established using the getNumWriteIO function.


**Team Members**
Anwesha Nayak(A20512145): 20%
Taufeeq Ahmed Mohammed(A20512082): 20%
Shruti Shankar Shete(A20518508): 20%
Jianqi Jin(A20523325): 20%
Syed Zoraiz Sibtain (A20521018): 20%
