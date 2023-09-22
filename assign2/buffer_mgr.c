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

int bufferSize = 0;

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
