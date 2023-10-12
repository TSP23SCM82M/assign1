#include "dberror.h"
#include "record_mgr.h"
#include "stdio.h"
#include "dt.h"
#include <stdlib.h>
#include <stdint.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"

typedef struct RecordMgr
{
	// This variable stores the location of first free page which has empty slots in table
	int freePage;
	// Record ID	
	RID recordID;
	// This variable defines the condition for scanning the records in the table
	Expr *condition;
	// This variable stores the count of the number of records scanned
	int scanCount;
	// This variable stores the total number of tuples in the table
	int tuplesCount;
	// Buffer Manager's PageHandle for using Buffer Manager to access Page files
	BM_PageHandle pageHandle;	// Buffer Manager PageHandle 
	// Buffer Manager's Buffer Pool for using Buffer Manager	
	BM_BufferPool bufferPool;
} RecordMgr;

RecordMgr* recordMgr;


loadSchema(Schema *schema, char *dataPointer) {
    // tuplesCount int
    // freePage int
    // schema->numAttr int
    // schema->attrNames char * schema->numAttr
    // schema->dataTypes DataType * schema->numAttr
    // schema->typeLength int * schema->numAttr
    // This function will load schema from data
    schema->numAttr = *(int *)dataPointer;
    dataPointer += sizeof(int);

    // Retrieving total number of tuples from the page file
    recordMgr->tuplesCount= *(int*)dataPointer;
    dataPointer = dataPointer + sizeof(int);

    // Getting free page from the page file
    recordMgr->freePage= *(int*) dataPointer;
    dataPointer = dataPointer + sizeof(int);

    // Getting the number of attributes from the page file
    int attributeCount = *(int*)dataPointer;
    dataPointer = dataPointer + sizeof(int);

    Schema *schema;

    // Allocating memory space to 'schema'
    schema = (Schema*) malloc(sizeof(Schema));

    // Setting schema's parameters
    schema->numAttr = attributeCount;
    schema->attrNames = (char**) malloc(sizeof(char*) *attributeCount);
    schema->dataTypes = (DataType*) malloc(sizeof(DataType) *attributeCount);
    schema->typeLength = (int*) malloc(sizeof(int) *attributeCount);

    // Allocate memory space for storing attribute name for each attribute
    for(int k = 0; k < attributeCount; k++)
        schema->attrNames[k]= (char*) malloc(ATTR_SIZE);
        
    for(int k = 0; k < schema->numAttr; k++)
        {
        // Setting attribute name
        strncpy(schema->attrNames[k], dataPointer, ATTR_SIZE);
        dataPointer = dataPointer + ATTR_SIZE;
        
        // Setting data type of attribute
        schema->dataTypes[k]= *(int*) dataPointer;
        dataPointer = dataPointer + sizeof(int);

        // Setting length of datatype (length of STRING) of the attribute
        schema->typeLength[k]= *(int*)dataPointer;
        dataPointer = dataPointer + sizeof(int);
    }
}

writeSchema(Schema *schema, char *dataPointer) {
    // This function will write schema to data
    // Setting number of tuples to 0
    *(int*)dataPointer = 0; 

    // Incrementing pointer by sizeof(int) because 0 is an integer
    dataPointer = dataPointer + sizeof(int);

    // Setting first page to 1 since 0th page if for schema and other meta data
    *(int*)dataPointer = 1;

    // Incrementing pointer by sizeof(int) because 1 is an integer
    dataPointer = dataPointer + sizeof(int);

    // Setting the number of attributes
    *(int*)dataPointer = schema->numAttr;

    // Incrementing pointer by sizeof(int) because number of attributes is an integer
    dataPointer = dataPointer + sizeof(int); 

    // Setting the Key Size of the attributes
    *(int*)dataPointer = schema->keySize;

    // Incrementing pointer by sizeof(int) because Key Size of attributes is an integer
    dataPointer = dataPointer + sizeof(int);

    for(int k = 0; k < schema->numAttr; k++)
    {
        // Setting attribute name
        strncpy(dataPointer, schema->attrNames[k], ATTR_SIZE);
        dataPointer = dataPointer + ATTR_SIZE;

        // Setting data type of attribute
        *(int*)dataPointer = (int)schema->dataTypes[k];

        // Incrementing pointer by sizeof(int) because we have data type using integer constants
        dataPointer = dataPointer + sizeof(int);

        // Setting length of datatype of the attribute
        *(int*)dataPointer = (int) schema->typeLength[k];

        // Incrementing pointer by sizeof(int) because type length is an integer
        dataPointer = dataPointer + sizeof(int);
    }

}

extern RC initRecordManager (void *mgmtData) {
    initStorageManager();
    return RC_OK;
}
extern RC shutdownRecordManager () {
    free(recordMgr);
    recordMgr = NULL;
    return RC_OK;
}
extern RC createTable (char *name, Schema *schema) {
    recordMgr = (RecordMgr *) malloc(sizeof(RecordMgr));
    createPageFile(name);
    char pageData[PAGE_SIZE];
    char *dataPointer = pageData;
    writeSchema(schema, dataPointer);
    RC result;
    SM_FileHandle fileHandle;
    // Creating a page file page name as table name using storage manager
    if((result = createPageFile(name)) != RC_OK)
        return result;
        
    // Opening the newly created page
    if((result = openPageFile(name, &fileHandle)) != RC_OK)
        return result;
        
    // Writing the schema to first location of the page file
    if((result = writeBlock(0, &fileHandle, pageData)) != RC_OK)
        return result;
        
    // Closing the file after writing
    if((result = closePageFile(&fileHandle)) != RC_OK)
        return result;
    initBufferPool(&recordMgr->bufferPool, name, MAX_PAGES, RS_LRU, NULL);
}
extern RC openTable (RM_TableData *rel, char *name) {
    pinPage(&recordMgr->bufferPool, &recordMgr->pageHandle, 0);
    Schema *schema;
    loadSchema(schema, &recordMgr->pageHandle);
    // Unpinning the page i.e. removing it from Buffer Pool using BUffer Manager
    unpinPage(&recordMgr->bufferPool, &recordMgr->pageHandle);

    // Write the page back to disk using BUffer Manger
    forcePage(&recordMgr->bufferPool, &recordMgr->pageHandle);
}
