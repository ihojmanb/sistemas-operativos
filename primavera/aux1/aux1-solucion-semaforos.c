typedef struct node {
  char *k *v;
  struct node *left, right;
  nSem semLeft, semRight;
} Node;

typedef struct {
  Node *root;
  nSem semRoot;
} ConcDict;

void addDef(ConcDict* dict, char *k, char *v){
//&dict->root es un puntero a un puntero: Node**
  insert(&dict->semRoot, &dict->root, k ,v);
}

Node *createLeafNode(char *k, char*v){
  ...
  leafNode
  ...
}

void insert(nSem sem, Node** ppnode, char*k, char*v){
  Node *ńode;
  nWaitSem(sem);
  if(*ppnode == NULL)
    *ppnode = createLeafNode(k,v);
    nSignalSem(sem);
  else {
    nSignalSem(sem); /*soltamos el semaforo*/
    pnode = *ppnode;
    if (strcmp(k, pnode->) < 0)
      insert(&pnode->left, k, v);
    else
      insert(&pnode->right, k, v);
  }
}
