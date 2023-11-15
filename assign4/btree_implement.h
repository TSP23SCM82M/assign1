#ifndef BTREE_IMPLEMENT_H
#define BTREE_IMPLEMENT_H

#include "btree_mgr.h"
#include "buffer_mgr.h"

// Functions to support printing of the B+ Tree
int enqueue(BTreeManager *treeManager, Node *new_node);
Node *dequeue(BTreeManager *treeManager);
int path_to_root(Node *root, Node *child);
