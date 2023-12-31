#include "storage_mgr.h"
#include "buffer_mgr_stat.h"
#include "buffer_mgr.h"
#include "dberror.h"
#include "test_helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// var to store the current test's name
char *testName;

// check whether two the content of a buffer pool is the same as an expected content
// (given in the format produced by sprintPoolContent)
#define ASSERT_EQUALS_POOL(expected,bm,message)                    \
do {                                    \
char *real;                                \
char *_exp = (char *) (expected);                                   \
real = sprintPoolContent(bm);                    \
if (strcmp((_exp),real) != 0)                    \
{                                    \
printf("[%s-%s-L%i-%s] FAILED: expected <%s> but was <%s>: %s\n",TEST_INFO, _exp, real, message); \
free(real);                            \
exit(1);                            \
}                                    \
printf("[%s-%s-L%i-%s] OK: expected <%s> and was <%s>: %s\n",TEST_INFO, _exp, real, message); \
free(real);                                \
} while(0)

// test and helper methods
static void createDummyPages(BM_BufferPool *bm, int num);

static void testCLOCK (void);

// main method
int
main (void)
{
    initStorageManager();
    testName = "";
    
    testCLOCK();
    return 0;
}


void
createDummyPages(BM_BufferPool *bm, int num)
{
    int i;
    BM_PageHandle *h = MAKE_PAGE_HANDLE();
    
    CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));
    
    for (i = 0; i < num; i++)
    {
        CHECK(pinPage(bm, h, i));
        sprintf(h->data, "%s-%i", "Page", h->pageNum);
        CHECK(markDirty(bm, h));
        CHECK(unpinPage(bm,h));
    }
    
    CHECK(shutdownBufferPool(bm));
    
    free(h);
}

// test the CLOCK page replacement strategy
void
testCLOCK (void)
{
    // expected results
    const char *poolContents[] = {
        // read first five pages and directly unpin them
        "[0 0],[-1 0],[-1 0],[-1 0],[-1 0]" ,
        "[0 0],[1 0],[-1 0],[-1 0],[-1 0]",
        "[0 0],[1 0],[2 0],[-1 0],[-1 0]",
        "[0 0],[1 0],[2 0],[3 0],[-1 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        // use some of the page to create a fixed CLOCK order without changing pool content
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        // check that pages get evicted in CLOCK order
        "[0 0],[5 0],[2 0],[3 0],[4 0]",
        "[0 0],[5 0],[6 0],[3 0],[4 0]",
        "[0 0],[5 0],[6 0],[7 0],[4 0]",
        "[0 0],[5 0],[6 0],[7 0],[8 0]",
        "[9 0],[5 0],[6 0],[7 0],[8 0]",
    };
    const int orderRequests[] = {3,4,0,2,1};
    const int numCLOCKOrderChange = 5;
    
    int i;
    int snapshot = 0;
    BM_BufferPool *bm = MAKE_POOL();
    BM_PageHandle *h = MAKE_PAGE_HANDLE();
    testName = "Testing CLOCK page replacement";
    
    CHECK(createPageFile("testbuffer.bin"));
    createDummyPages(bm, 100);
    CHECK(initBufferPool(bm, "testbuffer.bin", 5, RS_CLOCK, NULL));
    
    // reading first five pages linearly with direct unpin and no modifications
    for(i = 0; i < 5; i++)
    {
        pinPage(bm, h, i);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading in pages");
    }
    
    // read pages to change CLOCK order
    for(i = 0; i < numCLOCKOrderChange; i++)
    {
        pinPage(bm, h, orderRequests[i]);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");
    }
    
    // replace pages and check that it happens in CLOCK order
    for(i = 0; i < 5; i++)
    {
        pinPage(bm, h, 5 + i);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");
    }
    
    // check number of write IOs
    ASSERT_EQUALS_INT(0, getNumWriteIO(bm), "check number of write I/Os");
    ASSERT_EQUALS_INT(10, getNumReadIO(bm), "check number of read I/Os");
    
    CHECK(shutdownBufferPool(bm));
    CHECK(destroyPageFile("testbuffer.bin"));
    
    free(bm);
    free(h);
    TEST_DONE();
}
