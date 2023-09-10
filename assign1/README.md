# assign1

## How to run this project

First, you can change the variable "RUNNER" and "FLAG" for some build requirements. For example, use another compiler or build for better debugger. When we want to build for debugger, we need add flag for debugger.

The build method are based Makefile. There are the commands that you can use.

1) make clean

This command is used to remove the "output" directory and all of it's subfiles. It is really useful when you want to make sure your workplace is clean. For example, you want to push your code to remote repository.

2) make test1

This command is used to build the test1 file, and the output will generated in the "output" file.


3) make

This is run the default command, which is "test1".

4) ./output/test1.o

After build, you can use this command to run this project.


## Function explain


**extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);**

This function is used for reading the content from page. "pageNum" tell us which page we want to read. Initially we check if the page we want to seek is valid, i.e. if it exists or not. Then we go to the required location using fseek and valid pointer. Then we read data from the page to buffer "mempage".

**extern int getBlockPos (SM_FileHandle *fHandle);**

This function is used to get the current block position that we get back from FileHandle's curPagePos.

**extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);**

Using the file handle, get the first block from the file, then place it inside the memPage page. The readBlock function is called with a parameter value of 0 to retrieve the first page in the file. If the first block is missing from the file handle, an error will be raised.

**extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);**

Using the file handle, get the previous block from the file and save it in the memPage page. Check to see if the file handle contains the previous block. Return an error indicating a non-existent page if it is missing. But if it does, take it out of the fHandle data structure and put the current file location in the cur_page_num variable. To read the file's previous page, call the readBlock function with the parameter cur_page_num - 1.

**extern RC writeCurrentBlock (SM_FileHandle \*fHandle, SM_PageHandle memPage);**

This is function receives 2 parameters, the first one is the filehandle, and the second one is memPage.
This function will write memPage to the page file that the file heandler points to. And after that add the currentPageNumber by 1.

**extern RC appendEmptyBlock (SM_FileHandle \*fHandle);**

This is function receives 1 parameter, it's the file handler. We can use createPageFile function to get a file handler.
This is function just append an empty block to pagefile, and add the total page number by 1.

**extern RC ensureCapacity (int numberOfPages, SM_FileHandle \*fHandle);**

This function receives two parameters, the first one is the required page number, and the second one is the file handler which can get from createPageFile.
This function ensures the capacity of pageFile, when the current page number of pageFile is less than the required page number, then the function will add some empty blocks to make sure the capacity.

## Test explain

To check multiple page content:
**static void testMultiplePageContent(void);**

Here we wrote test cases for:

*Read First Block (readFirstBlock):*
Reads the first page into the file handle.Verifies that the page is empty by checking for zero bytes. Prints a message indicating that the first block was empty.

*Write First Block (writeBlock):*
Writes the content of the page. Prints a message confirming the writing of the first block.

*Read First Block Again (readFirstBlock):*
Reads the first block from the file again. Checks that the content matches what was previously written.

*Write Second Block (writeCurrentBlock):*
Writes the content of the page. Prints a message confirming the writing of the second block.

*Read Second Block (readCurrentBlock):*
Reads the second block i.e. the current block from the file. Verifies that the content matches the second string.

*Reads the previous block from the file.*
Verifies that the previous block content is read. 

*Ensure Capacity (ensureCapacity):*
Attempts to ensure that the file has a capacity of at least 4 pages.

*Destroy Page File (destroyPageFile):*
Destroys the new page file (cleanup). Test Completion (TEST_DONE):

**testAddedFunc(void)**
This test is to check rename function. It uses string compare and returns true if filename matches to newfile name.


