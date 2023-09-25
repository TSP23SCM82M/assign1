#include "buffer_mgr.h"
#include "dberror.h"
#include "stdio.h"
#include "dt.h"
#include "storage_mgr.h"
#include<stdlib.h>
typedef struct Page
{
	PageNumber pageNum;
	int dirtyFlag;
	int fixCount;
} PageFrame;

typedef struct BM_BufferPool {
	char *pageFile;
	int numPages;
	ReplacementStrategy strategy;
	void *mgmtData; // use this one to store the bookkeeping info your buffer
	// manager needs for a buffer pool
} BM_BufferPool;

typedef struct BM_PageHandle {
	PageNumber pageNum;
	char *data;
} BM_PageHandle;

PageFrame *bufferPool = NULL;
int bufferSize = 0;

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		const int numPages, ReplacementStrategy strategy,
		void *stratData){



		// Allocate memory for the buffer pool
		bufferPool = (PageFrame *)malloc(sizeof(PageFrame) * numPages);

		// Initialize the buffer pool
		for (int i = 0; i < numPages; i++) {
			bufferPool[i].pageNum = NO_PAGE;
			bufferPool[i].data = (char *)malloc(PAGE_SIZE); // Allocate memory for page data
			bufferPool[i].dirtyFlag = 0;
			bufferPool[i].fixCount = 0;
		}

		// Set buffer pool information
		bm->pageFile = (char *)pageFileName;
		bm->numPages = numPages;
		bm->strategy = strategy;
		bm->mgmtData = NULL; 
		bufferSize = numPages;
		return RC_OK;
	}

		

RC shutdownBufferPool(BM_BufferPool *const bm){


    // Check if there are any pinned pages in the pool
    for (int i = 0; i < bufferSize; i++) {
        if (bufferPool[i].fixCount > 0) {
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


RC forceFlushPool(BM_BufferPool *const bm){

// Check if the buffer pool is initialized
    // if (bm->pageFile == NULL) {
    //     return RC_BUFFER_POOL_NOT_INITIALIZED;
    // }

    // Iterate through the page frames and write dirty pages back to disk
    for (int i = 0; i < bm->numPages; i++) {
        if (pageFrames[i].dirty == 1) {
            // Write the dirty page back to the page file
            FILE *file = fopen(bm->pageFile, "rb+");
            if (file == NULL) {
                return RC_FILE_NOT_FOUND;
            }

            // Seek to the correct position in the page file
            if (fseek(file, pageFrames[i].pageNum * PAGE_SIZE, SEEK_SET) != 0) {
                fclose(file);
                //return RC_FILE_SEEK_ERROR;
            }

            // Write the page data to the page file
            if (fwrite(pageFrames[i].data, sizeof(char), PAGE_SIZE, file) != PAGE_SIZE) {
                fclose(file);
                return RC_WRITE_FAILED;
            }

            // Update the page frame's dirty flag
            pageFrames[i].dirty = 0;

            // Close the page file
            fclose(file);
        }
    }

    return RC_OK;

}


RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){
// Check if the requested page is already in the buffer pool
    for (int i = 0; i < bm->numPages; i++) {
        if (PageFrame[i].pageNum == page->pageNum) {
            // Mark the page as dirty
            PageFrame[i].dirty = 1;
            return RC_OK;
        }
    }


}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){

	 for (int i = 0; i < bm->numPages; i++) {
        if (PageFrame[i].pageNum == page->pageNum) {
            // Decrease the fix count
            if (PageFrame[i].fixCount > 0) {
                pageFrames[i].fixCount--;
                return RC_OK;
            } 
        }
    }

}

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){

}

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
		const PageNumber pageNum){

		}



extern PageNumber *getFrameContents (BM_BufferPool *const bm)
{
	// Read frames
	PageFrame *frames = (PageFrame *) bm->mgmtData;
	PageNumber *res = malloc(bufferSize * sizeof(PageNumber));
	
	// Use for loop, read all pageNum and assign to res.
	for (int curIndex = 0; curIndex < bufferSize; curIndex++) {
		PageNumber tmpNum = (frames[curIndex].pageNum != -1) ? frames[curIndex].pageNum : NO_PAGE;
		res[curIndex] = tmpNum;
	}
	return res;
}

extern bool *getDirtyFlags (BM_BufferPool *const bm)
{
	PageFrame *frames = (PageFrame *)bm->mgmtData;
	bool *resIsDirty = malloc(bufferSize * sizeof(bool));
	
	// Use for loop to read all dirty and assign it to res.
	for(int curIndex = 0; curIndex < bufferSize; curIndex++) {
		resIsDirty[curIndex] = !!(frames[curIndex].dirtyFlag == 1);
	}
	// Return the dirty results.
	return resIsDirty;
}

extern int *getFixCounts (BM_BufferPool *const bm) {
	// Read frames first from mgmtData.
	PageFrame *frames = (PageFrame *)bm->mgmtData;
	// Init a array for store the results.
	int *res = malloc(bufferSize * sizeof(int));
	
	// Use 'for' loop, read all fixCount and assign it to results.
	for(int curIndex = 0; curIndex < bufferSize; curIndex++) {
		res[curIndex] = (frames[curIndex].fixCount != -1) ? frames[curIndex].fixCount : 0;
	}
	// return the results.
	return res;
}
