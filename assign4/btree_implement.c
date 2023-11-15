

int enqueue(BTreeManager *treeManager, Node *new_node)
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
Node *dequeue(BTreeManager *treeManager)
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
  if (child == NULL) {
    return length;
  }
  Node *c = child;
  if (root == NULL) {
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