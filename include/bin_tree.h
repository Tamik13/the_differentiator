#ifndef _BIN_TREE
#define _BIN_TREE

#include "../include/debugging.h"

typedef double bin_tree_elem;
#define BNT_MDF "%lg"

enum elem_type_e {
    TYPE_OP   = 1,
    TYPE_VAR  = 2,
    TYPE_COST = 3
};

struct node_s {
    elem_type_e   type;
    bin_tree_elem data;
    node_s*       left;
    node_s*       right;
    node_s*       parent;
};

struct bin_tree_s {
    node_s* root;
    ssize_t count_nodes;
};

error_code_e bin_tree_assign (bin_tree_s* const bin_tree);
error_code_e bin_tree_destroy(bin_tree_s* const bin_tree);
error_code_e bin_tree_insert (bin_tree_s* const bin_tree, node_s** const node, node_s* parent, bin_tree_elem elem, elem_type_e elem_type);
error_code_e bin_tree_del    (bin_tree_s* const bin_tree, node_s** const node);
error_code_e bin_tree_verify (bin_tree_s* const bin_tree);

void bin_tree_dmp                    (bin_tree_s* const bin_tree);
error_code_e bin_tree_img            (bin_tree_s* const bin_tree);
error_code_e bin_tree_print_preorder(bin_tree_s* const bin_tree, char** buffer);

#endif
