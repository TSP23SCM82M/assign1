
**Team Members**
Anwesha Nayak(A20512145): 20%
Taufeeq Ahmed Mohammed(A20512082): 20%
Shruti Shankar Shete(A20518508): 20%
Jianqi Jin(A20523325): 20%
Syed Zoraiz Sibtain (A20521018): 20%


RUNNING THE SCRIPT
=======================================


SOLUTION DESCRIPTION
=======================================

MakeFile was made using following tutorial -
http://mrbook.org/blog/tutorials/make/

This B+ Tree implementation follows proper memory management practices, minimizing variable usage and ensuring effective resource deallocation. The solution includes additional features such as support for different data types, tested through custom test cases.

1. CUSTOM B+ TREE FUNCTIONS (btree_implement.h)
=================================================

findLeaf(...): 
--> Locates the leaf node containing the entry with the specified key.

findRecord(...): 
--> Searches the B+ Tree for an entry with the specified key.

makeRecord(...): 
--> Creates a new record element encapsulating a RID.

InsertIntoLeaf(…): 
--> Inserts a new pointer to a record and its corresponding key into a leaf node.

createNewTree(...): 
--> Creates a new tree when the first entry is inserted.

createNode(...): 
--> Creates a new general node (leaf/internal/root).

createLeaf(...): 
--> Creates a new leaf node.

AfterSplitting(...): 
--> Inserts a new key and pointer into a leaf, splitting the leaf if necessary.

insertIntoNode(...): 
--> Inserts a new key and pointer into a node without violating B+ tree properties.

insertIntoNodeAfterSplitting(...): 
--> Inserts a new key and pointer into a non-leaf node, causing a split if needed.

insertIntoParent(...): 
--> Inserts a new node (leaf or internal) into the B+ tree.

insertIntoNewRoot(...): 
--> Creates a new root for two subtrees and inserts the appropriate key.

getLeftIndex(...): 
--> Finds the index of the parent's pointer to the left of the key to be inserted.

adjustNode(...): 
--> Adjusts the root after a record deletion.

mergeNodes(...): 
--> Combines (merges) a node that has become too small after deletion with a neighboring node.

redistributeNodes(...): 
--> Redistributes entries between two nodes after deletion.

deleteEntry(...): 
--> Deletes an entry from the B+ tree.

delete(...): 
--> Deletes the entry with the specified key.

deleteNodeEntry(...): 
--> Removes a record with the specified key from a node.

getNeighborIndex(...): 
--> Returns the index of a node's nearest neighbor to the left.

2. INITIALIZE AND SHUTDOWN INDEX MANAGER
=================================================

initIndexManager(...): Initializes the index manager by calling the Storage Manager's `initStorageManager(...)` function.
shutdownIndexManager(...): Shuts down the index manager, deallocating all resources and setting the treeManager pointer to NULL.

2. B+ TREE INDEX RELATED FUNCTIONS
=================================================

createBtree(...): 
--> Creates a new B+ Tree, initializes the TreeManager, Buffer Pool, and page using Storage Manager.

openBtree(...): 
--> Opens an existing B+ Tree, retrieves the TreeManager, and initializes the Buffer Pool.

closeBtree(...): 
--> Closes the B+ Tree, marking pages dirty, shutting down the buffer pool, and freeing allocated resources.

deleteBtree(...): 
--> Deletes the page file with the specified name using Storage Manager.

3. ACCESS INFORMATION ABOUT OUR B+ TREE
=================================================

getNumNodes(...): 
--> Returns the number of nodes in the B+ Tree.

getNumEntries(...): 
--> Returns the number of entries/records/keys in the B+ Tree.

getKeyType(...): 
--> Returns the datatype of the keys in the B+ Tree.

4. ACCESSING B+ TREE FUNCTIONS
=========================================

findKey(...): 
--> Searches the B+ Tree for a specified key, returning the corresponding RID.

insertKey(...): 
--> Inserts a new entry/record with a specified key and RID into the B+ Tree.

deleteKey(...): 
--> Deletes the entry/record with the specified key from the B+ Tree.

openTreeScan(...): 
--> Initializes a scan for traversing entries in sorted order.

nextEntry(...): 
--> Traverses to the next entry during a scan.

closeTreeScan(...): 
--> Closes the scan mechanism and frees up resources.

5. DEBUGGING AND TEST FUNCTIONS
=========================================

printTree(...): 
--> Prints the structure of the B+ Tree.

TEST CASES 2
===============

--> Additional test cases in "test_assign4_2.c" for inserting, finding, and deleting entries of different datatypes (float and string).
--> Follow the provided instructions to run these test cases.
--> Feel free to reach out if you have any questions or encounter issues.
