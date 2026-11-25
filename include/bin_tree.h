#ifndef _BIN_TREE
#define _BIN_TREE

#include "../include/debugging.h"

typedef double bin_tree_elem;
#define BNT_MDF "%lg"

enum elem_type_e {
    TYPE_OP    = 1,
    TYPE_VAR   = 2,
    TYPE_CONST = 3,
    NAT        = 0  // Not a type
};

enum op_type_e {
    OP_MUL   = 0,
    OP_PLUS  = 1,
    OP_MINUS = 2,
    OP_DIV   = 3,
    OP_COS   = 4,
    OP_SIN   = 5
};

struct node_s {
    elem_type_e   type;
    bin_tree_elem data;
    node_s*       left;
    node_s*       right;
};

struct bin_tree_s {
    node_s* root;
    ssize_t count_nodes;
};

error_code_e bin_tree_assign  (bin_tree_s* const bin_tree);
error_code_e bin_tree_destroy (bin_tree_s* const bin_tree);
error_code_e bin_tree_insert  (bin_tree_s* const bin_tree, node_s** const node, bin_tree_elem elem, elem_type_e elem_type);
error_code_e bin_tree_del_node(bin_tree_s* const bin_tree, node_s** const node);
error_code_e bin_tree_verify  (bin_tree_s* const bin_tree);

void         bin_tree_dmp           (bin_tree_s* const bin_tree);
error_code_e bin_tree_img           (bin_tree_s* const bin_tree);
void         dot_init_node          (FILE*       const dot_file, const node_s* const node);
void         dot_init_edge          (FILE*       const dot_file, const node_s* const first_node, const node_s* const second_node, const char* const color);
error_code_e bin_tree_print_preorder(bin_tree_s* const bin_tree, char**  buffer);

char        math_hash_to_char  (unsigned long hash);
const char* elem_type_to_string(elem_type_e   elem_type);

#endif
