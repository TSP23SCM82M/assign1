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
	RC code = pinPage(&recordMgr->bufferPool, &recordMgr->pageHandle, 0);
	if (RC_OK != code) {
		return RC_ERROR;
	}
	// Setting table's meta data to our custom record manager meta data structure
	rel->mgmtData = recordMgr;
	// Setting the table's name
	rel->name = name;
    loadSchema(schema);
	rel->schema = schema;
    code = unpinPage(&recordMgr->bufferPool, &recordMgr->pageHandle);
	if (RC_OK != code) {
		return RC_ERROR;
	}

    code = forcePage(&recordMgr->bufferPool, &recordMgr->pageHandle);
	if (RC_OK != code) {
		return RC_ERROR;
	}

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
            return RC_ERROR; // File not found, no need to delete
        } else {
                return result; // Some other error occurred during deletion
            }
        }
    return RC_OK; // File was successfully deleted
}

extern int getNumTuples(RM_TableData* rel)
{
	RecordMgr* recordMgr = rel->mgmtData;
	if (!recordMgr) {
		return RC_ERROR;
	}
	return recordMgr->tuplesCount;
	
}

extern RC insertRecord(RM_TableData *rel, Record *record)
{
    RecordMgr* recordMgr = rel->mgmtData;
    RC returnCode = RC_OK;
    RID *recordID = &record->id;
	if (!recordID) {
		return RC_ERROR;
	}
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
	int size = recordID->slot;
	size *= recordSize;
    slot_p += size;
    *slot_p = '+';
	slot_p += 1;
    memcpy(slot_p, record->data + 1, recordSize - 1);
    RC code = unpinPage(&recordMgr->bufferPool, &recordMgr->pageHandle);
	if (RC_OK != code) {
		return RC_ERROR;
	}
    recordMgr->tuplesCount++;
    code = pinPage(&recordMgr->bufferPool, &recordMgr->pageHandle, 0);
	if (RC_OK != code) {
		return RC_ERROR;
	}

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
    RC code = pinPage(&rm->bufferPool, &rm->pageHandle, id.page);
	if (RC_OK != code) {
		return RC_ERROR;
	}

    // Calculate the offset to the slot within the page
    int slot_offset = id.slot * getRecordSize(rel->schema);

    // Check if the slot contains a valid record
    if (slot_offset >= PAGE_SIZE || rm->pageHandle.data[slot_offset] == 0) {
        RC code = unpinPage(&rm->bufferPool, &rm->pageHandle);
		if (RC_OK != code) {
			return RC_ERROR;
		}
        return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
    }

    // Mark the slot as empty (delete the record)
    memset(&rm->pageHandle.data + slot_offset, 0, getRecordSize(rel->schema));

    // Mark the page as dirty
    RC res = markDirty(&rm->bufferPool, &rm->pageHandle);
	if (res && res != RC_OK) {
		return RC_ERROR;
	}

    // Unpin the page
    res = unpinPage(&rm->bufferPool, &rm->pageHandle);
	if (res && res != RC_OK) {
		return RC_ERROR;
	}

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
        RC unpinCode = unpinPage(&rm->bufferPool, &rm->pageHandle);
		if (unpinCode || unpinCode != RC_OK) {
			return RC_ERROR;
		}
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
    RC res = unpinPage(&rm->bufferPool, &rm->pageHandle);
	if (res && res != RC_OK) {
		return res;
	}

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
    RC code = unpinPage(&rm->bufferPool, &rm->pageHandle);
	if (code && code != RC_OK) {
		return code;
	}

    return RC_OK;

}

// scans
extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{

    if (NULL == cond){
		return RC_SCAN_CONDITION_NOT_FOUND || RC_ERROR;
	}

if (rel == NULL || scan == NULL) {
        return RC_FILE_NOT_FOUND;
    }

	openTable(rel, rel->name);
    RecordMgr *scanMgr = (RecordMgr*) malloc(sizeof(RecordMgr));
    scanMgr->recordID.page = 1;
	scanMgr->recordID.slot = 0;  	
	scanMgr->scanCount = 0;
    scanMgr->condition = cond;
    
   
    RecordMgr *tableMgr = rel->mgmtData;
    tableMgr->tuplesCount = SIZE_ATTR;

    	
    scan->mgmtData = scanMgr;
    scan->rel= rel;

    return RC_OK;

}


extern RC next(RM_ScanHandle *scan, Record *record)
{
    if (scan == NULL || record == NULL)
    {
        return RC_FILE_NOT_FOUND;
    }

    RecordMgr *scanManager = scan->mgmtData;
    RM_TableData *rel = scan->rel;
    RecordMgr *tableManager = rel->mgmtData;
    Schema *schema = rel->schema;

    if (scanManager->condition == NULL)
    {
        return RC_SCAN_CONDITION_NOT_FOUND;
    }

    Value *result = (Value *)malloc(sizeof(Value));
    int recordSize = getRecordSize(schema);
    int totalSlots = PAGE_SIZE / recordSize;
    int tuplesCount = tableManager->tuplesCount;

    // If there are no tuples or we have scanned all the tuples, return RC_RM_NO_MORE_TUPLES
    if (tuplesCount == 0 || scanManager->scanCount >= tuplesCount)
    {
        return RC_RM_NO_MORE_TUPLES;
    }

    while (tuplesCount >= scanManager->scanCount)
    {
		if (0 >= scanManager->scanCount)
		{
			scanManager->recordID.slot = 0;
			scanManager->recordID.page = 1;
		}
		else
		{
			scanManager->recordID.slot += 1;
			if(scanManager->recordID.slot < totalSlots)
			{ } else {
				scanManager->recordID.page += 1;
				scanManager->recordID.slot = 0;
			}
		}

        // Pin the page
        pinPage(&tableManager->bufferPool, &scanManager->pageHandle, scanManager->recordID.page);
        BM_PageHandle pageHandle = scanManager->pageHandle;
		char *data = pageHandle.data;

		int size = scanManager->recordID.slot;
		size *= recordSize;
        // Calculate the data location from the record's slot and record size
        data += size;

        // Set the record's ID
		RID recordID = scanManager->recordID;
        record->id.page = recordID.page;
        record->id.slot = recordID.slot;

        // Initialize the record data's first location with Tombstone mechanism
		char *dataPointer;
        if (!&recordID) {
			RC_message = "recordID is empty";
		}
		dataPointer = record->data;
        *dataPointer = '-';

        // Copy the data from the page to the record
		dataPointer += 1;
        memcpy(dataPointer, data + 1, recordSize - 1);

        scanManager->scanCount += 1;

        // Test the record for the specified condition (test expression)
        evalExpr(record, schema, scanManager->condition, &result);

        // If the record satisfies the condition, unpin the page and return RC_OK
        if (result->v.boolV)
        {
            unpinPage(&tableManager->bufferPool, &scanManager->pageHandle);
            return RC_OK;
        }

        // Move to the next page or slot
        scanManager->recordID.slot += 1;
        if (totalSlots > scanManager->recordID.slot)
        {
        } else {
            scanManager->recordID.slot = 0;
            scanManager->recordID.page += 1;
		}
    }
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
	
	char *dataPointer = NULL;
	Value *attribute = NULL;
	if (dataPointer || attribute) {
		return RC_ERROR;
	}
	int offset = 0;

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
