

**extern RC initRecordManager (void *mgmtData):**

    Function initializes the Record Manager by calling the initStorageManager() function. It is responsible for setting up the necessary data structures and resources required for the Record Manager to function properly.

**extern RC shutdownRecordManager ():**

    Function frees up the memory allocated to the record manager and completely destroys it. It is responsible for cleaning up the resources used by the Record Manager and ensuring that there are no memory leaks.

**extern RC createTable (char *name, Schema *schema):**

    The initializeTable function is responsible for initializing a new table.The record management structure's memory is then allocated after the relevant resources, such as the buffer pool, have been set up. It makes use of the openPageFile method to gain access to the newly constructed table after using the createPageFile function to create the table. Additionally, it employs the writeBlock function to write data to a particular block of the table before using the closePageFile function to close the file.  

**extern RC openTable (RM_TableData *rel, char *name):**
    
    Function is tasked with gaining access to the table with the name "name." This is achieved by using the PageHandle function. To remove the page from the buffer pool during this procedure, the openTable function uses the buffer manager unpinPage method. The forcePage function, which is also offered by the buffer manager, is used to write the page back to disk after it has been withdrawn from the buffer pool.  

**extern RC closeTable (RM_TableData *rel):**

    Function is responsible for completing the procedure on the table specified by the variable rel. It fully closes the table after first preserving its metadata. The closeTable function starts the Buffer Pools closure after preserving the metadata. Additionally, it frees up the RAM that the rel parameter was using.
        
**extern RC deleteTable (char *name):**

    The table with the name "name" must be deleted using the deleteTable function. The table is effectively deleted from storage as a result of this operation using the destroyPageFile function of the storage management.    

**extern int getNumTuples (RM_TableData *rel):** 

    The reference variable rel is used by the getNumTuples function to retrieve and return the number of tuples or records that are present in the table.

**extern RC insertRecord (RM_TableData *rel, Record *record):**

    The function labels the page as "dirty" to make sure the Buffer Manager writes the page's content back to the disk. The data of the supplied record (via the "record" option) is then copied into the new record using the memcpy() C function. The page is subsequently unpinned. Using a certain function, the record is added to the table, and its Record ID is created.

**extern RC deleteRecord (RM_TableData *rel, RID id):**

    This function is used to remove the record from the table by using the Record ID and references to the table name inside the provided bounds. In order to enable the Buffer Manager to store the page contents back to the disk, the function first marks the page as "dirty\" before unpinning it.

**extern RC updateRecord (RM_TableData *rel, Record *record):**

    The updateRecord functions goal is to make changes to a record that is referenced by the argument "record" and is part of the table that is referenced by the parameter "rel." It locates the page on which the record is stored using the table\'s metadata and pins that page in the buffer pool. In order to make the change, it then sets the Record ID and navigates to the location where the record\'s data is located.

**extern RC getRecord (RM_TableData *rel, RID id, Record *record):**

    The getRecord function fetches a record from the table referred to by the parameter "rel," using the Record ID "id" that is indicated in the parameter. The data is then copied, and the "record" argument's Record ID is set to match the ID of the record that is now on the page.

**extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond):**

    The RM ScanHandle data structure is passed as an input to the startScan function, which then uses it to start a scan operation.

**extern RC next (RM_ScanHandle *scan, Record *record):**

    The tuple that fulfills a specific criteria is returned by this function. "RC_RM_NO_MORE_TUPLES" is the error code that is returned if none of the tuples satisfy the required criteria.

**extern RC closeScan (RM_ScanHandle *scan):**

    To determine whether the scan is finished, this function evaluates the scanCount number found in the table's metadata. The scan was not completely done if the scanCount value is greater than 0.

**extern int getRecordSize (Schema *schema):**

    The getRecordSize function analyzes the schema, determines the record's size, and returns it.

**extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys):**

    The createSchema function handles memory allocation for global variables when it hasn't already been done. Then it returns the schema after creating it with values like numAttr, attrNames, dataTypes, typeLength, keys, and keySize.

**extern RC freeSchema (Schema *schema):**

    To free up memory allotted for a schema, use the freeSchema function. It makes sure memory resources are dealtlocated correctly, which helps the system avoid memory leaks.

**extern RC createRecord (Record **record, Schema *schema):**

    Setting the pointer and allocating memory for the schema and record are the duties of this function. The size of the current record is calculated, space is allotted for the record, and the page and slot are initialized.

**extern RC freeRecord (Record *record)**

    This function's goal is to release the memory that was previously reserved for the "record" within the given bounds.

**extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value):**

    The getAttr method sets aside memory for the Value data, records the data type, computes the length using a certain function, adds the length to the offset, and then returns the value of the '*value' parameter as well as the data type of the attribute.

**extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value):**

    With the use of arguments, the setAttr function transfers data between the record, schema, and attribute number while allocating memory for the Value data and determining the data type. It is in charge of determining the value of the property.

**Team Members**
Anwesha Nayak(A20512145): 20%
Taufeeq Ahmed Mohammed(A20512082): 20%
Shruti Shankar Shete(A20518508): 20%
Jianqi Jin(A20523325): 20%
Syed Zoraiz Sibtain (A20521018): 20%
