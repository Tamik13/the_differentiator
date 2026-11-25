#include "../include/bin_tree.h"

typedef node_s*(new_op_node_t)(node_s* first_node, node_s* second_node);
typedef node_s*(diff_op_t)    (node_s* node);
typedef void   (tex_dump_op_t)(node_s* node);

struct op_info {
    unsigned long  hash;
    new_op_node_t* new_op_node;
    diff_op_t*     diff_op;
    tex_dump_op_t* tex_dump;
    ssize_t        number;
};

ssize_t op_hash_bin_psk(unsigned long hash);

node_s* new_node (elem_type_e type, double value, node_s* left, node_s* right);
node_s* copy_node(node_s*     node);

node_s* copy_branch(node_s* node);

node_s*      diff               (      node_s*             node);
error_code_e branch_simplify    (      bin_tree_s*         bin_tree, node_s* node);
double       node_solve         (      bin_tree_s*         bin_tree, node_s* node);

node_s* op_plus (node_s* first_node, node_s* second_node);
node_s* op_minus(node_s* first_node, node_s* second_node);
node_s* op_mul  (node_s* first_node, node_s* second_node);
node_s* op_div  (node_s* first_node, node_s* second_node);
node_s* op_sin  (node_s* first_node, node_s* second_node);
node_s* op_cos  (node_s* first_node, node_s* second_node);

node_s* diff_plus (node_s* node);
node_s* diff_minus(node_s* node);
node_s* diff_mul  (node_s* node);
node_s* diff_div  (node_s* node);
node_s* diff_sin  (node_s* node);
node_s* diff_cos  (node_s* node);

error_code_e open_tex            ();
void         tex_dump_node       (node_s* node);
void         tex_dump_new_formula(node_s* node);
error_code_e close_tex           ();

void tex_dump_const(node_s* node);
void tex_dump_var  (node_s* node);
void tex_dump_op   (node_s* node);

void tex_dump_plus (node_s* node);
void tex_dump_minus(node_s* node);
void tex_dump_mul  (node_s* node);
void tex_dump_div  (node_s* node);
void tex_dump_cos  (node_s* node);
void tex_dump_sin  (node_s* node);
