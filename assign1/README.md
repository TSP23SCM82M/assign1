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