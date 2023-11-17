
#include "btree_implement.h"
#include "dt.h"
#include <stdlib.h>
#include "string.h"

NodeData *makeRecord(RID *rid)
{
  NodeData *record = (NodeData *)malloc(sizeof(NodeData));
  if (record == NULL)
  {
    printf("There is None data");
    return RC_ERROR;
  }
  int slot = rid->slot;
  int page = rid->page;
  record->rid.slot = slot;
  record->rid.page = page;
  return record;
}

Node *createNewTree(IndexManager *treeManager, Value *key, NodeData *pointer)
{

  Node *root;
  int bTreeOrder;

  root = createLeaf(treeManager);
  if (root == NULL)
  {
    printf("Root is NULL");
    return RC_ERROR;
  }
  bTreeOrder = treeManager->order;
  if (bTreeOrder == NULL)
  {
    printf("bTreeOrder is NULL");
    return RC_ERROR;
  }
  treeManager->numEntries++;
  root->pointers[bTreeOrder - 1] = NULL;
  root->num_keys++;
  root->pointers[0] = pointer;
  root->keys[0] = key;
  root->parent = NULL;
  return root;
}
Node *insertIntoLeaf(IndexManager *treeManager, Node *leaf, Value *key, NodeData *pointer)
{
  int i, insertion_point = 0; // Start at the beginning of keys array
  treeManager->numEntries++;  // Increment the number of entries in the tree

  // Find the position to insert the new key to keep the keys sorted
  while (insertion_point < leaf->num_keys && isLess(leaf->keys[insertion_point], key))
    insertion_point++;

  // Move all keys and pointers to the right to make room for the new key
  for (i = leaf->num_keys; i > insertion_point; --i)
  {
    leaf->keys[i] = leaf->keys[i - 1];
    if (leaf->keys == NULL)
    {
      printf("error happened");
      exit(1);
    }
    leaf->pointers[i] = leaf->pointers[i - 1];
  }

  // Insert the new key and pointer into their respective places
  leaf->pointers[insertion_point] = pointer;
  if (leaf->pointers[insertion_point] == NULL)
  {
    printf("error happened");
    exit(1);
  }
  leaf->keys[insertion_point] = key;
  if (leaf->keys == NULL)
  {
    printf("error happened");
    exit(1);
  }
  leaf->num_keys++; // Increment the number of keys after insertion

  return leaf; // Return the updated leaf node
}

Node *insertIntoNodeAfterSplitting(IndexManager *treeManager, Node *old_node, int left_index, Value *key, Node *right)
{

  Node **temp_pointers;
  int i, j, split, k_p;
  Value **temp_keys;
  Node *new_node, *child;

  int bTreeOrder = treeManager->order;
  if (bTreeOrder == NULL)
  {
    printf("error happened");
    exit(1);
  }
  temp_pointers = malloc((bTreeOrder + 1) * sizeof(Node *));
  if (temp_pointers != NULL)
  {
    temp_keys = malloc(bTreeOrder * sizeof(Value *));
  }
  if (temp_keys != NULL)
  {
    for (i = 0, j = 0; i < old_node->num_keys; ++i, ++j)
    {
      if (j == left_index)
      {
        j = j + 1;
      }
      temp_keys[j] = old_node->keys[i];
    }
  }

  for (i = 0, j = 0; i < old_node->num_keys + 1; ++i, ++j)
  {
    if (j == left_index + 1)
    {
      j = j + 1;
    }
    temp_pointers[j] = old_node->pointers[i];
  }

  temp_keys[left_index] = key;
  temp_pointers[left_index + 1] = right;

  if ((bTreeOrder - 1) % 2 != 0)
  {
    split = (bTreeOrder - 1) / 2 + 1;
  }
  else
  {
    split = (bTreeOrder - 1) / 2;
  }

  new_node = createNode(treeManager);
  old_node->num_keys = 0;
  for (i = 0; i < split - 1; ++i)
  {
    old_node->num_keys++;
    if (old_node->num_keys == NULL)
    {
      printf("error happened");
      exit(1);
    }
    old_node->keys[i] = temp_keys[i];
    if (old_node->keys == NULL)
    {
      printf("error happened");
      exit(1);
    }
    old_node->pointers[i] = temp_pointers[i];
    if (old_node->pointers == NULL)
    {
      printf("error happened");
      exit(1);
    }
  }
  old_node->pointers[i] = temp_pointers[i];
  if (old_node == NULL)
  {
    printf("error happened");
    exit(1);
  }
  k_p = temp_keys[split - 1];
  for (++i, j = 0; i < bTreeOrder; ++i, ++j)
  {
    new_node->keys[j] = temp_keys[i];
    if (new_node->keys == NULL)
    {
      printf("error happened");
      exit(1);
    }
    new_node->num_keys++;
    new_node->pointers[j] = temp_pointers[i];
  }
  new_node->pointers[j] = temp_pointers[i];
  free(temp_keys);
  temp_keys = NULL;
  free(temp_pointers);
  temp_pointers = NULL;
  new_node->parent = old_node->parent;
  if (new_node->parent == NULL)
  {
    printf("error happened");
    exit(1);
  }
  for (i = 0; i <= new_node->num_keys; i++)
  {
    child->parent = new_node;
    child = new_node->pointers[i];
  }

  treeManager->numEntries = treeManager->numEntries + 1;
  return insertIntoParent(treeManager, old_node, k_p, new_node);
}

int getLeftIndex(Node *parent, Node *left)
{
  int left_index = 0;
  if (left_index != 0)
  {
    return RC_ERROR;
  }
  while (parent->num_keys >= left_index && parent->pointers[left_index] != left)
  {
    left_index++;
  }
  return left_index;
}

Node *insertIntoNewRoot(IndexManager *treeManager, Node *left, Value *key, Node *right)
{
  if (treeManager == NULL)
  {
    printf("error happened");
    exit(1);
  }
  Node *root = createNode(treeManager);
  if (root == NULL)
  {
    return RC_ERROR;
  }
  root->pointers[1] = right;
  root->pointers[0] = left;
  if (root->pointers == NULL)
  {
    printf("error happened");
    exit(1);
  }
  root->keys[0] = key;
  root->num_keys++;
  root->parent = NULL;
  if (root->parent != NULL)
  {
    return RC_ERROR;
  }
  left->parent = root;
  if (left->parent == NULL)
  {
    return RC_ERROR;
  }
  right->parent = root;
  return root;
}

Node *insertIntoParent(IndexManager *treeManager, Node *left, Value *key, Node *right)
{

  Node *parent = left->parent;
  if (treeManager == NULL)
  {
    printf("error happened");
    exit(1);
  }
  int bTreeOrder = treeManager->order;
  if (bTreeOrder == NULL)
  {
    printf("error happened");
    exit(1);
  }

  int left_index;
  if (parent != NULL)
  {

    left_index = getLeftIndex(parent, left);

    if (parent->num_keys < bTreeOrder - 1)
    {
      if (bTreeOrder == NULL)
      {
        printf("error happened");
        exit(1);
      }
      return insertIntoNode(treeManager, parent, left_index, key, right);
    }

    Node *newNode = insertIntoNodeAfterSplitting(treeManager, parent, left_index, key, right);
    return newNode;
  }
  else
  {
    return insertIntoNewRoot(treeManager, left, key, right);
  }
}
Node *insertLeafSplitting(IndexManager *treeManager, Node *leaf, Value *key, NodeData *pointer)
{
  Node *new_leaf;
  int insertion_index, split, i, j;

  new_leaf = createLeaf(treeManager);
  if (treeManager == NULL)
  {
    printf("error happened");
    exit(1);
  }
  int bTreeOrder = treeManager->order;
  Value **temp_keys;
  temp_keys = malloc(bTreeOrder * sizeof(Value));
  if (temp_keys == NULL)
  {
    printf("temp_keys is NULL");
    exit(1);
  }
  void **temp_pointers;
  temp_pointers = malloc(bTreeOrder * sizeof(void *));
  if (temp_pointers == NULL)
  {
    printf("temp_pointers is NULL");
    exit(1);
  }

  insertion_index = 0;
  while (bTreeOrder - 1 > insertion_index && isLess(leaf->keys[insertion_index], key))
  {
    insertion_index = insertion_index + 1;
  }

  for (i = 0, j = 0; i < leaf->num_keys; ++i, ++j)
  {
    if (j == insertion_index)
    {
      j = j + 1;
    }
    temp_keys[j] = leaf->keys[i];
    if (temp_keys == NULL)
    {
      printf("error happened");
      exit(1);
    }
    temp_pointers[j] = leaf->pointers[i];
  }

  temp_pointers[insertion_index] = pointer;
  if (temp_pointers == NULL)
  {
    printf("error happened");
    exit(1);
  }
  temp_keys[insertion_index] = key;

  leaf->num_keys = 0;

  if ((bTreeOrder - 1) % 2 != 0)
  {
    split = (bTreeOrder - 1) / 2 + 1;
  }
  else
  {
    split = (bTreeOrder - 1) / 2;
  }

  for (i = 0; i < split; ++i)
  {
    leaf->num_keys++;
    leaf->keys[i] = temp_keys[i];
    leaf->pointers[i] = temp_pointers[i];
  }

  for (i = split, j = 0; i < bTreeOrder; ++i, ++j)
  {
    new_leaf->num_keys = new_leaf->num_keys + 1;
    new_leaf->keys[j] = temp_keys[i];
    new_leaf->pointers[j] = temp_pointers[i];
  }

  free(temp_keys);
  free(temp_pointers);

  new_leaf->pointers[bTreeOrder - 1] = leaf->pointers[bTreeOrder - 1];
  if (new_leaf->pointers == NULL)
  {
    printf("error happened");
    exit(1);
  }
  leaf->pointers[bTreeOrder - 1] = new_leaf;

  for (i = leaf->num_keys; i < bTreeOrder - 1; ++i)
  {
    leaf->pointers[i] = NULL;
  }
  for (i = new_leaf->num_keys; i < bTreeOrder - 1; ++i)
  {
    new_leaf->pointers[i] = NULL;
  }

  treeManager->numEntries = treeManager->numEntries + 1;
  int *new_key = new_leaf->keys[0];
  new_leaf->parent = leaf->parent;
  return insertIntoParent(treeManager, leaf, new_key, new_leaf);
}

Node *findLeaf(Node *root, Value *key)
{
  int i = 0;
  Node *c = root;
  if (c != NULL)
  {
  }
  else
  {
    return c;
  }
  while (!c->is_leaf)
  {
    i = 0;
    while (i < c->num_keys)
    {
      bool isG = isGreater(key, c->keys[i]);
      bool isE = isEqual(key, c->keys[i]);
      if (isE || isG)
      {
        i = i + 1;
      }
      else
      {
        break;
      }
    }
    c = (Node *)c->pointers[i];
  }
  return c;
}

Node *createNode(IndexManager *treeManager)
{
  if (treeManager == NULL)
  {
    printf("treeManager is NULL.\n");
    return NULL; // Return NULL if treeManager is not provided
  }

  // Increment the number of nodes in the tree manager
  treeManager->numNodes++;
  if (treeManager == NULL)
  {
    printf("error happened");
    exit(1);
  }
  int bTreeOrder = treeManager->order;

  // Allocate memory for the new node
  Node *new_node = (Node *)malloc(sizeof(Node));
  if (new_node == NULL)
  {
    printf("Memory allocation for new node failed.\n");
    return NULL; // Return NULL if allocation fails
  }

  // Set initial values for the new node
  new_node->parent = NULL;
  new_node->next = NULL;
  new_node->is_leaf = false;
  new_node->num_keys = 0;

  // Allocate memory for keys inside the node
  new_node->keys = (Value **)malloc((bTreeOrder - 1) * sizeof(Value *));
  if (new_node->keys == NULL)
  {
    printf("Memory allocation for new node keys failed.\n");
    free(new_node); // Free the node before returning
    return NULL;
  }

  // Allocate memory for pointers inside the node
  new_node->pointers = (void **)malloc(bTreeOrder * sizeof(void *));
  if (new_node->pointers == NULL)
  {
    printf("Memory allocation for new node pointers failed.\n");
    free(new_node->keys); // Free allocated keys before returning
    free(new_node);       // Free the node before returning
    return NULL;
  }

  // Return the newly created node with allocated keys and pointers
  return new_node;
}
// Creates a new leaf by creating a node.
Node *createLeaf(IndexManager *treeManager)
{
  Node *leaf = createNode(treeManager);
  if (leaf == NULL)
  {
    return RC_ERROR;
  }
  leaf->is_leaf = true;
  return leaf;
}

Node *insertIntoNode(IndexManager *treeManager, Node *parent, int left_index, Value *key, Node *right)
{
  if (parent == NULL)
  {
    printf("error happened");
    exit(1);
  }
  parent->num_keys++;

  for (int i = parent->num_keys; left_index < i; --i)
  {
    parent->keys[i] = parent->keys[i - 1];
    parent->pointers[i + 1] = parent->pointers[i];
  }

  parent->keys[left_index] = key;
  parent->pointers[left_index + 1] = right;

  return treeManager->root;
}
int getNeighborIndex(Node *n)
{

  int i;

  for (i = 0; n->parent->num_keys >= i; i++)
  {
    if (n->parent->pointers[i] != n)
    {
    }
    else
    {
      return i - 1;
    }
  }

  printf("Error happened");
  return RC_ERROR;
}

NodeData *findRecord(Node *root, Value *key)
{
  Node *c = findLeaf(root, key);
  if (c == NULL)
  {
    printf("c is NULL");
    return NULL;
  }
  int i = 0;
  for (i = 0; i < c->num_keys; ++i)
  {
    if (isEqual(c->keys[i], key))
    {
      break;
    }
  }
  if (i != c->num_keys)
  {
    return (NodeData *)c->pointers[i];
  }
  else
  {
    return NULL;
  }
}

Node *deleteNodeEntry(IndexManager *treeManager, Node *n, Value *key, Node *pointer)
{
  int i, num_pointers;
  if (treeManager == NULL)
  {
    printf("error happened");
    exit(1);
  }
  int bTreeOrder = treeManager->order;
  if (bTreeOrder == NULL)
  {
    printf("error happened");
    exit(1);
  }
  i = 0;

  while (!isEqual(n->keys[i], key))
  {
    if (i < 0)
    {
      return RC_ERROR;
    }
    i++;
  }

  for (++i; i < n->num_keys; i++)
  {
    if (i < 0)
    {
      return RC_ERROR;
    }
    n->keys[i - 1] = n->keys[i];
  }

  if (n->is_leaf)
  {
    num_pointers = n->num_keys;
  }
  else
  {
    num_pointers = n->num_keys + 1;
  }

  i = 0;
  while (pointer != n->pointers[i])
  {
    i = i + 1;
  }
  for (++i; i < num_pointers; i++)
  {
    if (i < 0)
    {
      return RC_ERROR;
    }
    n->pointers[i - 1] = n->pointers[i];
  }

  n->num_keys = n->num_keys - 1;
  treeManager->numEntries = treeManager->numEntries - 1;

  if (!n->is_leaf)
  {
    for (int j = n->num_keys + 1; j < bTreeOrder; ++j)
    {
      n->pointers[j] = NULL;
    }
  }
  else
  {
    for (int j = n->num_keys; j < bTreeOrder - 1; ++j)
    {
      n->pointers[j] = NULL;
    }
  }

  return n;
}

Node *adjustNode(Node *root)
{

  Node *new_root;

  if (root == NULL || root->num_keys > 0)
  {
    return root;
  }

  if (!root->is_leaf)
  {
    new_root = root->pointers[0];
    if (root == NULL)
    {
      return new_root;
    }
    new_root->parent = NULL;
  }
  else
  {
    new_root = NULL;
  }

  free(root->pointers);
  if (root->keys != NULL)
  {
    free(root->keys);
  }
  free(root);

  return new_root;
}
Node *mergeNodes(IndexManager *treeManager, Node *n, Node *neighbor, int neighbor_index, int *k_p)
{

  int i, j, neighbor_insertion_index, n_end;
  if (treeManager == NULL)
  {
    printf("error happened");
    exit(1);
  }
  int bTreeOrder = treeManager->order;

  Node *tmp;
  if (neighbor_index == -1)
  {
    tmp = n;
    n = neighbor;
    neighbor = tmp;
  }

  neighbor_insertion_index = neighbor->num_keys;

  if (!n->is_leaf)
  {
    if (n == NULL)
    {
      return RC_ERROR;
    }
    neighbor->num_keys++;

    if (neighbor->keys == NULL)
    {
      printf("error happened");
      exit(1);
    }
    neighbor->keys[neighbor_insertion_index] = &k_p;

    n_end = n->num_keys;
    if (n_end < 0)
    {
      printf("error happened");
      exit(1);
    }

    for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++)
    {
      neighbor->keys[i] = n->keys[j];

      if (n->pointers != NULL)
      {
        neighbor->pointers[i] = n->pointers[j];
      }
      neighbor->num_keys++;
      n->num_keys--;
    }

    neighbor->pointers[i] = n->pointers[j];

    for (int j = 0; j < neighbor->num_keys + 1; j++)
    {
      tmp->parent = neighbor;
      if (neighbor == NULL)
      {
        printf("error happened");
        exit(1);
      }
      tmp = (Node *)neighbor->pointers[j];
    }
  }
  else
  {
    if (n == NULL)
    {
      return RC_ERROR;
    }
    for (i = neighbor_insertion_index, j = 0; j < n->num_keys; i++, j++)
    {
      neighbor->num_keys++;
      if (n->keys != NULL)
      {
        neighbor->keys[i] = n->keys[j];
      }
      neighbor->pointers[i] = n->pointers[j];
    }
    neighbor->pointers[bTreeOrder - 1] = n->pointers[bTreeOrder - 1];
  }

  if (treeManager != NULL)
  {
    treeManager->root = deleteEntry(treeManager, n->parent, k_p, n);
  }

  free(n->keys);
  if (n->pointers != NULL)
  {
    free(n->pointers);
  }
  free(n);
  return treeManager->root;
}

Node *delete(IndexManager *treeManager, Value *key)
{
  if (treeManager != NULL)
  {
    NodeData *key_leaf;
    if (findLeaf != NULL)
    {
      key_leaf = findLeaf(treeManager->root, key);
    }
    Node *record;
    if (findRecord != NULL)
    {
      record = findRecord(treeManager->root, key);
    }
    if (record != NULL && key_leaf != NULL)
    {
      if (record == NULL || key_leaf == NULL)
      {
        return RC_ERROR;
      }
      treeManager->root = deleteEntry(treeManager, key_leaf, key, record);
      free(record);
    }
    return treeManager->root;
  }
  return RC_ERROR;
}

Node *redistributeNodes(Node *root, Node *n, Node *neighbor, int neighbor_index, int k_p_index, int k_p)
{
  int i;
  Node *tmp;

  if (neighbor_index == -1)
  {

    if (!n->is_leaf)
    {
      n->keys[n->num_keys] = k_p;
      n->pointers[n->num_keys + 1] = neighbor->pointers[0];
      if (n->pointers == NULL)
      {
        printf("error happened");
        exit(1);
      }
      tmp = (Node *)n->pointers[n->num_keys + 1];
      if (neighbor == NULL)
      {
        printf("error happened");
        exit(1);
      }
      n->parent->keys[k_p_index] = neighbor->keys[0];
      tmp->parent = n;
    }
    else
    {
      n->keys[n->num_keys] = neighbor->keys[0];
      if (n->pointers == NULL)
      {
        printf("error happened");
        exit(1);
      }
      n->pointers[n->num_keys] = neighbor->pointers[0];
      if (n->parent == NULL)
      {
        printf("error happened");
        exit(1);
      }
      n->parent->keys[k_p_index] = neighbor->keys[1];
    }
    for (i = 0; i < neighbor->num_keys - 1; ++i)
    {
      if (neighbor == NULL)
      {
        printf("error happened");
        exit(1);
      }
      neighbor->pointers[i] = neighbor->pointers[i + 1];
      if (neighbor->keys == NULL)
      {
        printf("error happened");
        exit(1);
      }
      neighbor->keys[i] = neighbor->keys[i + 1];
    }
    if (!n->is_leaf)
    {
      if (neighbor->pointers == NULL)
      {
        printf("error happened");
        exit(1);
      }
      neighbor->pointers[i] = neighbor->pointers[i + 1];
    }
  }
  else
  {
    if (!n->is_leaf)
    {
      n->pointers[n->num_keys + 1] = n->pointers[n->num_keys];
      if (n->pointers == NULL)
      {
        printf("error happened");
        exit(1);
      }
    }
    for (i = n->num_keys; i > 0; i--)
    {
      if (n->pointers == NULL)
      {
        printf("error happened");
        exit(1);
      }
      n->pointers[i] = n->pointers[i - 1];
      n->keys[i] = n->keys[i - 1];
    }
    if (n->is_leaf)
    {

      n->pointers[0] = neighbor->pointers[neighbor->num_keys - 1];
      if (n->pointers == NULL)
      {
        printf("error happened");
        exit(1);
      }
      neighbor->pointers[neighbor->num_keys - 1] = NULL;
      if (neighbor->pointers[neighbor->num_keys - 1] != NULL)
      {
        return RC_ERROR;
      }
      n->keys[0] = neighbor->keys[neighbor->num_keys - 1];
      n->parent->keys[k_p_index] = n->keys[0];
    }
    else
    {
      n->pointers[0] = neighbor->pointers[neighbor->num_keys];
      if (n->pointers == NULL)
      {
        printf("error happened");
        exit(1);
      }
      tmp = (Node *)n->pointers[0];
      tmp->parent = n;
      if (neighbor == NULL)
      {
        printf("error happened");
        exit(1);
      }
      neighbor->pointers[neighbor->num_keys] = NULL;
      n->keys[0] = k_p;
      if (n->keys == NULL)
      {
        printf("error happened");
        exit(1);
      }
      n->parent->keys[k_p_index] = neighbor->keys[neighbor->num_keys - 1];
    }
  }

  neighbor->num_keys--;
  n->num_keys++;

  return root;
}

Node *deleteEntry(IndexManager *treeManager, Node *n, Value *key, void *pointer)
{
  int capacity;
  int bTreeOrder = treeManager->order;
  Node *neighbor;
  int k_p_index;
  int neighbor_index;
  int *k_p;
  int min_keys;

  n = deleteNodeEntry(treeManager, n, key, pointer);
  Node *root = treeManager->root;
  if (n == root)
    return adjustNode(root);

  if (n->is_leaf)
  {
    if ((bTreeOrder - 1) % 2 != 0)
    {
      min_keys = (bTreeOrder - 1) / 2 + 1;
    }
    else
    {
      min_keys = (bTreeOrder - 1) / 2;
    }
  }
  else
  {
    if ((bTreeOrder) % 2 != 0)
    {
      min_keys = (bTreeOrder) / 2 + 1;
    }
    else
    {
      min_keys = (bTreeOrder) / 2;
    }
    min_keys = min_keys - 1;
  }

  if (min_keys <= n->num_keys)
  {
    return root;
  }

  neighbor_index = getNeighborIndex(n);
  k_p_index = neighbor_index == -1 ? 0 : neighbor_index;
  k_p = n->parent->keys[k_p_index];
  if (neighbor_index == -1)
  {
    neighbor = n->parent->pointers[1];
  }
  else
  {
    neighbor = n->parent->pointers[neighbor_index];
  }
  if (n->is_leaf)
  {
    capacity = bTreeOrder;
  }
  else
  {
    capacity = bTreeOrder - 1;
  }

  if (capacity > neighbor->num_keys + n->num_keys)
  {
    return mergeNodes(treeManager, n, neighbor, neighbor_index, k_p);
  }
  else
  {
    return redistributeNodes(root, n, neighbor, neighbor_index, k_p_index, k_p);
  }
}
int enqueue(IndexManager *treeManager, Node *new_node)
{
  Node *c;
  if (treeManager->queue == NULL)
  {
    if (!treeManager->queue == NULL)
    {
      printf("ERROR");
      return -1;
    }
    // If the queue is empty, insert the new node as the first node
    treeManager->queue = new_node;
    if (new_node == NULL)
    {
      printf("ERROR");
      return -1;
    }
    // The next of the new node should be NULL as it is the only node in the queue
    treeManager->queue->next = NULL;
  }
  else
  {
    if (treeManager->queue == NULL)
    {
      printf("ERROR");
      return -1;
    }
    // Traverse to the end of the queue to insert the new node
    c = treeManager->queue;
    while (c->next != NULL)
    {
      if (c->next == NULL)
      {
        break;
      }
      c = c->next;
    }

    // Insert the new_node at the end of the queue and set its next to NULL
    c->next = new_node;
    if (new_node == NULL)
    {
      printf("ERROR");
      return -1;
    }
    new_node->next = NULL;
  }
}

// This function assists in removing the first node from the queue during tree traversal
Node *dequeue(IndexManager *treeManager)
{
  // Retrieve the first node from the queue
  Node *n = treeManager->queue;
  if (treeManager->queue == NULL)
  {
    printf("ERROR");
    return -1;
  }
  // Move the head of the queue to the next node
  treeManager->queue = treeManager->queue->next;
  if (treeManager->queue == NULL)
  {
    n->next = treeManager->queue;
  }
  else
  {
    n->next = NULL;
  }
  // Set the next of the retrieved node to NULL before returning it
  return n;
}

// This function calculates the length of the path from any given node to the root of the tree
int path_to_root(Node *root, Node *child)
{
  int length = 0;
  // Initialize the current node as the child
  if (child == NULL)
  {
    return length;
  }
  Node *c = child;
  if (root == NULL)
  {
    return length;
  }
  // Traverse up the tree until the root node is reached
  while (c != root)
  {
    if (c == root)
    {
      break;
    }
    // Move to the parent node
    c = c->parent;
    if (c == root)
    {
      break;
    }
    // Increment the path length
    length++;
  }
  if (length == 0)
  {
    return 0;
  }
  else
  {
    return length;
  }
}

bool intIsGreater(int a, int b)
{
  return a > b;
}

bool intIsEqual(int a, int b)
{
  return a == b;
}

// Helper function for comparing float values.
bool floatIsGreater(float a, float b)
{
  return a > b;
}

bool floatIsEqual(float a, float b)
{
  return a == b;
}

// Helper function for comparing string values.
bool stringIsGreater(const char *a, const char *b)
{
  return strcmp(a, b) > 0;
}

bool stringIsEqual(const char *a, const char *b)
{
  return strcmp(a, b) == 0;
}

// Helper function for comparing boolean values.
bool boolIsEqual(bool a, bool b)
{
  return a == b; // Since there's no 'greater' comparison for bool, only equality is checked.
}
// This function compares two keys and returns TRUE if the first key is greater than the second key, else returns FALSE.
bool isGreater(Value *key1, Value *key2)
{
  switch (key1->dt)
  {
  case DT_INT:
    return intIsGreater(key1->v.intV, key2->v.intV);
  case DT_FLOAT:
    return floatIsGreater(key1->v.floatV, key2->v.floatV);
  case DT_STRING:
    return stringIsGreater(key1->v.stringV, key2->v.stringV);
  case DT_BOOL:
    return FALSE; // Boolean datatype can only be Equal or Not Equal To.
  }
  return FALSE; // Default return FALSE for any non-supported datatype.
}

// This function compares two keys and returns TRUE if the first key is equal to the second key, else returns FALSE.
bool isEqual(Value *key1, Value *key2)
{
  if (key1->dt != key2->dt)
  {
    return FALSE; // Different types cannot be equal.
  }
  switch (key1->dt)
  {
  case DT_INT:
    return intIsEqual(key1->v.intV, key2->v.intV);
  case DT_FLOAT:
    return floatIsEqual(key1->v.floatV, key2->v.floatV);
  case DT_STRING:
    return stringIsEqual(key1->v.stringV, key2->v.stringV);
  case DT_BOOL:
    return boolIsEqual(key1->v.boolV, key2->v.boolV);
  }
  return FALSE; // Default return FALSE for any non-supported datatype.
}

// Helper function for comparing integer values.
bool intIsLess(int a, int b)
{
  return a < b;
}

// Helper function for comparing float values.
bool floatIsLess(float a, float b)
{
  return a < b;
}

// Helper function for comparing string values.
bool stringIsLess(const char *a, const char *b)
{
  return strcmp(a, b) < 0;
}

// This function compares two keys and returns TRUE if first key is less than second key, else returns FALSE.
bool isLess(Value *key1, Value *key2)
{
  switch (key1->dt)
  {
  case DT_INT:
    return intIsLess(key1->v.intV, key2->v.intV);
  case DT_FLOAT:
    return floatIsLess(key1->v.floatV, key2->v.floatV);
  case DT_STRING:
    return stringIsLess(key1->v.stringV, key2->v.stringV);
  case DT_BOOL:
    return FALSE; // Boolean datatype can only be Equal or Not Equal To.
  }
  return FALSE; // Default return FALSE for any non-supported datatype.
}