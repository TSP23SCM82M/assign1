#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<string.h>

#include "storage_mgr.h"

FILE *textFile;

extern void initStorageManager (void) {

}

extern RC createPageFile (char *fileName) {
	
	char * memory;
	//Open file in write mode 
	textFile=fopen(fileName,"w");
	
	//Check if the file opened successfully
    if(textFile == NULL)
	{
        return RC_FILE_NOT_FOUND;
	}

	//Allocate memory using malloc() with size of PAGE_SIZE
     memory=(char*)malloc(PAGE_SIZE);

	//Initialize memory with null bytes
    memset(memory, '\0', PAGE_SIZE);

	//Store the number of bytes written in no_of_bytes
	size_t no_of_bytes=fwrite(memory, 1 , PAGE_SIZE, textFile);	

	//Check if the number of bytes written matches the expected page size. If it does not match, return RC_WRITE_FAILED
	if(no_of_bytes!=PAGE_SIZE)
	{
		return RC_WRITE_FAILED;	
	}

	//Free the allocated memory
	free(memory);

	//Close the file		
	fclose(textFile);	

	return RC_OK;
	
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle) {

	//Open fileName in read and write(r+) mode
	textFile=fopen(fileName,"r+");

	//Check if file opens successfully
	 if(textFile == NULL)
	{
        return RC_FILE_NOT_FOUND;
	}

	//Initialize the file handle fields

	fHandle->fileName=fileName;

	fHandle->mgmtInfo=textFile;

	fseek(textFile,0,SEEK_END);

	long size=ftell(textFile);

	int totalPages=(int)(size/PAGE_SIZE);

	fHandle->totalNumPages=totalPages;

	fHandle->curPagePos=0;

	return RC_OK;

}

extern RC closePageFile (SM_FileHandle *fHandle) {
	
	//Close the textFile using fclose()
	fclose(textFile);
    
    // Set fHandle to NULL
    fHandle = NULL;
    
    return RC_OK;
}


extern RC destroyPageFile (char *fileName) {
	
	//Check whether textFile exists
	if (textFile==NULL)
	{
        return RC_FILE_NOT_FOUND;
    }

	//delete the textFile using remove() function
    remove(fileName);

    return RC_OK;
}

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {

	if (pageNum < 0 || pageNum > fHandle->totalNumPages) {

        return RC_READ_NON_EXISTING_PAGE;
    }
	else{
        
		float bytesRead;
		
		fseek(textFile,pageNum*PAGE_SIZE, SEEK_SET);
		
		bytesRead=fread(memPage,sizeof(char),PAGE_SIZE,textFile);

        if (bytesRead != PAGE_SIZE) {
            if (feof(textFile)) {
            // The requested page is not found because the end of the file has been reached.
                return RC_READ_NON_EXISTING_PAGE;
            }      
            else if (ferror(textFile)) {
            // During the read operation an error occurred. So handle the error or return an appropriate error code.
                return RC_READ_NON_EXISTING_PAGE;
            }
        }

		fHandle->curPagePos=pageNum;
		return RC_OK;
        
		}
	}


	
extern int getBlockPos (SM_FileHandle *fHandle) {

	{
	// Check if the file handle is Null and returns File Not Found Error
	if (fHandle == NULL) 
	{
		return RC_FILE_NOT_FOUND;
	}

	// Otherwise the function reads the current page position and stores it in the curPagePos variable
	else
	{
		return fHandle->curPagePos;
	}
}

}

extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {


	if (fHandle != NULL) {
        RC result = readBlock(0, fHandle, memPage); // Reads the first block
        if (result == RC_OK) {
            return RC_OK; 
        } else {
            return result; // Return the error code if failed
        }
	}

	// If the page is not found, returns an error
	else
	{
		return RC_READ_NON_EXISTING_PAGE;
	}

}

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {

	// If the file handle is Null, returns an error 
	if (fHandle == NULL)
	{
		return RC_READ_NON_EXISTING_PAGE;
	}

	/*If the file handle is not null, this method reads the file handle's position and stores it in 'curPagePos.' 
    'cur_page_num' holds the current block position value, and the 'readblock' function returns the previous block position*/
	else
	{
		int cur_page_num;
		cur_page_num = fHandle->curPagePos;
		return readBlock(cur_page_num - 1, fHandle, memPage); // cur_page_num - 1 i.e. previous block position
		return RC_OK;
	}
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {

	// If the file handle is NULL, return an error 
	if (fHandle == NULL)
	{
		return RC_READ_NON_EXISTING_PAGE;
	}
	
	/*If the file handle is not null, this method reads the file handle's position and stores it in 'curPagePos.' 
    'cur_page_num' holds the current block position value, and the 'readblock' function returns the current block position*/
	int cur_page_num;
	cur_page_num = fHandle->curPagePos;
	return readBlock(cur_page_num, fHandle, memPage);
	return RC_OK;
	
}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

	// If the file handle is NULL, return an error 
	if (fHandle == NULL)
	{
		return RC_READ_NON_EXISTING_PAGE;
	}
	
	/*If the file handle is not null, this method reads the file handle's position and stores it in 'curPagePos.' 
    'cur_page_num' holds the current block position value, and the 'readblock' function returns the next block position*/
	int cur_page_num;
	cur_page_num = fHandle->curPagePos;
	return readBlock(cur_page_num + 1, fHandle, memPage); // cur_page_num + 1 i.e. next block position
	return RC_OK;
	
}

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

	// If the file handle is NULL, return an error 
	if (fHandle == NULL)
	{
		return RC_READ_NON_EXISTING_PAGE;
	}

	/*If the file handle is not null, lastPageNum holds the last block position value and the 'readblock' function returns the last block position*/
	int lastPageNum = fHandle->totalNumPages - 1; //lastPageNum holds the last block position
	return readBlock(lastPageNum, fHandle, memPage);
}

extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {

	// If pageNum is negative or pageNum exceeds the total number of pages, return an error
   if (pageNum < 0 || fHandle->totalNumPages < pageNum) {
		return RC_READ_NON_EXISTING_PAGE;
	}
	
	//calculate the byte offset where the data should be written and use fseek to position the file pointer to that location
	long offset = pageNum * PAGE_SIZE * sizeof(char);
	
	int seekTag = fseek(fHandle->mgmtInfo, offset, SEEK_SET);
	
	int writtenSize = fwrite(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo); // return size
	
	fHandle->curPagePos;
	
	fHandle = pageNum;
	
	return RC_OK;
}

extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	
	// Because we write one, so the total need be added by one
	fHandle->totalNumPages = fHandle->totalNumPages + 1;

	int curPagePos = fHandle->curPagePos;
	// Calculate the current page num, using page size and page position
	int curPageNum = curPagePos / PAGE_SIZE;
	
	// Call the write function to write block
	return writeBlock(curPageNum, fHandle, memPage);
}


extern RC appendEmptyBlock (SM_FileHandle *fHandle) {
	// Use fseek to reset the position.
	int isSeekSuccess = fseek(textFile, 0, SEEK_END);
	// Init an empty block using calloc.
	SM_PageHandle newBlock = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
	
	if(isSeekSuccess != 0) {
		// This means that the fseek failed.
		free(newBlock);
		return RC_WRITE_FAILED;
	}
	// We need add total number first, cause we append a new block.
	fHandle->totalNumPages = fHandle->totalNumPages + 1;
	// Write the newBlock to the file.
	fwrite(newBlock, sizeof(char), PAGE_SIZE, textFile);
	// Use free to release the memory usage.
	free(newBlock);
	
	return RC_OK;
}

extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {
	// Open the file first and use "a" mode. Because we need append something to the file.
	// textFile = fopen(fHandle->fileName, "a");
	
	if (textFile == NULL) {
		// This means open failed.
		return RC_FILE_NOT_FOUND;
	}
	
	int totalNum = fHandle->totalNumPages;
	int neededNum = numberOfPages - totalNum;
	for (int i = 0; i < neededNum; i++) {
		// Append empty to the block to ensure enough capacity.
		appendEmptyBlock(fHandle);
	}
	// Close the file.
	// fclose(textFile);
	return RC_OK;
}

//Additional Rename Function

extern RC renameFile (char *fileName,char *newFileName, SM_FileHandle *fHandle){

	//Open fileName in read and write mode
	textFile=fopen(fileName,"r+");

	//Check if textFile was opened successfully
	 if(textFile == NULL)
	{
        return RC_FILE_NOT_FOUND;
	}

	//Rename the fileName using rename() function
	rename(fileName, newFileName);

	//Change the fHandle->fileName to the newFileName
	fHandle->fileName=newFileName;

	return RC_OK;

}