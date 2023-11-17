#ifndef BTREE_IMPLEMENT_H
#define BTREE_IMPLEMENT_H

#include "btree_mgr.h"
#include "buffer_mgr.h"

// Structure that holds the actual data of an entry
typedef struct NodeData
{
  RID rid;
} NodeData;

// Structure that represents a node in the B+ Tree
typedef struct Node
{
  void **pointers;
  Value **keys;
  struct Node *parent;
  bool is_leaf;
  int num_keys;
  struct Node *next; // Used for queue.
} Node;

// Structure that stores additional information of B+ Tree
typedef struct IndexManager
{
  BM_BufferPool bufferPool;
  BM_PageHandle pageHandler;
  int order;
  int numNodes;
  int numEntries;
  Node *root;
  Node *queue;
  DataType keyType;
} IndexManager;

// Structure that faciltates the scan operation on the B+ Tree
typedef struct ScanManager
{
  int keyIndex;
  int totalKeys;
  int order;
  Node *node;
} ScanManager;

// Functions to find an element (record) in the B+ Tree
Node *findLeaf(Node *root, Value *key);
NodeData *findRecord(Node *root, Value *key);

// Functions to support printing of the B+ Tree
int enqueue(IndexManager *treeManager, Node *new_node);
Node *dequeue(IndexManager *treeManager);
int path_to_root(Node *root, Node *child);

// Functions to support addition of an element (record) in the B+ Tree
NodeData *makeRecord(RID *rid);
Node *insertIntoLeaf(IndexManager *treeManager, Node *leaf, Value *key, NodeData *pointer);
Node *createNewTree(IndexManager *treeManager, Value *key, NodeData *pointer);
Node *createNode(IndexManager *treeManager);
Node *createLeaf(IndexManager *treeManager);
Node *insertLeafSplitting(IndexManager *treeManager, Node *leaf, Value *key, NodeData *pointer);
Node *insertIntoNode(IndexManager *treeManager, Node *parent, int left_index, Value *key, Node *right);
Node *insertIntoNodeAfterSplitting(IndexManager *treeManager, Node *parent, int left_index, Value *key, Node *right);
Node *insertIntoParent(IndexManager *treeManager, Node *left, Value *key, Node *right);
Node *insertIntoNewRoot(IndexManager *treeManager, Node *left, Value *key, Node *right);
int getLeftIndex(Node *parent, Node *left);

// Functions to support deleting of an element (record) in the B+ Tree
Node *adjustNode(Node *root);
Node *mergeNodes(IndexManager *treeManager, Node *n, Node *neighbor, int neighbor_index, int *k_prime);
Node *redistributeNodes(Node *root, Node *n, Node *neighbor, int neighbor_index, int k_prime_index, int k_prime);
Node *deleteEntry(IndexManager *treeManager, Node *n, Value *key, void *pointer);
Node *delete(IndexManager *treeManager, Value *key);
Node *deleteNodeEntry(IndexManager *treeManager, Node *n, Value *key, Node *pointer);
int getNeighborIndex(Node *n);

// Functions to support KEYS of multiple datatypes.
bool isLess(Value *key1, Value *key2);
bool isGreater(Value *key1, Value *key2);
bool isEqual(Value *key1, Value *key2);

#endif // BTREE_IMPLEMENT_H
