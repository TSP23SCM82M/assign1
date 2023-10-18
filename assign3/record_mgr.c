#include "dberror.h"
#include "record_mgr.h"
#include "stdio.h"
#include "dt.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
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


// Function to find a free slot in the data page : Required for InsertRecord
int FreeSlot(char *data, int recordSize)
{
    int i = 0;                                   
    int totalSlots = PAGE_SIZE / recordSize;   // Calculate total number of slots

    while (i < totalSlots)                       
    {
        if (data[i * recordSize] != '+') {      // Check if the slot is free
            return i;                             // Return the index of the free slot
        }
        i++;                                      
    }

    return -1;                                    // Return -1 if no free slot is found
}

typedef struct RM_ScanData {
    Expr *condition;  // The condition for scanning
    int currentRecordPage;  // The current page being scanned
    int currentRecordSlot;  // The current slot on the current page
    RecordMgr *recordManager;  // Reference to the Record Manager
} RM_ScanData;

int sizeInt = sizeof(int);
int sizeFloat = sizeof(float);
int sizeBool = sizeof(bool);

extern RC loadSchema(Schema *schema) {
    // tuplesCount int
    // freePage int
    // schema->numAttr int
    // schema->attrNames char * schema->numAttr
    // schema->dataTypes DataType * schema->numAttr
    // schema->typeLength int * schema->numAttr
    // This function will load schema from data
	SM_PageHandle dataPointer = (char*) recordMgr->pageHandle.data;
    // schema->numAttr = *(int *)dataPointer;

    recordMgr->tuplesCount= *(int*)dataPointer;
    dataPointer += sizeInt;

    recordMgr->freePage= *(int*) dataPointer;
    dataPointer += sizeInt;

    int attrNumber = *(int*)dataPointer;
    dataPointer += sizeInt;

    schema->typeLength = (int*) malloc(attrNumber * sizeInt);
    schema->dataTypes = (DataType*) malloc(attrNumber * sizeof(DataType));
    schema->attrNames = (char**) malloc(attrNumber * sizeof(char*));
    schema->numAttr = attrNumber;

    // We need to allocate memory for schema
    for(int k = 0; k < attrNumber; k++)
        schema->attrNames[k]= (char*) malloc(SIZE_ATTR);
        
    for(int k = 0; k < schema->numAttr; k++)
    {
        // set the value
        strncpy(schema->attrNames[k], dataPointer, SIZE_ATTR);
        if (k < schema->numAttr) {
            // move the dataPointer to the next one
            dataPointer += SIZE_ATTR;
            
            schema->dataTypes[k]= *(int*) dataPointer;
            dataPointer += sizeInt;

            schema->typeLength[k]= *(int*)dataPointer;
            dataPointer += sizeInt;
        } else {
        }
    }
    return RC_OK;
}

RC writeSchema(Schema *schema, char *dataPointer) {
    // This function will write schema to data
    *(int*)dataPointer = 0; 

    dataPointer += sizeInt;

    *(int*)dataPointer = 1;

    dataPointer += sizeInt;

    *(int*)dataPointer = schema->numAttr;

    dataPointer += sizeInt; 

    *(int*)dataPointer = schema->keySize;

    dataPointer += sizeInt;

    for(int k = 0; k < schema->numAttr; k++)
    {
        strncpy(dataPointer, schema->attrNames[k], SIZE_ATTR);
        if (k < schema->numAttr) {
            dataPointer = dataPointer + SIZE_ATTR;

            *(int*)dataPointer = (int)schema->dataTypes[k];

            dataPointer += sizeInt;

            *(int*)dataPointer = (int) schema->typeLength[k];

            dataPointer += sizeInt;
        }
    }
    return RC_OK;

}

extern RC initRecordManager (void *mgmtData) {
    initStorageManager();
    return RC_OK;
}
extern RC shutdownRecordManager () {
	shutdownBufferPool(&recordMgr->bufferPool);
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
    SM_FileHandle fileHandle;
    if(0 != createPageFile(name))
        return RC_ERROR;
    if(0 != openPageFile(name, &fileHandle))
        return RC_ERROR;
        
    if(0 != writeBlock(0, &fileHandle, pageData))
        return RC_ERROR;
        
    if(0 != closePageFile(&fileHandle))
        return RC_ERROR;
    initBufferPool(&recordMgr->bufferPool, name, MAX_PAGES, RS_LRU, NULL);
	return RC_OK;
}
extern RC openTable (RM_TableData *rel, char *name) {
    Schema *schema;
	schema = (Schema*) malloc(sizeof(Schema));
	// Pinning a page i.e. putting a page in Buffer Pool using Buffer Manager
	pinPage(&recordMgr->bufferPool, &recordMgr->pageHandle, 0);
	// Setting table's meta data to our custom record manager meta data structure
	rel->mgmtData = recordMgr;
	// Setting the table's name
	rel->name = name;
    loadSchema(schema);
	rel->schema = schema;
    unpinPage(&recordMgr->bufferPool, &recordMgr->pageHandle);

    forcePage(&recordMgr->bufferPool, &recordMgr->pageHandle);
	return RC_OK;
}

extern RC closeTable(RM_TableData* rel)
{
    // Store the table's metadata before closing it
    RecordMgr* recordMgr = rel->mgmtData;

    // Shutdown the Buffer Pool
    // shutdownBufferPool(&recordMgr->bufferPool);
    
    // Free the schema
    rel->mgmtData = NULL;
    
    return RC_OK;
}

extern RC deleteTable(char* name)
{
    RC result;
    while (1) {
        result = destroyPageFile(name);
        if (result != RC_OK) 
        {
            return RC_FILE_NOT_FOUND; // File not found, no need to delete
        } else {
                return result; // Some other error occurred during deletion
            }
        }
    return RC_OK; // File was successfully deleted
}

extern int getNumTuples(RM_TableData* rel)
{
	RecordMgr* recordMgr = rel->mgmtData;
	return recordMgr->tuplesCount;
	
}

extern RC insertRecord(RM_TableData *rel, Record *record)
{
    RecordMgr* recordMgr = rel->mgmtData;
    RC returnCode = RC_OK;
    RID *recordID = &record->id;
    char *data, *slot_p;
    int recordSize = getRecordSize(rel->schema);

    recordID->page = recordMgr->freePage;
    
    do {
        pinPage(&recordMgr->bufferPool, &recordMgr->pageHandle, recordID->page);
        data = recordMgr->pageHandle.data;
        recordID->slot = FreeSlot(data, recordSize);

        if (recordID->slot == -1) {
            unpinPage(&recordMgr->bufferPool, &recordMgr->pageHandle);
            recordID->page++;
        }
    } while (recordID->slot == -1);

    slot_p = data;
    markDirty(&recordMgr->bufferPool, &recordMgr->pageHandle);
    slot_p += recordID->slot * recordSize;
    *slot_p = '+';
    memcpy(++slot_p, record->data + 1, recordSize - 1);
    unpinPage(&recordMgr->bufferPool, &recordMgr->pageHandle);
    recordMgr->tuplesCount++;
    pinPage(&recordMgr->bufferPool, &recordMgr->pageHandle, 0);

    return returnCode;
}



extern RC deleteRecord (RM_TableData *rel, RID id) {
    if (rel == NULL || rel->mgmtData == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    // Check if the RID is valid
    if (id.page < 0 || id.slot < 0) {
        return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
    }

    // We have the table and the record ID. First, we need to go to the record.
    RecordMgr *rm = (RecordMgr*)rel->mgmtData;

    // Pin the page in the buffer pool
    pinPage(&rm->bufferPool, &rm->pageHandle, id.page);

    // Calculate the offset to the slot within the page
    int slot_offset = id.slot * getRecordSize(rel->schema);

    // Check if the slot contains a valid record
    if (slot_offset >= PAGE_SIZE || rm->pageHandle.data[slot_offset] == 0) {
        unpinPage(&rm->bufferPool, &rm->pageHandle);
        return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
    }

    // Mark the slot as empty (delete the record)
    memset(&rm->pageHandle.data + slot_offset, 0, getRecordSize(rel->schema));

    // Mark the page as dirty
    markDirty(&rm->bufferPool, &rm->pageHandle);

    // Unpin the page
    unpinPage(&rm->bufferPool, &rm->pageHandle);

    return RC_OK;
}


extern RC updateRecord (RM_TableData *rel, Record *record)
{

    if (rel == NULL || rel->mgmtData == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    // Check if the RID is valid
    if (record==NULL || record->id.page < 0 || record->id.slot < 0) {
        return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
    }

    // We have the table and the record ID. First, we need to go to the record.
    RecordMgr *rm = (RecordMgr*)rel->mgmtData;

    // Pin the page in the buffer pool
    pinPage(&rm->bufferPool, &rm->pageHandle, record->id.page);

    // Calculate the offset to the slot within the page
    int slot_offset = record->id.slot * getRecordSize(rel->schema);

    // Check if the slot contains a valid record
    if (slot_offset >= PAGE_SIZE || rm->pageHandle.data[slot_offset] == 0) {
        unpinPage(&rm->bufferPool, &rm->pageHandle);
        return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
    }

    // Copy the updated record data into the slot
    char *source = record->data;
    char *destination = rm->pageHandle.data;
	destination += slot_offset;
    memcpy(++destination, source + 1, getRecordSize(rel->schema) - 1);

    // Mark the page as dirty
    markDirty(&rm->bufferPool, &rm->pageHandle);

    // Unpin the page
    unpinPage(&rm->bufferPool, &rm->pageHandle);

    return RC_OK;

}


extern RC getRecord (RM_TableData *rel, RID id, Record *record)
{
    // Check if the provided pointers are valid
    if (rel == NULL || rel->mgmtData == NULL || record == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    // Check if the RID is valid
    if (id.page < 0 || id.slot < 0) {
        return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
    }

    // Retrieve the record manager data
    RecordMgr *rm = (RecordMgr *)rel->mgmtData;

    // Pin the page in the buffer pool
    pinPage(&rm->bufferPool, &rm->pageHandle, id.page);

    // Calculate the offset to the slot within the page
    int slot_offset = id.slot * getRecordSize(rel->schema);

    // Check if the slot contains a valid record
    if (slot_offset >= PAGE_SIZE || rm->pageHandle.data[slot_offset] == 0) {
        unpinPage(&rm->bufferPool, &rm->pageHandle);
        return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
    }

    // Copy the record data from the slot to the provided Record structure
    char *source = &rm->pageHandle.data[slot_offset];
    char *destination = record->data;
    memcpy(++destination, source + 1, getRecordSize(rel->schema) - 1);

    // Set the record's ID
    record->id.page = id.page;
    record->id.slot = id.slot;

    // Unpin the page
    unpinPage(&rm->bufferPool, &rm->pageHandle);

    return RC_OK;

}

// scans
extern RC startScan1 (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{

if (rel == NULL || scan == NULL || cond == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    // Initialize the scan handle
    scan->rel = rel;
    scan->mgmtData = malloc(sizeof(RM_ScanData));
    RM_ScanData *scanData = (RM_ScanData *)scan->mgmtData;
    scanData->condition = cond;
    scanData->currentRecordPage = -1;
    scanData->currentRecordSlot = -1;
    scanData->recordManager = (RecordMgr *)rel->mgmtData;

    return RC_OK;

}



extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{
	// Checking if scan condition (test expression) is present
	if (cond == NULL)
	{
		return RC_SCAN_CONDITION_NOT_FOUND;
	}

	// Open the table in memory
	openTable(rel, "ScanTable");

    	RecordMgr *scanManager;
	RecordMgr *tableManager;

	// Allocating some memory to the scanManager
    	scanManager = (RecordMgr*) malloc(sizeof(RecordMgr));
    	
	// Setting the scan's meta data to our meta data
    	scan->mgmtData = scanManager;
    	
	// 1 to start scan from the first page
    	scanManager->recordID.page = 1;
    	
	// 0 to start scan from the first slot	
	scanManager->recordID.slot = 0;
	
	// 0 because this just initializing the scan. No records have been scanned yet    	
	scanManager->scanCount = 0;

	// Setting the scan condition
    	scanManager->condition = cond;
    	
	// Setting the our meta data to the table's meta data
    	tableManager = rel->mgmtData;

	// Setting the tuple count
    	tableManager->tuplesCount = SIZE_ATTR;

	// Setting the scan's table i.e. the table which has to be scanned using the specified condition
    	scan->rel= rel;

	return RC_OK;
}
extern RC next1 (RM_ScanHandle *scan, Record *record)
{

  if (scan == NULL || record == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    RM_ScanData *scanData = (RM_ScanData *)scan->mgmtData;
    RecordMgr *rm = scanData->recordManager;
    RM_TableData *rel = scan->rel;

    // Loop through pages and slots to find the next matching record
    while (scanData->currentRecordPage < rm->freePage) {
        // Move to the next page or slot
        if (scanData->currentRecordSlot + 1 >= rel->schema->numAttr) {
            scanData->currentRecordSlot = 0;
            scanData->currentRecordPage++;
        } else {
            scanData->currentRecordSlot++;
        }

        // Get the record at the current page and slot
        RID id;
        id.page = scanData->currentRecordPage;
        id.slot = scanData->currentRecordSlot;
        RC status = getRecord(rel, id, record);

        // Check if the record matches the scan condition
        Value *result = NULL;
        evalExpr(record, rel->schema, scanData->condition, &result);
        if (result->v.boolV) {
            freeVal(result);
            return status;
        }
    }

    return RC_RM_NO_MORE_TUPLES;

}
extern RC next (RM_ScanHandle *scan, Record *record)
{
	// Initiliazing scan data
	RecordMgr *scanManager = scan->mgmtData;
	RecordMgr *tableManager = scan->rel->mgmtData;
    	Schema *schema = scan->rel->schema;
	
	// Checking if scan condition (test expression) is present
	if (scanManager->condition == NULL)
	{
		return RC_SCAN_CONDITION_NOT_FOUND;
	}

	Value *result = (Value *) malloc(sizeof(Value));
   
	char *data;
   	
	// Getting record size of the schema
	int recordSize = getRecordSize(schema);

	// Calculating Total number of slots
	int totalSlots = PAGE_SIZE / recordSize;

	// Getting Scan Count
	int scanCount = scanManager->scanCount;

	// Getting tuples count of the table
	int tuplesCount = tableManager->tuplesCount;

	// Checking if the table contains tuples. If the tables doesn't have tuple, then return respective message code
	if (tuplesCount == 0)
		return RC_RM_NO_MORE_TUPLES;

	// Iterate through the tuples
	while(scanCount <= tuplesCount)
	{  
		// If all the tuples have been scanned, execute this block
		if (scanCount <= 0)
		{
			// printf("INSIDE If scanCount <= 0 \n");
			// Set PAGE and SLOT to first position
			scanManager->recordID.page = 1;
			scanManager->recordID.slot = 0;
		}
		else
		{
			// printf("INSIDE Else scanCount <= 0 \n");
			scanManager->recordID.slot++;

			// If all the slots have been scanned execute this block
			if(scanManager->recordID.slot >= totalSlots)
			{
				scanManager->recordID.slot = 0;
				scanManager->recordID.page++;
			}
		}

		// Pinning the page i.e. putting the page in buffer pool
		pinPage(&tableManager->bufferPool, &scanManager->pageHandle, scanManager->recordID.page);
			
		// Retrieving the data of the page			
		data = scanManager->pageHandle.data;

		// Calulate the data location from record's slot and record size
		data = data + (scanManager->recordID.slot * recordSize);
		
		// Set the record's slot and page to scan manager's slot and page
		record->id.page = scanManager->recordID.page;
		record->id.slot = scanManager->recordID.slot;

		// Intialize the record data's first location
		char *dataPointer = record->data;

		// '-' is used for Tombstone mechanism.
		*dataPointer = '-';
		
		memcpy(++dataPointer, data + 1, recordSize - 1);

		// Increment scan count because we have scanned one record
		scanManager->scanCount++;
		scanCount++;

		// Test the record for the specified condition (test expression)
		evalExpr(record, schema, scanManager->condition, &result); 

		// v.boolV is TRUE if the record satisfies the condition
		if(result->v.boolV == TRUE)
		{
			// Unpin the page i.e. remove it from the buffer pool.
			unpinPage(&tableManager->bufferPool, &scanManager->pageHandle);
			// Return SUCCESS			
			return RC_OK;
		}
	}
	
	// Unpin the page i.e. remove it from the buffer pool.
	unpinPage(&tableManager->bufferPool, &scanManager->pageHandle);
	
	// Reset the Scan Manager's values
	scanManager->recordID.page = 1;
	scanManager->recordID.slot = 0;
	scanManager->scanCount = 0;
	
	// None of the tuple satisfy the condition and there are no more tuples to scan
	return RC_RM_NO_MORE_TUPLES;
}

extern RC closeScan (RM_ScanHandle *scan)
{

if (scan == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    if (scan->mgmtData != NULL) {
        free(scan->mgmtData);
        scan->mgmtData = NULL;
    }

    return RC_OK;

}

// dealing with schemas
extern int getRecordSize (Schema *schema)
{
    int recordSize = 0;

    if (schema == NULL || schema->numAttr <= 0) {
        return 0; 
    }

    // Calculate the size of the record based on attribute types and lengths
    for (int i = 0; i < schema->numAttr; i++) {
        int attrLength = 0;

        // Determine the length of the attribute based on its data type
        if (schema->dataTypes[i] == DT_INT) {
            attrLength = sizeof(int);
        } else if (schema->dataTypes[i] == DT_STRING) {
            attrLength = schema->typeLength[i];
        } else if (schema->dataTypes[i] == DT_FLOAT) {
            attrLength = sizeof(float);
        } else if (schema->dataTypes[i] == DT_BOOL) {
            attrLength = sizeof(bool);
        }

        // Add the attribute length to the total record size
        recordSize += attrLength;
    }

    return recordSize;

}

extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
    // Check for valid inputs
    if (numAttr <= 0 || attrNames == NULL || dataTypes == NULL || typeLength == NULL) 
    {
        return NULL; 
    }

    Schema *sc = (Schema *)malloc(sizeof(Schema));

    if (sc == NULL) 
    {
        return NULL;
    }
    sc->keyAttrs = keys;
    // Initialize the Schema struct members
    if (numAttr <= 0) {
        return RC_ERROR;
    }
    sc->keySize = keySize;
    sc->numAttr = numAttr;
    if (attrNames == NULL) {
        return RC_ERROR;
    } else {
    sc->attrNames = attrNames;
    }
    if (dataTypes == NULL) {
        return RC_ERROR;
    } else {
        sc->dataTypes = dataTypes;
    }
    if (typeLength == NULL) {
        return RC_ERROR;
    } else {
        sc->typeLength = typeLength;
    }

    return sc;

}


//last 5 functions
extern RC freeSchema (Schema *schema)
{
	if (schema == NULL) {
    // Handle the exception: schema is NULL
    RC_message = "Schema is NULL";
    return RC_ERROR;
	}

	free(schema);
	return RC_OK;

}

extern RC createRecord (Record **record, Schema *schema)
{
	if (schema == NULL) {
		RC_message = "Schema is NULL";
		return RC_ERROR;
	}

	int dataSize = getRecordSize(schema);
	Record *newRecord = (Record*) malloc(sizeof(Record));
	newRecord->data= (char*) malloc(dataSize);
	newRecord->id.page = -1;
	newRecord->id.slot = -1;
	if (newRecord == NULL) {
    // Handle the exception: newRecord is NULL
    RC_message = "newRecord is NULL";
    return RC_ERROR;
}

	char *dataPointer = newRecord->data;
	if (dataPointer != NULL) {
		*dataPointer = '-';
		++dataPointer;
		*dataPointer = '\0';
		*record = newRecord;
	} else {
		// Handle the exception: dataPointer is NULL
		RC_message = "dataPointer is NULL";
		return RC_ERROR;
	}

	return RC_OK;
}

RC attrOffset(Schema *schema, int attrNum, int *result) {
    if (schema == NULL || result == NULL) {
		RC_message = "schema or result is NULL";
        return RC_ERROR;
    }
    
    if (attrNum < 0 || attrNum >= schema->numAttr) {
		RC_message = "attrNum is out of bounds";
        return RC_ERROR;
    }
    
    //int attr = 0;
    *result = 0;
    for (int attr = 0; attr < attrNum; attr++) {
        switch (schema->dataTypes[attr]) {
			case DT_INT:
                *result += sizeInt;
                break;
            case DT_STRING:
                *result += schema->typeLength[attr];
                break;
			case DT_FLOAT:
                *result += sizeFloat;
                break;
            case DT_BOOL:
                *result += sizeBool;
                break;
        }
    }
    
    *result += 1;
    return RC_OK;
}

extern RC freeRecord (Record *record)
{
	if (record == NULL || record == NULL) {
        RC_message = "Pointer Null";
        return RC_ERROR;
    }
    
    free(record);
    record = NULL;
    return RC_OK;
}

extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value)
{
	int offset = 0;
	char *dataPointer = NULL;
	Value *attribute = NULL;

	if (schema == NULL || record == NULL) {
		RC_message = "Invalid parameter";
        return RC_ERROR;
	}

	if (schema == NULL || record == NULL) {
		if (attribute != NULL) {
			free(attribute); // Free any allocated memory
		}
		RC_message = "Invalid parameter";
        return RC_ERROR;
	}

	RC attrOffsetStatus = attrOffset(schema, attrNum, &offset);
	if (attrOffsetStatus != RC_OK) {
		if (attribute != NULL) {
			free(attribute); // Free any allocated memory
		}
		return attrOffsetStatus;
	}

	dataPointer = record->data + offset;
	attribute = (Value *)malloc(sizeof(Value));
	if (attribute == NULL) {
		RC_message = "memory allocation failure";
        return RC_ERROR;
	}

	if (schema == NULL) {
    RC_message = "Invalid parameter";
        return RC_ERROR;
	}

	if (attrNum < 0 || attrNum >= schema->numAttr) {
		RC_message = "invalid attribute";
        return RC_ERROR;
	}

	if (attrNum == 1) {
		if (schema->dataTypes == NULL) {
			RC_message = "invalid schema";
        return RC_ERROR;
		}
		schema->dataTypes[attrNum] = 1;
	} else {
		if (schema->dataTypes == NULL) {
			RC_message = "invalid schema";
        	return RC_ERROR;
		}
		schema->dataTypes[attrNum] = schema->dataTypes[attrNum];
	}

	if(dataPointer != NULL && ((*schema).dataTypes[attrNum])==DT_STRING)
	{
		(*attribute).dt = DT_STRING;
		if(!false)
		{
			if (dataPointer != NULL && schema != NULL && attrNum >= 0 && attrNum < schema->numAttr) {
			if (schema->dataTypes[attrNum] == DT_STRING) {
				attribute->dt = DT_STRING;

				if (schema->typeLength != NULL && schema->typeLength[attrNum] >= 0) {
					attribute->v.stringV = (char *)malloc(schema->typeLength[attrNum] + 1);

					if (attribute->v.stringV != NULL) {
						strncpy(attribute->v.stringV, dataPointer, schema->typeLength[attrNum]);
                        int attrLen = schema->typeLength[attrNum];
						attribute->v.stringV[attrLen] = '\0';
					} else {
						RC_message = "memory allocation failure";
        				return RC_ERROR;
					}
				} else {
					RC_message = "invalid schema";
        			return RC_ERROR;
				}
			}
		} else {
			RC_message = "invalid parameter";
        	return RC_ERROR;
			}
		}
	}

	else if(dataPointer != NULL && (schema->dataTypes[attrNum])==DT_INT)
	{
		int value = 0;
		attribute->dt = DT_INT;
		memcpy(&value, dataPointer, sizeof(int));
		if (true) {
		(*attribute).v.intV = value;
        } else {
            return RC_ERROR;
        }
	}
	
	else if(dataPointer != NULL && ((*schema).dataTypes[attrNum])==DT_FLOAT)
	{
		if (true) {
		(*attribute).dt = DT_FLOAT;
		float value;
		if (!false) {
			memcpy(&value, dataPointer, sizeof(float));
			attribute->v.floatV = value;
		} else {
			return RC_ERROR; 
		}
		} else {
			return RC_ERROR;
		}
	}

	else if(dataPointer != NULL && (schema->dataTypes[attrNum])==DT_BOOL)
	{
		attribute->dt = DT_BOOL;
		bool value;

		if (dataPointer != NULL && schema->dataTypes[attrNum] == DT_BOOL) {
			if (dataPointer + sizeof(bool) > record->data + PAGE_SIZE) {
				RC_message = "Attribute data exceeds record boundaries";
				return RC_ERROR;
			}
			
			memcpy(&value, dataPointer, sizeof(bool));
			(*attribute).v.boolV = value;
		} else {
			RC_message = "Invalid attribute type or NULL data pointer"; // Handle invalid type or NULL pointer
			return RC_ERROR;
		}
		//value = attribute;
		return RC_OK;
	}

	*value = attribute;
	return RC_OK;
	free(attribute);
}

extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value)
{
	//int length;
	int offset = 0;

	if (schema == NULL || record == NULL || value == NULL) {
		RC_message = "Invalid parameter";
        return RC_ERROR;
    }
	
	if (schema == NULL || record == NULL) {
		RC_message = "Invalid parameter";
        return RC_ERROR;
	}
	
	RC attrOffsetStatus = attrOffset(schema, attrNum, &offset);
	if (attrOffsetStatus != RC_OK) {
		return attrOffsetStatus; // Handle the attrOffset function error
	}

	char *dataPointer = record->data + offset;
	DataType attrDataType = schema->dataTypes[attrNum];

	if (attrDataType == DT_STRING) {
        if (value->dt != DT_STRING) {
            RC_message = "Invalid attribute value";
        	return RC_ERROR;
        }
        int length = schema->typeLength[attrNum];
        // if (strlen(value->v.stringV) >= length) {
        // RC_message = "Invalid attribute value";
        // return RC_ERROR;
        // }
        strncpy(dataPointer, value->v.stringV, length);
        dataPointer += length;
    } else if (attrDataType == DT_FLOAT) {
        if (value->dt != DT_FLOAT) {
            RC_message = "Invalid attribute value";
        	return RC_ERROR;
        }
        *(float *)dataPointer = value->v.floatV;
        dataPointer += sizeof(float);
    } else if (attrDataType == DT_INT) {
        if (value->dt != DT_INT) {
            RC_message = "Invalid attribute value";
        	return RC_ERROR;
        }
        *(int *)dataPointer = value->v.intV;
        dataPointer += sizeof(int);
    } else if (attrDataType == DT_BOOL) {
        if (value->dt != DT_BOOL) {
            RC_message = "Invalid attribute value";
        	return RC_ERROR;
        }
        *(bool *)dataPointer = value->v.boolV;
        dataPointer += sizeof(bool);
    } else {
        RC_message = "Invalid attribute type";
        return RC_ERROR;
    }
	return RC_OK;
}
