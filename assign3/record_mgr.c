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

typedef struct RM_ScanData {
    Expr *condition;  // The condition for scanning
    int currentRecordPage;  // The current page being scanned
    int currentRecordSlot;  // The current slot on the current page
    RecordMgr *recordManager;  // Reference to the Record Manager
} RM_ScanData;


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
    char *destination = &rm->pageHandle.data[slot_offset];
    memcpy(destination, source, getRecordSize(rel->schema));

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
    memcpy(destination, source, getRecordSize(rel->schema));

    // Set the record's ID
    record->id.page = id.page;
    record->id.slot = id.slot;

    // Unpin the page
    unpinPage(&rm->bufferPool, &rm->pageHandle);

    return RC_OK;

}

// scans
extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
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

extern RC next (RM_ScanHandle *scan, Record *record)
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

    // Initialize the Schema struct members
    sc->numAttr = numAttr;
    sc->attrNames = attrNames;
    sc->dataTypes = dataTypes;
    sc->typeLength = typeLength;
    sc->keySize = keySize;
    sc->keyAttrs = keys;

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
    
    int attr = 0;
    *result = 0;
    
    for (int attr = 0; attr < attrNum; attr++) {
        switch (schema->dataTypes[attr]) {
			case DT_INT:
                *result += sizeof(int);
                break;
            case DT_STRING:
                *result += schema->typeLength[attr];
                break;
			case DT_FLOAT:
                *result += sizeof(float);
                break;
            case DT_BOOL:
                *result += sizeof(bool);
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
						attribute->v.stringV[schema->typeLength[attrNum]] = '\0';
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
		value = attribute;
		return RC_OK;
	}

	*value = attribute;
	return RC_OK;
	free(attribute);
}

extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value)
{
	int length;
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
extern RC closeTable(RM_TableData* rel)
{
    // Store the table's metadata before closing it
    record_Info* rm = rel->mgmtData;

    // Shutdown the Buffer Pool
    shutdownBufferPool(&rm->bmBufferManager);
    
    // Free the schema
    freeSchema(rel->schema);
    
    return RC_OK;
}

extern RC deleteTable(char* name)
{
    RC result = destroyPageFile(name); // Attempt to delete the page file

    if (result == RC_OK)
    {
        return RC_OK; // File was successfully deleted
    }
    else (result == RC_FILE_NOT_FOUND)
    {
        return RC_FILE_NOT_FOUND; // File not found, no need to delete
    }
}

extern int getNumTuples(RM_TableData* rel)
{
	record_Info* rm = rel->mgmtData;
	return rm->tuple_count;
	
}

extern RC insertRecord(RM_TableData *rel, Record *record)
{
    record_Info *mgr = rel->mgmtData;
    RC returnCode = RC_OK;
    RID *recordId = &record->id;

    // Try to find an available slot on an existing page
    for (recordId->page = 1; recordId->page <= mgr->numPages; recordId->page++) {
        // Pin the page
        pinPage(&mgr->bmBufferManager, &mgr->pageHandle, recordId->page);

        // Find an empty slot on the page
        recordId->slot = Page_empty_slot(getRecordSize(rel->schema), mgr->pageHandle.data);

        if (recordId->slot >= 0) {
            // Mark the page as dirty, as we are modifying it
            markDirty(&mgr->bmBufferManager, &mgr->pageHandle);

            // Calculate the pointer to the slot where the record will be inserted
            char *slotPointer = mgr->pageHandle.data + recordId->slot * getRecordSize(rel->schema);

            // Set a marker ('$') to indicate that the slot is now in use
            *slotPointer = '$';

            // Copy the record data (excluding the marker) to the slot
            memcpy(slotPointer + 1, record->data, getRecordSize(rel->schema) - 1);

            // Unpin the page
            unpinPage(&mgr->bmBufferManager, &mgr->pageHandle);

            // Update the tuple count in the table's metadata
            mgr->tuple_count += 1;

            return returnCode;
        }

        // Unpin the page if no empty slot was found
        unpinPage(&mgr->bmBufferManager, &mgr->pageHandle);
    }

    // If no empty slot was found on any existing page, append a new page
    recordId->page = mgr->numPages + 1;

    // Allocate a new page
    ensureCapacity(recordId->page, &mgr->bmBufferManager);

    // Pin the new page
    pinPage(&mgr->bmBufferManager, &mgr->pageHandle, recordId->page);

    // Find the first slot on the new page
    recordId->slot = Page_empty_slot(getRecordSize(rel->schema), mgr->pageHandle.data);

    // Mark the page as dirty, as we are modifying it
    markDirty(&mgr->bmBufferManager, &mgr->pageHandle);

    // Calculate the pointer to the slot where the record will be inserted
    char *slotPointer = mgr->pageHandle.data + recordId->slot * getRecordSize(rel->schema);

    // Set a marker ('$') to indicate that the slot is now in use
    *slotPointer = '$';

    // Copy the record data (excluding the marker) to the slot
    memcpy(slotPointer + 1, record->data, getRecordSize(rel->schema) - 1);

    // Unpin the new page
    unpinPage(&mgr->bmBufferManager, &mgr->pageHandle);

    // Update the tuple count in the table's metadata
    mgr->tuple_count += 1;

    return returnCode;
}
