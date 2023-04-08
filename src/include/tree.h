#ifndef __GENERAL_TREE_H__
#define __GENERAL_TREE_H__

#include "system.h"
#include "list.h"
#include "kheap.h"

typedef struct gentree_node
{
    list_t *children;
    void *data;
}treenode_t;

typedef struct gentree
{
    treenode_t *root;
}tree_t;

tree_t* tree_create();

treenode_t* tree_node_create(void *data);
treenode_t* tree_insert(tree_t *tree, treenode_t *subroot, void *data);
treenode_t* tree_findParent(tree_t *tree, treenode_t *remove_node, int* child_index);
treenode_t* tree_findParent_recur(tree_t *tree, treenode_t *remove_node, treenode_t *subroot, int *child_index);

void tree_remove(tree_t *tree, treenode_t *remove_node);
void tree2list_recur(treenode_t *subroot, list_t *list);
void tree2list(tree_t *tree, list_t *list);
void tree2array(tree_t *tree, void **array, int *size);
void tree2array_recur(treenode_t *subroot, void **array, int *size);

#endif /*__GENERAL_TREE_H__*/