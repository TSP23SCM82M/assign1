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
	textFile=fopen(fileName,"w");
	
    
    if(textFile == NULL)
	{
        return RC_FILE_NOT_FOUND;
	}

	else{
        memory=(char*)malloc(PAGE_SIZE);
        memset(memory, '\0', PAGE_SIZE); 
		size_t no_of_bytes=fwrite(memory, 1 , PAGE_SIZE, textFile);	
		if(no_of_bytes!=PAGE_SIZE)
		{
			return RC_WRITE_FAILED;	
		}
		free(memory);		
		fclose(textFile);			
		return RC_OK;
	}
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
	textFile=fopen(fileName,"r+");
	 if(textFile == NULL)
	{
        return RC_FILE_NOT_FOUND;
	}
	fHandle->fileName=fileName;
	fHandle->mgmtInfo=textFile;
	fseek(textFile,0,SEEK_END);
	long size=ftell(textFile);
	int totalPages=(int)(size/PAGE_SIZE);
	fHandle->totalNumPages=totalPages;
	fHandle->curPagePos=0;
	//fclose(textFile);
	return RC_OK;

}

extern RC closePageFile (SM_FileHandle *fHandle) {
	
	fclose(textFile);
    fHandle=NULL;
    return RC_OK;
}


extern RC destroyPageFile (char *fileName) {
	if (textFile==NULL){
        return RC_FILE_NOT_FOUND;
    }
    remove(fileName);
    return RC_OK;
}

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {

	if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {

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

// If the file handle is not null, using readblock, first block is read
    //Also returns an OK message  
    /*if (fHandle != NULL) {
        
		return readBlock(0, fHandle, memPage); // Reads the first block
        return RC_OK; 
        
	}*/

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
    'current_pg_num' holds the current block position value, and the 'readblock' function returns the previous block position*/
	else
	{
		int current_pg_num;
		current_pg_num = fHandle->curPagePos;
		return readBlock(current_pg_num - 1, fHandle, memPage); // current_pg_num - 1 i.e. previous block position
		return RC_OK;
	}
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {

	if (fHandle == NULL)
	{
		return RC_READ_NON_EXISTING_PAGE;
	}
	

	int current_pg_num;
		current_pg_num = fHandle->curPagePos;
		return readBlock(current_pg_num, fHandle, memPage);
		return RC_OK;
	
}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

	if (fHandle == NULL)
	{
		return RC_READ_NON_EXISTING_PAGE;
	}
	
		int current_pg_num;
		current_pg_num = fHandle->curPagePos;

		return readBlock(current_pg_num + 1, fHandle, memPage);
		return RC_OK;
	
}

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	
	int lastPageNum = fHandle->totalNumPages - 1; 
	return readBlock(lastPageNum, fHandle, memPage);
}

extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {

	if (pageNum < 0 || fHandle->totalNumPages < (pageNum + 1)) {
		return RC_READ_NON_EXISTING_PAGE;
	}

	long offset = pageNum * PAGE_SIZE * sizeof(char);

	int seekTag = fseek(fHandle->mgmtInfo, offset, SEEK_SET);

	
	int writtenSize = fwrite(memPage, sizeof(char), PAGE_SIZE,
		fHandle->mgmtInfo); // return size


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
	textFile = fopen(fHandle->fileName, "a");
	
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
	fclose(textFile);
	return RC_OK;
}

extern RC renameFile (char *fileName,char *newFileName, SM_FileHandle *fHandle){
	textFile=fopen(fileName,"r+");
	 if(textFile == NULL)
	{
        return RC_FILE_NOT_FOUND;
	}
	fHandle->fileName=newFileName;
	return RC_OK;

}