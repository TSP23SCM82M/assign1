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

**extern void initStorageManager ( void );**

**extern RC createPageFile ( char * fileName );**

This function is used to create a new file with the name fileName. The file is opened using fopen() C function in write mode.  It allocates memory for a blank page using malloc, and initializes the memory with '\0' bytes using memset(). It verifies if the number of bytes written matches the expected page size and returns an error code if they do not match. Finally,it closes the file, deallocates memory and returns RC_OK.

**extern RC openPageFile ( char * fileName , SM_FileHandle * fHandle );**

This function opens the page file specified by fileName using the fopen() C function in read and write mode. If the file doesn't exist, RC FILE NOT FOUND is returned. fseek() is used the file pointer to the end of the file and calculate the total number of pages. Fields of the file handle are initialized with the information about the opened file.


**extern RC closePageFile ( SM_FileHandle * fHandle );**\

This function closes the page file using the fclose() function and sets the file handle to NULL. 

**extern RC destroyPageFile ( char * fileName );**

This function is used to delete a file specified by filename. It checks if fileName exists in memory and, if it does, deletes it using the remove function. 

**extern RC readBlock (int pageNum, SM_FileHandle \*fHandle, SM_PageHandle memPage);**

This function is used for reading the content from page. "pageNum" tell us which page we want to read. Initially we check if the page we want to seek is valid, i.e. if it exists or not. Then we go to the required location using fseek and valid pointer. Then we read data from the page to buffer "mempage".

**extern int getBlockPos (SM_FileHandle \*fHandle);**

This function is used to get the current block position that we get back from FileHandle's curPagePos.

**extern RC readFirstBlock (SM_FileHandle \*fHandle, SM_PageHandle memPage);**

Using the file handle, get the first block from the file, then place it inside the memPage page. The readBlock function is called with a parameter value of 0 to retrieve the first page in the file. If the first block is missing from the file handle, an error will be raised.

**extern RC readPreviousBlock (SM_FileHandle \*fHandle, SM_PageHandle memPage);**

Using the file handle, get the previous block from the file and save it in the memPage page. Check to see if the file handle contains the previous block. Return an error indicating a non-existent page if it is missing. But if it does, take it out of the fHandle data structure and put the current file location in the cur_page_num variable. To read the file's previous page, call the readBlock function with the parameter cur_page_num - 1.

**extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);**

Using the file handle, get the current block from the file and save it in the memPage page. Check to see if the file handle contains the current block. Return an error indicating a non-existent page if it is missing. But if it does, take it out of the fHandle data structure and put the current file location in the cur_page_num variable. To read the file's current page, call the readBlock function with the parameter cur_page_num.

**extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);**

Using the file handle, get the next block from the file and save it in the memPage page. Check to see if the file handle contains the next block. Return an error indicating a non-existent page if it is missing. But if it does, take it out of the fHandle data structure and put the current file location in the cur_page_num variable. To read the file's next page, call the readBlock function with the parameter cur_page_num + 1.

**extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);**

Using the file handle, get the last block from the file and save it in the memPage page. Check to see if the file handle contains the last block. Return an error indicating a non-existent page if it is missing. But if it does, store the fHandle->totalNumPages - 1 in lastPageNum. To read the file's last page, call the readBlock function with the parameter lastPageNum.

**extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);**

This function writes a page of data (specified by memPage) to a specific page (pageNum) within a file. If pageNum is negative or pageNum exceeds the total number of pages, error is returned. The byte offset where the data should be written is calculated and fseek is used to position the file pointer to that location. If fseek() issuccessful, the content is stored in memPage.

**extern RC writeCurrentBlock (SM_FileHandle \*fHandle, SM_PageHandle memPage);**

This is function receives 2 parameters, the first one is the filehandle, and the second one is memPage.
This function will write memPage to the page file that the file heandler points to. And after that add the currentPageNumber by 1.

**extern RC appendEmptyBlock (SM_FileHandle \*fHandle);**

This is function receives 1 parameter, it's the file handler. We can use createPageFile function to get a file handler.
This is function just append an empty block to pagefile, and add the total page number by 1.

**extern RC ensureCapacity (int numberOfPages, SM_FileHandle \*fHandle);**

This function receives two parameters, the first one is the required page number, and the second one is the file handler which can get from createPageFile.
This function ensures the capacity of pageFile, when the current page number of pageFile is less than the required page number, then the function will add some empty blocks to make sure the capacity.

## Additional Functions added

**extern RC renameFile (char *fileName,char *newFileName, SM_FileHandle *fHandle);**

This function is used to rename the file name specified by fileName to newFileName using the rename() C function. The file handle's fileName is also initialized with the newFileName.

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


