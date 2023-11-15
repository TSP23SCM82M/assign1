#include "dberror.h"
#include "btree_mgr.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"
#include "tables.h"
#include "btree_implement.h"

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
    if (scanmeta->keyIndex == NULL)
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
      if (rid == -1)
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
  BTreeManager *treeManager = (BTreeManager *)tree->mgmtData;
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
    print("enqueue failed");
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