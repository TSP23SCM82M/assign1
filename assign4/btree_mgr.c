#include "dberror.h"
#include "btree_mgr.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"
#include "tables.h"
#include <stdlib.h>
#include "btree_implement.h"

IndexManager *treeManager = NULL;
static bool storageManagerInitialized = false;

extern RC initIndexManager(void *mgmtData) {

  if (!storageManagerInitialized) {
        RC initStorageResult = initStorageManager();
        if (initStorageResult != RC_OK) {
            return initStorageResult;  
        }
        storageManagerInitialized = true; 
    }
  if (mgmtData == NULL) {
        return RC_ERROR;
    }
	
	return RC_OK;
}


extern RC shutdownIndexManager ()
{
  if (treeManager != NULL) 
  {
    treeManager = NULL; 
  }
  else
  {
    return RC_ERROR;
  }
  return RC_OK;
}

extern RC createBtree(char *idxId, DataType keyType, int n) {
  if (idxId == NULL) {
        return RC_ERROR;
    }

	int max = PAGE_SIZE / sizeof(Node);
	if (n <= 0 || n > max) {
		return RC_ORDER_TOO_HIGH_FOR_PAGE;
	}

  if (keyType < DT_INT || keyType > DT_STRING) {
        return RC_ERROR;
    }

	treeManager = (IndexManager *) malloc(sizeof(IndexManager));

  if (treeManager == NULL) {
        return RC_ERROR;
    }

  BM_BufferPool * bm = (BM_BufferPool *) malloc(sizeof(BM_BufferPool));

  if (bm == NULL) {
        free(treeManager);
        return RC_ERROR;
    }
  treeManager->root = NULL;		
	treeManager->queue = NULL;
	treeManager->order = n + 2;		
	treeManager->numNodes = 0;		
	treeManager->numEntries = 0;			
	treeManager->keyType = keyType;	
	treeManager->bufferPool = *bm;

  RC result = createPageFile(idxId);
    if (result != RC_OK) {
        free(treeManager);
        return result;
    }

   SM_FileHandle fileHandler;
    result = openPageFile(idxId, &fileHandler);
    if (result != RC_OK) {
        free(treeManager);
        return result;
    }

  char data[PAGE_SIZE];
  result = writeBlock(0, &fileHandler, data);
    if (result != RC_OK) {
        free(treeManager);
        closePageFile(&fileHandler);
        return result;
    }

  result = closePageFile(&fileHandler);
    if (result != RC_OK) {
        free(treeManager);
        return result;
    }
	
  return RC_OK;
}

RC openBtree(BTreeHandle **tree, char *idxId) {
	
  if (idxId == NULL) 
  {
        return RC_ERROR;
  }

	*tree = (BTreeHandle *) malloc(sizeof(BTreeHandle));

  if (*tree == NULL) 
  {
        return RC_ERROR;
  }

	(*tree)->mgmtData = treeManager;

	if ((initBufferPool(&treeManager->bufferPool, idxId, 1000, RS_FIFO, NULL)) != RC_OK) {
        free(*tree);
        *tree = NULL;
        return RC_ERROR;
    }

    return RC_OK;
}

extern RC closeBtree (BTreeHandle *tree)
{

  if (tree == NULL || tree->mgmtData == NULL)
  {
        return RC_ERROR;
  }

  IndexManager *treeManager = (IndexManager *)tree->mgmtData;

  if (treeManager == NULL)
  {
    return RC_ERROR;
  }

  if (treeManager->bufferPool.pageFile == NULL) 
  {
        return RC_READ_NON_EXISTING_PAGE;
  }
  
  markDirty(&treeManager->bufferPool, &treeManager->pageHandler);
  shutdownBufferPool(&treeManager->bufferPool);
  free(tree->mgmtData);
  free(tree);
  return RC_OK;
}

extern RC deleteBtree (char *idxId)
{
   if (idxId == NULL) 
    {
        return RC_ERROR;
    }

  if (treeManager == NULL)
  {
      return RC_ERROR; 
  }

   destroyPageFile(idxId);

    return RC_OK;
}

// Number of nodes present in the B+ Tree.
RC getNumNodes(BTreeHandle *tree, int *result) {
	// Fetches metadata of B+ Tree.
	IndexManager * treeManager = (IndexManager *) tree->mgmtData;
	// Save the count in result
	*result = treeManager->numNodes;
	return RC_OK;
}

// To get number of entries in the B+ Tree.
RC getNumEntries(BTreeHandle *tree, int *result) {
	// Fetches metadata of B+ Tree.
	IndexManager * treeManager = (IndexManager *) tree->mgmtData;
	// Save the numEntries found in metadata to result.
	*result = treeManager->numEntries;
	return RC_OK;
}

// Gives datatype of the keys in the B+ Tree.
RC getKeyType(BTreeHandle *tree, DataType *result) {
	// Fetches metadata of B+ Tree.
	IndexManager * treeManager = (IndexManager *) tree->mgmtData;
	// Result stores the datatype of the Key.
	*result = treeManager->keyType;
	return RC_OK;
}

// zoraiz
extern RC findKey(BTreeHandle *tree, Value *key, RID *result) {
	
	// Check if the tree and its manager are valid
    if (tree == NULL || tree->mgmtData == NULL) {
        return RC_ERROR;
    }

    IndexManager *treeManager = (IndexManager *)tree->mgmtData;

    // Find the record with the given key
    NodeData *record = findRecord(treeManager->root, key);

    // Check if the record was not found
    if (record == NULL) {
        return RC_IM_KEY_NOT_FOUND;
    }

    // Copy the RID to the result
    *result = record->rid;

    return RC_OK;

}

// This function adds a new entry/record with the specified key and RID.
RC insertKey(BTreeHandle *tree, Value *key, RID rid) {
	// Retrieve B+ Tree's metadata information.
	IndexManager *treeManager = (IndexManager *)tree->mgmtData;

  // Check for NULL treeManager
  if (treeManager == NULL) {
      fprintf(stderr, "Invalid tree manager\n");
      return RC_ERROR;
  }

  // Check if order is not specified
  if (treeManager->order <= 0) {
      fprintf(stderr, "Invalid B+ tree order\n");
      return RC_ERROR;
  }

  // Use the specified order
	int bTreeOrder = treeManager->order;

  // Accessing pointer and leaf variables.
	NodeData *pointer;
	Node *leaf;

  // Check if a record with the specified key already exists.
	if (findRecord(treeManager->root, key) != NULL) {
      fprintf(stderr, "insertKey :: Key already exists\n");
      // Handle the error appropriately, e.g., return an error code or perform other actions.
		return RC_IM_KEY_ALREADY_EXISTS;
	}

	// Create a new record (NodeData) for the value RID.
	pointer = makeRecord(&rid);
  if (pointer == NULL) {
      fprintf(stderr, "Error creating record\n");
      // Handle the error appropriately, e.g., return an error code or perform other actions.
      return RC_ERROR;
  }

	// If the tree doesn't exist yet, create a new tree.
	if (treeManager->root == NULL) {
      // Create a new tree if the root is NULL
		treeManager->root = createNewTree(treeManager, key, pointer);

      // Check for errors during tree creation
      if (treeManager->root == NULL) {
          fprintf(stderr, "Error creating a new tree\n");
          // Handle the error appropriately, e.g., return an error code or perform other actions.
          return RC_ERROR;
      }
		return RC_OK;
	}


	// If the tree already exists, find a leaf where the key can be inserted.
	leaf = findLeaf(treeManager->root, key);

// Check if the leaf has room for the new key.
	if (leaf->num_keys < bTreeOrder - 1) {
		// If the leaf has room, insert the new key into that leaf.
		leaf = insertIntoLeaf(treeManager, leaf, key, pointer);
	} else {
		// If the leaf doesn't have room, split the leaf and then insert the new key into that leaf.
		Node *newRoot = insertLeafSplitting(treeManager, leaf, key, pointer);

      // Check for errors during leaf splitting
      if (newRoot == NULL) {
          fprintf(stderr, "Error inserting into leaf after splitting\n");
          // Handle the error appropriately, e.g., return an error code or perform other actions.
          return RC_ERROR;
      }

      // Update the tree's root after leaf splitting.
    treeManager->root = newRoot;
	}
	return RC_OK;
}

// RC insertKey_origin(BTreeHandle *tree, Value *key, RID rid) {
	
// 	// Check if the tree and its manager are valid
//     if (tree == NULL || tree->mgmtData == NULL) {
//         return RC_ERROR;
//     }

//     IndexManager *treeManager = (IndexManager *)tree->mgmtData;

//     // Ensure key does not already exist
//     if (findRecord(treeManager->root, key) != NULL) {
//         printf("\n insertKey :: KEY EXISTS");
//         return RC_IM_KEY_ALREADY_EXISTS;
//     }

//     NodeData *pointer = makeRecord(&rid);
//     Node *leaf;

//     int bTreeOrder = treeManager->order;

//     // If the tree is empty, create a new tree
//     if (treeManager->root == NULL) {
//         treeManager->root = createNewTree(treeManager, key, pointer);
//         return RC_OK;
//     }

//     // Find the leaf to insert into
//     leaf = findLeaf(treeManager->root, key);

//     // Check if there's enough space in the leaf
//     if (leaf->num_keys < bTreeOrder - 1) {
//         leaf = insertIntoLeaf(treeManager, leaf, key, pointer);
//     } else {
//         if (treeManager == NULL || leaf == NULL) {
//             return RC_ERROR;
//         }

//         // Split the leaf and update the root
//         Node *newRoot = insertLeafSplitting(treeManager, leaf, key, pointer);

//         // Check if the split was successful
//         if (newRoot == NULL) {
//             return RC_ERROR; // or another appropriate error code
//         }

//         treeManager->root = newRoot;

//         return RC_OK;
//     }

//     return RC_OK;

// }

// This function deletes the entry/record with the specified "key" in the B+ Tree.
RC deleteKey(BTreeHandle *tree, Value *key) {
	// Retrieve B+ Tree's metadata information.
	IndexManager *treeManager = (IndexManager *)tree->mgmtData;

  // Check for NULL treeManager
  if (treeManager == NULL) {
      fprintf(stderr, "Invalid tree manager\n");
      return RC_ERROR;
  }

	// Deleting the entry with the specified key.
	treeManager->root = delete(treeManager, key);

  // Check for errors during deletion
  if (treeManager->root == NULL) {
      fprintf(stderr, "Error deleting entry with key %d\n", key);
      return RC_ERROR;
  }

	//printTree(tree); // Uncomment if needed
	return RC_OK;
}

// RC deleteKey_origin(BTreeHandle *tree, Value *key) {
	
// 	if (tree == NULL || tree->mgmtData == NULL) {
//     return RC_ERROR;
// 	}

// 	IndexManager *treeManager = (IndexManager *)tree->mgmtData;

// 	// Ensure treeManager has a valid root
// 	if (treeManager->root == NULL) {
// 		return RC_ERROR;
// 	}

// 	// Attempt to delete the key from the B-tree
// 	treeManager->root = deleteKey(treeManager, key);

// 	// Check if the deletion was successful
// 	if (treeManager->root == NULL) {
// 		return RC_IM_NO_MORE_ENTRIES; // or another appropriate error code
// 	}

// 	return RC_OK;

// }

// // function for next entry
// RC nextEntry (BT_ScanHandle *handle, RID *result)
// {

//   ScanManager * scanmeta = (ScanManager *) handle->mgmtData;

	
// 	int keyIndex = scanmeta->keyIndex;
// 	int totalKeys = scanmeta->totalKeys;
// 	int bTreeOrder = scanmeta->order;
// 	RID rid;

	
// 	Node * node = scanmeta->node;

	
// 	if (node == NULL) {
// 		return RC_IM_NO_MORE_ENTRIES;
// 	}

// 	if (keyIndex < totalKeys) {
		
// 		rid = ((NodeData *) node->pointers[keyIndex])->rid;
		
		
// 		scanmeta->keyIndex++;
// 	} else {
		
// 		if (node->pointers[bTreeOrder - 1] != NULL) {
// 			node = node->pointers[bTreeOrder - 1];
// 			scanmeta->keyIndex = 1;
// 			scanmeta->totalKeys = node->num_keys;
// 			scanmeta->node = node;
// 			rid = ((NodeData *) node->pointers[0])->rid;
			
// 		} else {
			
// 			return RC_IM_NO_MORE_ENTRIES;
// 		}
// 	}
	
// 	*result = rid;
// 	return RC_OK;

// }

// // function to close tree scan
// RC closeTreeScan (BT_ScanHandle *handle)
// {
//     if (handle == NULL) {
//         return RC_SCAN_CONDITION_NOT_FOUND;
//     }

//     // Additional cleanup or resource release if needed

//     free(handle);
//     handle = NULL;

//     return RC_OK;
// }

// //function to print tree
// char *printTree (BTreeHandle *tree)
// {
//     if (tree == NULL) {
//         // Handle the case where the tree handle is not initialized
//         return "Error: B-tree handle is not initialized.";
//     }

//     // Implement tree printing logic here

//     // Example: Print the number of nodes in the tree
//     int numNodes;
//     RC result = getNumNodes(tree, &numNodes);
//     if (result != RC_OK) {
//         // Handle the error, e.g., log it or return an error message
//         return "Error: Unable to retrieve the number of nodes.";
//     }

//     // Placeholder for actual tree printing logic
//     char *treePrintOutput = malloc(100); // Adjust size accordingly
//     snprintf(treePrintOutput, 100, "Number of nodes in the tree: %d", numNodes);

//     return treePrintOutput;
// }
// zoraiz

// This function initializes the scan which is used to scan the entries in the B+ Tree.
RC openTreeScan(BTreeHandle *tree, BT_ScanHandle **handle) {
	// Retrieve B+ Tree's metadata information.
	IndexManager *treeManager = (IndexManager *)tree->mgmtData;

  // Check for NULL treeManager
    if (treeManager == NULL) {
        fprintf(stderr, "Invalid tree manager\n");
        return RC_ERROR;
    }

  Node* node = treeManager->root;

	// Retrieve B+ Tree Scan's metadata information.
  // Allocate memory for scanmeta
	ScanManager *scanmeta = malloc(sizeof(ScanManager));

	// Allocate memory for the handle.
	*handle = malloc(sizeof(BT_ScanHandle));


	if (treeManager->root == NULL) {
		//printf("Empty tree.\n");
		return RC_NO_RECORDS_TO_SCAN;
	} else {
		//printf("\n openTreeScan() ......... Inside ELse  ");
		while (!node->is_leaf) {
      node = node->pointers[0];
    }

		// Initializing (setting) the Scan's metadata information.
		scanmeta->keyIndex = 0;

    if (scanmeta == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation error for ScanMeta\n");
        return RC_ERROR;
    }
		scanmeta->totalKeys = node->num_keys;
		scanmeta->node = node;
    // Set mgmtData of handle to scanmeta
    if (handle == NULL || *handle == NULL) {
        // Handle invalid handle
        fprintf(stderr, "Invalid handle\n");
        free(scanmeta); // Free allocated memory before returning
        return RC_ERROR;
    }

		scanmeta->order = treeManager->order;


		(*handle)->mgmtData = scanmeta;
		//printf("\n keyIndex = %d, totalKeys = %d ", scanmeta->keyIndex, scanmeta->totalKeys);
	}
	return RC_OK;
}

// This function traverses the B+ Tree's entries and stores the record details (RID) in the memory location pointed to by the "result" parameter.
RC nextEntry(BT_ScanHandle *handle, RID *result)
{
  // Retrieve the metadata information for the B+ Tree Scan.
  ScanManager *scanmeta = (ScanManager *)handle->mgmtData;
  if (scanmeta == NULL)
  {
    return RC_ERROR;
  }
  // Get all necessary information.
  int bTreeOrder = scanmeta->order;
  RID rid;
  int totalKeys = scanmeta->totalKeys;
  Node *node = scanmeta->node;
  int keyIndex = scanmeta->keyIndex;

  // If the current node is null, there are no more entries.
  if (node == NULL || keyIndex == -1)
  {
    printf("Node is NULL");
    return RC_IM_NO_MORE_ENTRIES;
  }

  // Check if the current key index is within the total number of keys.
  if (keyIndex < totalKeys)
  {
    if (keyIndex >= totalKeys)
    {
      return RC_ERROR;
    }
    // Get the RID from the current key entry in the same leaf node.
    if (node->pointers[keyIndex] == NULL)
    {
      return RC_ERROR;
    }
    rid = ((NodeData *)node->pointers[keyIndex])->rid;
    // Move to the next key index.
    if (scanmeta->keyIndex == -1)
    {
      return RC_ERROR;
    }
    scanmeta->keyIndex++;
  }
  else
  {
    if (keyIndex < totalKeys)
    {
      return RC_ERROR;
    }
    // If all entries on the current leaf node have been scanned, move to the next node...
    if (node->pointers[bTreeOrder - 1] != NULL)
    {
      node = node->pointers[bTreeOrder - 1];
      // Reset the key index for the next node and update the total keys count.
      if (node == NULL)
      {
        return RC_ERROR;
      }
      scanmeta->keyIndex = 1;
      scanmeta->totalKeys = node->num_keys;
      if (scanmeta == NULL)
      {
        return RC_ERROR;
      }
      // Update the scan's current node to the next node.
      scanmeta->node = node;
      if (scanmeta->node == NULL)
      {
        return RC_ERROR;
      }
      // Get the RID from the first entry in the new node.
      rid = ((NodeData *)node->pointers[0])->rid;
      // -1 means invalid
      if (&rid == NULL)
      {
        return RC_ERROR;
      }
    }
    else
    {
      if (node->pointers[bTreeOrder - 1] == NULL)
      {
        // If there are no more nodes to scan, indicate that no more entries are left.
        return RC_IM_NO_MORE_ENTRIES;
      }
    }
  }
  // Assign the record/result/RID to the result pointer.
  *result = rid;
  // Return success status.
  return RC_OK;
}

// This function closes the B+ Tree scan and releases resources.
extern RC closeTreeScan(BT_ScanHandle *handle)
{
  // Nullify the management data pointer as a part of cleanup.
  handle->mgmtData = NULL;
  if (handle->mgmtData != NULL)
  {
    return RC_ERROR;
  }
  // Deallocate the memory for the scan handle.
  free(handle);
  // Return success status.
  return RC_OK;
}

// This function prints the B+ Tree structure.
extern char *printTree(BTreeHandle *tree)
{
  // Get the tree manager from the BTree handle.
  IndexManager *treeManager = (IndexManager *)tree->mgmtData;
  if (treeManager == NULL || tree == NULL)
  {
    printf("ERROR");
    return '\0';
  }
  int rank = 0;
  printf("\nPRINTING TREE:\n");
  int i = 0;
  Node *n = NULL;
  int new_rank = 0;

  // If the root of the tree is NULL, the tree is empty.
  if (treeManager == NULL || treeManager->root == NULL)
  {
    printf("Empty tree.\n");
    return '\0';
  }
  // Initialize the queue to start the level order print.
  treeManager->queue = NULL;
  if (enqueue(treeManager, treeManager->root) == -1)
  {
    printf("enqueue failed");
    return '\0';
  }
  // Perform level order traversal using the queue.
  while (treeManager->queue != NULL)
  {
    n = dequeue(treeManager);
    // Check if a new tree level is reached.
    if (n == n->parent->pointers[0] && n->parent != NULL)
    {
      if (treeManager->root == NULL) {
        printf("ERROR treeManager.root is NULL");
        return '\0';
      }
      new_rank = path_to_root(treeManager->root, n);
      if (new_rank != rank)
      {
        rank = new_rank;
        printf("\n");
      }
    }
    if (treeManager->queue == NULL)
    {
      break;
    }

    // Print the keys depending on the data type.
    for (i = 0; i < n->num_keys; i++)
    {
      // Based on the key type, format the print statement accordingly.
      if (treeManager->keyType == DT_INT)
      {
        printf("%d ", (*n->keys[i]).v.intV);
        break;
      }
      if (treeManager->keyType == DT_FLOAT)
      {
        printf("%.02f ", (*n->keys[i]).v.floatV);
        break;
      }
      if (treeManager->keyType == DT_STRING)
      {
        printf("%s ", (*n->keys[i]).v.stringV);
        break;
      }
      if (treeManager->keyType == DT_BOOL)
      {
        printf("%d ", (*n->keys[i]).v.boolV);
        break;
      }
      // Print the associated RID with each key.
      int val1 = ((NodeData *)n->pointers[i])->rid.page;
      int val2 = ((NodeData *)n->pointers[i])->rid.slot;
      printf("(%d - %d) ", val1, val2);
    }
    // If the node is not a leaf, enqueue its children to continue the level order traversal.
    if (!n->is_leaf)
      for (i = 0; i <= n->num_keys; i++)
      {
        if (treeManager != NULL)
        {
          enqueue(treeManager, n->pointers[i]);
        }
      }

    // Print a separator between nodes.
    printf("| ");
  }
  printf("\n");

  // Return an 'empty string' as the function's required return type is char*.
  return '\0';
}