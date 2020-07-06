/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef MPL_GAVL_H_INCLUDED
#define MPL_GAVL_H_INCLUDED

typedef void *MPL_gavl_tree_t;

enum {
    SEARCH_LEFT,
    SEARCH_RIGHT,
    BUFFER_MATCH,
    NO_BUFFER_MATCH
};

typedef struct gavl_tree_node {
    struct gavl_tree_node *parent;
    struct gavl_tree_node *left;
    struct gavl_tree_node *right;
    uintptr_t height;
    uintptr_t addr;
    uintptr_t len;
    const void *val;
} gavl_tree_node_s;

typedef struct gavl_tree {
    gavl_tree_node_s *root;
    void (*gavl_free_fn) (void *);
} gavl_tree_s;

int MPL_gavl_tree_create(void (*free_fn) (void *), MPL_gavl_tree_t * gavl_tree);
int MPL_gavl_tree_insert(MPL_gavl_tree_t gavl_tree, const void *addr, uintptr_t len,
                         const void *val);
int MPL_gavl_tree_free(MPL_gavl_tree_t gavl_tree);
int MPL_gavl_tree_delete(MPL_gavl_tree_t gavl_tree, const void *addr, uintptr_t len);
int MPL_gavl_tree_delete_base(MPL_gavl_tree_t gavl_tree, const void *addr);

MPL_STATIC_INLINE_PREFIX int gavl_subset_cmp_func(uintptr_t ustart, uintptr_t len,
                                                  gavl_tree_node_s * tnode)
{
    int cmp_ret;
    uintptr_t uend = ustart + len;
    uintptr_t tstart = tnode->addr;
    uintptr_t tend = tnode->addr + tnode->len;

    if (tstart <= ustart && uend <= tend)
        cmp_ret = BUFFER_MATCH;
    else if (ustart < tstart)
        cmp_ret = SEARCH_LEFT;
    else
        cmp_ret = SEARCH_RIGHT;

    return cmp_ret;
}

MPL_STATIC_INLINE_PREFIX int MPL_gavl_tree_search(MPL_gavl_tree_t gavl_tree, const void *addr,
                                                  uintptr_t len, void **val)
{
    int mpl_err = MPL_SUCCESS;
    gavl_tree_node_s *cur_node;
    gavl_tree_s *gavl_tree_iptr = (gavl_tree_s *) gavl_tree;

    *val = NULL;
    cur_node = gavl_tree_iptr->root;
    while (cur_node) {
        int cmp_ret = gavl_subset_cmp_func((uintptr_t) addr, len, cur_node);
        if (cmp_ret == BUFFER_MATCH) {
            *val = (void *) cur_node->val;
            break;
        } else if (cmp_ret == SEARCH_LEFT) {
            cur_node = cur_node->left;
        } else {
            cur_node = cur_node->right;
        }
    }

    return mpl_err;
}

#endif /* MPL_GAVL_H_INCLUDED  */
