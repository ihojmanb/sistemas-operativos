typedef struct node {
  char *k *v;
  struct node *left, right;
} Node;

typedef struct {
  Node *root;
} ConcDict;

void addDef(ConcDict* dict, char *k, char *v){
//&dict->root es un puntero a un puntero: Node**
  insert(&dict->root, k ,v);
}

void insert(Node** ppnode, char*k, char*v){
  if(*ppnode == NULL)
    *ppnode = createLeafNode(k,v);
  else {
    pnode = *ppnode;
    if (strcmp(k, pnode->) < 0)
      insert(&pnode->left, k, v);
    else
      insert(&pnode->right, k, v);
  }
}
