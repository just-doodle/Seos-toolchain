#include "tree.h"


tree_t* tree_create()
{
    return (tree_t *)kcalloc(sizeof(tree_t), 1);
}

treenode_t *tree_node_create(void *data)
{
    treenode_t *n = (treenode_t*)kcalloc(sizeof(treenode_t), 1);
    n->data = data;
    n->children = list_create();
    return n;
}

treenode_t *tree_insert(tree_t *tree, treenode_t *subroot, void *data)
{
    treenode_t *treenode = (treenode_t*)kcalloc(sizeof(treenode_t), 1);
    treenode->children = list_create();
    treenode->data = data;

    if (!tree->root)
    {
        tree->root = treenode;
        return treenode;
    }
    list_insert_front(subroot->children, treenode);
    return treenode;
}

treenode_t *tree_findParent(tree_t *tree, treenode_t *remove_node, int *child_index)
{
    if (remove_node == tree->root)
        return NULL;
    return tree_findParent_recur(tree, remove_node, tree->root, child_index);
}

treenode_t *tree_findParent_recur(tree_t *tree, treenode_t *remove_node, treenode_t *subroot, int *child_index)
{
    int idx;
    if ((idx = list_contain(subroot->children, remove_node)) != -1)
    {
        *child_index = idx;
        return subroot;
    }
    foreach (child, subroot->children)
    {
        treenode_t *ret = tree_findParent_recur(tree, remove_node, (treenode_t*)child->val, child_index);
        if (ret != NULL)
        {
            return ret;
        }
    }
    return NULL;
}

void tree_remove(tree_t *tree, treenode_t *remove_node)
{
    int child_index = -1;
    treenode_t *parent = tree_findParent(tree, remove_node, &child_index);
    if (parent != NULL)
    {
        treenode_t *freethis = (treenode_t*)list_remove_by_index(parent->children, child_index);
        kfree(freethis);
    }
}

void tree2list_recur(treenode_t *subroot, list_t *list)
{
    if (subroot == NULL)
        return;
    foreach (child, subroot->children)
    {
        treenode_t *curr_treenode = (treenode_t *)child->val;
        void *curr_val = curr_treenode->data;
        list_insert_back(list, curr_val);
        tree2list_recur((treenode_t*)child->val, list);
    }
}

void tree2list(tree_t *tree, list_t *list)
{
    tree2list_recur(tree->root, list);
}

void tree2array(tree_t *tree, void **array, int *size)
{
    tree2array_recur(tree->root, array, size);
}

void tree2array_recur(treenode_t *subroot, void **array, int *size)
{
    if (subroot == NULL)
        return;
    void *curr_val = (void *)subroot->data;
    array[*size] = curr_val;
    *size = *size + 1;
    foreach (child, subroot->children)
    {
        tree2array_recur((treenode_t*)child->val, array, size);
    }
}