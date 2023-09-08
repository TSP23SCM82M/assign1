#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<string.h>

#include "storage_mgr.h"

FILE *pageFile;

extern void initStorageManager (void) {
}

extern RC createPageFile (char *fileName) {
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
}

extern RC closePageFile (SM_FileHandle *fHandle) {
}


extern RC destroyPageFile (char *fileName) {
}

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
}

extern int getBlockPos (SM_FileHandle *fHandle) {
}

extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
}

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {

}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
}

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
}

extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
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
	int isSeekSuccess = fseek(pageFile, 0, SEEK_END);
	// Init an empty block using calloc.
	SM_PageHandle newBlock = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
	
	if(isSeekSuccess != 0) {
		// This means that the fseek failed.
		free(newBlock);
		return RC_WRITE_FAILED;
	}
	// We need add total number first, cause we append a new block.
	fHandle->totalNumPages++;
	// Write the newBlock to the file.
	fwrite(newBlock, sizeof(char), PAGE_SIZE, pageFile);
	// Use free to release the memory usage.
	free(newBlock);
	
	return RC_OK;
}

extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {
	// Open the file first and use "a" mode. Because we need append something to the file.
	pageFile = fopen(fHandle->fileName, "a");
	
	if (pageFile == NULL) {
		// This means open failed.
		return RC_FILE_NOT_FOUND;
	}
	
	int totalNum = fHandle->totalNumPages;
	int neededNum = numberOfPages - totalNum;
	if (int i = 0; i < neededNum; i++) {
		// Append empty to the block to ensure enough capacity.
		appendEmptyBlock(fHandle);
	}
	// Close the file.
	fclose(pageFile);
	return RC_OK;
}

