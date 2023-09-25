#include "buffer_mgr.h"
#include "dberror.h"
#include "stdio.h"
#include "dt.h"
#include "storage_mgr.h"
#include <stdlib.h>

typedef struct Page
{
	SM_PageHandle pageData;
	PageNumber pageNum;
	int dirtyFlag;
	int fixCount;
	int totalCount;		   // Total pin number, used by LRU
	int clockFlag;		   // used by CLOCK, 0 means we need replace it, 1 means pass by
	int lastUsedTimeStamp; // used by LRU, when page is hitted, it updates the timestamp;
} PageFrame;

typedef struct BM_BufferPool
{
	char *pageFile;
	int numPages; // pagenumber in storage manager
	ReplacementStrategy strategy;
	void *mgmtData; // use this one to store the bookkeeping info your buffer
	// manager needs for a buffer pool
	int readIOCount;  // Variable to track read I/O operations
	int writeIOCount; // New variable to track write I/O operations

} BM_BufferPool;

typedef struct BM_PageHandle
{
	PageNumber pageNum;
	char *data;
} BM_PageHandle;

PageFrame *bufferPool = NULL;
int bufferSize = 0;
int totalRead = 0;
int totalWrite = 0;
// The pointer is used for FIFO and CLOCK algroithm
// it usually changes in a loop, when it greats bufferSize, it will equal to pointer % bufferSize
int pointer = 0;
// used for LRU when a page is hitted, the timeStamp will increased and set to that buffer page.
int timeStamp = 0;

void writeWhenDirty(BM_BufferPool *const bm, PageFrame curPage)
{

	if (curPage.dirtyFlag == 1)
	{
		SM_FileHandle *sph;
		openPageFile(bm->pageFile, &sph);
		writeBlock(curPage.pageNum, &sph, curPage.pageData);
		totalWrite++;
		closePageFile(sph);
	}
}

void FIFO(BM_BufferPool *const bm, PageFrame page)
{
	PageFrame *buffers = bm->mgmtData;
	while (true)
	{
		if (pointer >= bufferSize)
		{
			pointer = pointer % bufferSize;
		}
		if (buffers[pointer].fixCount != 0)
		{
			pointer++;
			continue;
		}
		writeWhenDirty(bm, buffers[pointer]);
		buffers[pointer] = page;
		pointer++;
		break;
	}
}

void LFU(BM_BufferPool *const bm, PageFrame page)
{
	PageFrame *buffers = bm->mgmtData;
	int minTotalCount = INT16_MAX;
	int replaceIndex = 0;
	int loopNum = 0;
	while (true)
	{
		if (loopNum >= bufferSize)
		{
			break;
		}
		if (buffers[loopNum].fixCount = 0)
		{
			if (minTotalCount > buffers[loopNum].totalCount)
			{
				minTotalCount = buffers[loopNum].totalCount;
				replaceIndex = loopNum;
			}
		}
		loopNum++;
	}
	writeWhenDirty(bm, buffers[replaceIndex]);
	buffers[replaceIndex] = page;
	buffers[replaceIndex].totalCount = 1;
}
void LRU(BM_BufferPool *const bm, PageFrame page)
{
	PageFrame *buffers = bm->mgmtData;
	int minLastUsedTimeStamp = INT16_MAX;
	int replaceIndex = 0;
	int loopNum = 0;
	while (true)
	{
		if (loopNum >= bufferSize)
		{
			break;
		}
		if (buffers[loopNum].fixCount = 0)
		{
			if (minLastUsedTimeStamp > buffers[loopNum].lastUsedTimeStamp)
			{
				minLastUsedTimeStamp = buffers[loopNum].lastUsedTimeStamp;
				replaceIndex = loopNum;
			}
		}
		loopNum++;
	}
	writeWhenDirty(bm, buffers[replaceIndex]);
	buffers[replaceIndex] = page;
	buffers[replaceIndex].lastUsedTimeStamp = ++timeStamp;
}
void CLOCK(BM_BufferPool *const bm, PageFrame page)
{
	PageFrame *buffers = bm->mgmtData;
	while (true)
	{
		if (pointer >= bufferSize)
		{
			pointer = pointer % bufferSize;
		}
		if (buffers[pointer].fixCount != 0)
		{
			pointer++;
			continue;
		}
		if (buffers[pointer].clockFlag != 0)
		{
			buffers[pointer].clockFlag--;
			continue;
		}
		writeWhenDirty(bm, buffers[pointer]);
		buffers[pointer] = page;
		buffers[pointer].clockFlag = 1;
		pointer++;
		break;
	}
}

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
				  const int numPages, ReplacementStrategy strategy,
				  void *stratData)
{

	// Allocate memory for the buffer pool
	bufferPool = (PageFrame *)malloc(sizeof(PageFrame) * numPages);

	// Initialize the buffer pool
	for (int i = 0; i < numPages; i++)
	{
		bufferPool[i].pageNum = NO_PAGE;
		bufferPool[i].pageData = (char *)malloc(PAGE_SIZE); // Allocate memory for page data
		bufferPool[i].dirtyFlag = 0;
		bufferPool[i].fixCount = 0;
	}

	// Set buffer pool information
	bm->pageFile = (char *)pageFileName;
	bm->numPages = numPages;
	bm->strategy = strategy;
	bm->mgmtData = bufferPool;
	bufferSize = numPages;
	totalRead = 0;
	totalWrite = 0;
	return RC_OK;
}

RC shutdownBufferPool(BM_BufferPool *const bm)
{

	// Check if there are any pinned pages in the pool
	for (int i = 0; i < bufferSize; i++)
	{
		if (bufferPool[i].fixCount > 0)
		{
			return RC_PAGE_PINNED_IN_BUFFER_POOL;
		}
	}

	// Write dirty pages back to disk if fixCount=0
	forceFlushPool(bm);

	// Free memory used by the buffer pool
	free(bufferPool);
	bufferPool = NULL;
	bufferSize = 0;

	return RC_OK;
}

RC forceFlushPool(BM_BufferPool *const bm)
{

	SM_FileHandle fileHandle;
	RC rc;
	rc = openPageFile(bm->pageFile, &fileHandle);
	PageFrame *buffers = bm->mgmtData;

	// Open the page file
	if (rc != RC_OK)
	{
		return rc;
	}
	// Iterate through the page frames and write dirty pages back to disk
	for (int i = 0; i < bm->numPages; i++)
	{
		writeWhenDirty(bm, buffers[i]);
		// if (buffers[i].dirty == 1) {
		//     // Calculate the byte offset where the data should be written
		//     long offset = PageFrame[i].pageNum * PAGE_SIZE * sizeof(char);

		//     // Write the page data to the page file using writeBlock
		// 	rc = writeBlock(PageFrame[i].pageNum, &fileHandle, PageFrame[i].data);
		//     if (rc  != RC_OK) {
		//         return rc;
		//     }

		//     // Update the page frame's dirty flag
		//     buffers[i].dirty = 0;

		// }
	}

	// Close the page file
	// closePageFile(fileHandle);

	return RC_OK;
}

RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	PageFrame *buffers = bm->mgmtData;
	// Check if the requested page is already in the buffer pool
	for (int i = 0; i < bm->numPages; i++)
	{
		if (buffers[i].pageNum == page->pageNum)
		{
			// Mark the page as dirty
			buffers[i].dirty = 1;
			return RC_OK;
		}
	}
}

RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page)
{

	for (int i = 0; i < bm->numPages; i++)
	{
		if (PageFrame[i].pageNum == page->pageNum)
		{
			// Decrease the fix count
			if (PageFrame[i].fixCount > 0)
			{
				pageFrame[i].fixCount--;
				return RC_OK;
			}
		}
	}
}

RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page)
{

	PageFrame *buffers = bm->mgmtData;
	// Iterate through the page frames and find the page to force to disk
	for (int i = 0; i < bm->numPages; i++)
	{
		if (buffers[i].pageNum == page->pageNum)
		{
			// Check if the page is dirty
			writeWhenDirty(bm, buffers[i]);
			return RC_OK; // Page found and processed
		}
	}

	return RC_FILE_NOT_FOUND; // Page not found in the buffer pool
}

void readFromSMBlock(BM_BufferPool *const bm, PageFrame curPage, BM_PageHandle *const page, PageNumber pageNum)
{
	SM_FileHandle fileHandler;
	openPageFile(bm->pageFile, &fileHandler);
	curPage.pageData = (SM_PageHandle)malloc(PAGE_SIZE);
	ensureCapacity(pageNum, &fileHandler);
	readBlock(pageNum, &fileHandler, curPage.pageData);
	totalRead++;
}

RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page,
		   const PageNumber pageNum)
{

	PageFrame *frames = (PageFrame *)bm->mgmtData;
	SM_FileHandle fileHandle;
	RC rc;

	// Check if buffer pool is initialized
	// if (bm->pageFile == NULL) {
	//     return RC_BUFFER_POOL_NOT_INITIALIZED;
	// }

	// Search for the page in the buffer pool
	for (int i = 0; i < bufferSize; i++)
	{
		if (frames[i].pageNum == pageNum)
		{
			// Page found in buffer, update fixCount and return the data
			frames[i].fixCount++;
			page->pageNum = pageNum;
			page->data = frames[i].pageData;
			return RC_OK;
		}
	}
	PageFrame *newPage;

	readFromSMBlock(bm, newPage, page, pageNum);
	newPage->clockFlag = 1;
	newPage->fixCount = 1;
	newPage->lastUsedTimeStamp = ++timeStamp;
	newPage->totalCount = 1;
	newPage->pageNum = pageNum;
	// Page not found in the buffer pool, need to load it from disk
	openPageFile(bm->pageFile, &fileHandle);

	// Find an empty frame or apply the replacement strategy
	int emptyFrameIndex = -1;
	for (int i = 0; i < bufferSize; i++)
	{
		if (frames[i].pageNum == NO_PAGE)
		{
			emptyFrameIndex = i;
			break;
		}
	}

	if (emptyFrameIndex == -1)
	{
		// No empty frames, apply the replacement strategy based on bm->strategy
		switch (bm->strategy)
		{
		case RS_FIFO:
			FIFO(bm, newPage, page, pageNum);

			break;
		case RS_LFU:
			LFU(bm, newPage, page, pageNum);

			break;
		case RS_LRU:
			LRU(bm, newPage, page, pageNum);
			break;
		case RS_CLOCK:
			CLOCK(bm, newPage, page, pageNum);
			break;
		default:
			FIFO(bm, newPage, page, pageNum);
		}
	}
	else
	{
		// Found an empty frame, load the page into it

		frames[emptyFrameIndex] = newPage;
	}

	closePageFile(&fileHandle);
	return RC_OK;
}

// Retrieve the number of read I/O operations for a given buffer manager.
extern int getNumReadIO(BM_BufferPool *const bufferManager)
{
	// Check if bufferManager is NULL
	if (bufferManager == NULL)
	{
		return -1; // Return an error code or handle this case as needed
	}

	// Return the read I/O count from the bufferManager
	return totalRead;
}

// Retrieve the number of write I/O operations for a given buffer manager.
extern int getNumWriteIO(BM_BufferPool *const bufferManager)
{
	// Check if bufferManager is NULL
	if (bufferManager == NULL)
	{
		return -1; // Return an error code or handle this case as needed
	}

	// Return the write I/O count from the bufferManager
	return totalWrite;
}

extern PageNumber *getFrameContents(BM_BufferPool *const bm)
{
	// Read frames
	PageFrame *frames = (PageFrame *)bm->mgmtData;
	PageNumber *res = malloc(bufferSize * sizeof(PageNumber));

	// Use for loop, read all pageNum and assign to res.
	for (int curIndex = 0; curIndex < bufferSize; curIndex++)
	{
		PageNumber tmpNum = (frames[curIndex].pageNum != -1) ? frames[curIndex].pageNum : NO_PAGE;
		res[curIndex] = tmpNum;
	}
	return res;
}

extern int *getFixCounts(BM_BufferPool *const bm)
{
	// Read frames first from mgmtData.
	PageFrame *frames = (PageFrame *)bm->mgmtData;
	// Init a array for store the results.
	int *res = malloc(bufferSize * sizeof(int));

	// Use 'for' loop, read all fixCount and assign it to results.
	for (int curIndex = 0; curIndex < bufferSize; curIndex++)
	{
		res[curIndex] = (frames[curIndex].fixCount != -1) ? frames[curIndex].fixCount : 0;
	}
	// return the results.
	return res;
}

extern bool *getDirtyFlags(BM_BufferPool *const bm)
{
	PageFrame *buffers = (PageFrame *)bm->mgmtData;
	// resIsDirty is an array indicates all dirty flags
	bool *resIsDirty = malloc(bufferSize * sizeof(bool));

	// Use for loop to read all dirty and assign it to res.
	for (int curIndex = 0; curIndex < bufferSize; curIndex++)
	{
		resIsDirty[curIndex] = !!(frames[curIndex].dirtyFlag == 1);
	}
	// Return the dirty results.
	return resIsDirty;
}
