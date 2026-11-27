#include <assert.h>
#include <string.h>
#include <math.h>

#include "../include/differentiator.h"

                       //  hash    new_op_node  diff_op     tex_dump       number
op_info op_info_arr[] = {{177615,    op_mul,   diff_mul,   tex_dump_mul,   OP_MUL},
                         {177616,    op_plus,  diff_plus,  tex_dump_plus,  OP_PLUS},
                         {177618,    op_minus, diff_minus, tex_dump_minus, OP_MINUS},
                         {177620,    op_div,   diff_div,   tex_dump_div,   OP_DIV},
                         {193488586, op_cos,   diff_cos,   tex_dump_cos,   OP_COS},
                         {193505807, op_sin,   diff_sin,   tex_dump_sin,   OP_SIN}};

const char* const tex_file_name   = "./logs/tex/log.tex";
      FILE*       GLOBAL_TEX_FILE = NULL;



node_s* diff(node_s* node) {
    assert(node != NULL);

    switch (node->type) {

        case TYPE_CONST:
            return new_node(TYPE_CONST, 0, NULL, NULL);

        case TYPE_VAR:
            return new_node(TYPE_CONST, 1, NULL, NULL);

        case TYPE_OP:
            return op_info_arr[(size_t)(node->data)].diff_op(node);

        case NAT:
        default:
            LOG_ERROR("Incorrect param");
            return NULL;
    }

    return NULL;
}


void init_node(node_s* node, elem_type_e type, double value, node_s* left, node_s* right) {
    assert(node != NULL);

    node->data  = value;
    node->left  = left;
    node->right = right;
    node->type  = type;

    return;
}


node_s* new_node(elem_type_e type, double value, node_s* left, node_s* right) {

    node_s* node = (node_s*)calloc(1, sizeof(node_s));
    if (node == NULL) {
        LOG_ERROR("allocation error in calloc new node");
        return NULL;
    }

    init_node(node, type, value, left, right);

    return node;
}


node_s* copy_node(node_s* node) {
    assert(node != NULL);

    node_s* node_copy = (node_s*)calloc(1, sizeof(node_s));
    if (node == NULL) {
        LOG_ERROR("allocation error in calloc node copy");
        return NULL;
    }

    init_node(node_copy, node->type, node->data, node->left, node->right);

    return node_copy;
}


node_s* copy_branch(node_s* node) {

    if (node == NULL) {
        return NULL;
    }

    node_s* node_copy = copy_node  (node);
    node_copy->left   = copy_branch(node->left);
    node_copy->right  = copy_branch(node->right);

    return node_copy;
}


error_code_e branch_simplify(bin_tree_s* bin_tree, node_s* node) {
    assert(node != NULL);

    if (node->left != NULL) {
        SAFE_CALL( branch_simplify(bin_tree, node->left), )
    }

    if (node->right != NULL) {
        SAFE_CALL( branch_simplify(bin_tree, node->right), )
    }

    if (node->left == NULL || node->right == NULL) {
        return SUCCESS;
    }

    SAFE_CALL( conv_consts   (bin_tree, node), )
    SAFE_CALL( plus_simplify (bin_tree, node), )
    SAFE_CALL( minus_simplify(bin_tree, node), )
    SAFE_CALL( mul_simplify  (bin_tree, node), )

    bin_tree_dmp(bin_tree);
    LOG_OK("brunch simplify successfully");
    return SUCCESS;
}


double node_solve(bin_tree_s* bin_tree, node_s* node) {
    assert(node        != NULL);
    assert(node->left  != NULL);
    assert(node->right != NULL);



    switch ((op_type_e)node->data) {
            case OP_PLUS:
                return node->left->data + node->right->data;

            case OP_MINUS:
                return node->left->data - node->right->data;

            case OP_MUL:
                return node->left->data * node->right->data;

            case OP_DIV:
                return node->left->data / node->right->data;

            case OP_SIN:
                return sin(node->left->data);

            case OP_COS:
                return cos(node->left->data);

            default:
                LOG_ERROR("Incorrect param");
        }

    bin_tree_dmp(bin_tree);
    return NAN;
}


error_code_e conv_consts(bin_tree_s* bin_tree, node_s* node) {
    assert(node     != NULL);
    assert(bin_tree != NULL);

    if (node->left->type == TYPE_CONST && node->right->type == TYPE_CONST) {
        assert(node->type == TYPE_OP);

        double new_data = node_solve(bin_tree, node);
        if (isnan(new_data)) {
            LOG_ERROR("cant solve node");
            return INCORRECT_PARAM;
        }

        node->data  = new_data;
        node->type  = TYPE_CONST;

        bin_tree_del_node(bin_tree, &node->left);
        bin_tree_del_node(bin_tree, &node->right);

    }

    return SUCCESS;
}

error_code_e plus_simplify (bin_tree_s* bin_tree, node_s* node) {
    assert(node     != NULL);
    assert(bin_tree != NULL);

    if (node->type != TYPE_OP || node->data != OP_PLUS) {
        return SUCCESS;
    }

    if (node->left->data == 0  && node->left->type == TYPE_CONST) {

        init_node(node, node->right->type, node->right->data, node->right->left, node->right->right);
        bin_tree_del_node(bin_tree, &node->left);
        bin_tree_del_node(bin_tree, &node->right);

    } else if (node->right->data == 0 && node->right->type  == TYPE_CONST) {

        init_node(node, node->left->type, node->left->data, node->left->left, node->left->right);
        bin_tree_del_node(bin_tree, &node->left);
        bin_tree_del_node(bin_tree, &node->right);

    }

    return SUCCESS;
}


error_code_e minus_simplify(bin_tree_s* bin_tree, node_s* node) {
    assert(node     != NULL);
    assert(bin_tree != NULL);

    if (node->type != TYPE_OP || node->data != OP_MINUS) {
        return SUCCESS;
    }

    if (node->left->data == 0  && node->left->type == TYPE_CONST) {

        init_node(node, node->right->type, -node->right->data, node->right->left, node->right->right);
        bin_tree_del_node(bin_tree, &node->left);
        bin_tree_del_node(bin_tree, &node->right);

    } else if (node->right->data == 0 && node->right->type  == TYPE_CONST) {

        init_node(node, node->left->type, node->left->data, node->left->left, node->left->right);
        bin_tree_del_node(bin_tree, &node->left);
        bin_tree_del_node(bin_tree, &node->right);

    }

    return SUCCESS;
}


error_code_e mul_simplify  (bin_tree_s* bin_tree, node_s* node) {
    assert(bin_tree != NULL);
    assert(node     != NULL);

    if (node->type != TYPE_OP || node->data != OP_MUL) {
        return SUCCESS;
    }

    if ((node->left->data  == 0 && node->left->type  == TYPE_CONST) ||
        (node->right->data == 0 && node->right->type == TYPE_CONST)) {

        node->data = 0;
        node->type = TYPE_CONST;
        bin_tree_del_node(bin_tree, &node->left);
        bin_tree_del_node(bin_tree, &node->right);
        node->right = NULL;
        node->left  = NULL;

    } else if (node->left->data == 1 && node->left->type  == TYPE_CONST) {

        init_node(node, node->right->type, node->right->data, node->right->left, node->right->right);
        bin_tree_del_node(bin_tree, &node->left);
        bin_tree_del_node(bin_tree, &node->right);

    } else if (node->right->data == 1 && node->right->type  == TYPE_CONST) {

        init_node(node, node->left->type, node->left->data, node->left->left, node->left->right);
        bin_tree_del_node(bin_tree, &node->left);
        bin_tree_del_node(bin_tree, &node->right);

    }

    return SUCCESS;
}


node_s* op_plus(node_s* first_node, node_s* second_node) {
    assert(first_node != NULL); assert(second_node != NULL);
    return new_node(TYPE_OP, OP_PLUS, first_node, second_node);
}


node_s* op_minus(node_s* first_node, node_s* second_node) {
    assert(first_node != NULL); assert(second_node != NULL);
    return new_node(TYPE_OP, OP_MINUS, first_node, second_node);
}


node_s* op_mul(node_s* first_node, node_s* second_node) {
    assert(first_node != NULL); assert(second_node != NULL);
    return new_node(TYPE_OP, OP_MUL, first_node, second_node);
}


node_s* op_div(node_s* first_node, node_s* second_node) {
    assert(first_node != NULL); assert(second_node != NULL);
    return new_node(TYPE_OP, OP_DIV, first_node, second_node);
}


node_s* op_sin(node_s* first_node, node_s* second_node) {
    assert(first_node != NULL);
    return new_node(TYPE_OP, OP_SIN, first_node, NULL);
}


node_s* op_cos(node_s* first_node, node_s* second_node) {
    assert(first_node != NULL);
    return new_node(TYPE_OP, OP_COS, first_node, NULL);
}


node_s* diff_plus(node_s* node) {
    assert(node != NULL);

    return op_plus(diff(node->left), diff(node->right));
}


node_s* diff_minus(node_s* node) {
    assert(node != NULL);

    return op_minus(diff(node->left), diff(node->right));
}


node_s* diff_mul(node_s* node) {
    assert(node != NULL);

    node_s* diff_left  = diff(node->left);
    node_s* diff_right = diff(node->right);
    node_s* copy_left  = copy_branch(node->left);
    node_s* copy_right = copy_branch(node->right);

    return op_plus(op_mul(diff_left, copy_right), op_mul(diff_right, copy_left));
}


node_s* diff_div(node_s* node) {

    node_s* diff_left  = diff(node->left);
    node_s* diff_right = diff(node->right);
    node_s* copy_left  = copy_branch(node->left);
    node_s* copy_right = copy_branch(node->right);

    return op_div(op_minus(op_mul(diff_left, copy_right), op_mul(copy_left, diff_right)), op_mul(copy_right, copy_right));
}


node_s* diff_sin(node_s* node) {
    assert(node != NULL);

    return op_mul(op_cos(copy_branch(node->left), NULL), diff(node->left));
}


node_s* diff_cos(node_s* node) {
    assert(node != NULL);

    return op_mul(diff(node->left), op_mul(new_node(TYPE_CONST, -1, NULL, NULL), op_sin(copy_branch(node->left), NULL)));
}


error_code_e open_tex() {
    GLOBAL_TEX_FILE = fopen(tex_file_name, "w");

    if (GLOBAL_TEX_FILE == NULL) {
        LOG_ERROR("cant open tex file");
        return ERROR_DURING_OPEN;
    }

    fprintf(GLOBAL_TEX_FILE,
            "\\documentclass[12pt]{article}\n"
            "\\usepackage[T2A]{fontenc}\n"
            "\\usepackage[russian]{babel}\n"
            "\\begin{document}\n");

    return SUCCESS;
}



void tex_dump_node(node_s* node) {

    switch (node->type) {
        case TYPE_CONST:
            tex_dump_const(node);
            return;

        case TYPE_VAR:
            tex_dump_var(node);
            return;

        case TYPE_OP:
            tex_dump_op(node);
            return;

        case NAT:
        default:
            LOG_ERROR("incorrect param");
    }
}


void tex_dump_new_formula(node_s* node) {
    assert(node != NULL);

    fprintf(GLOBAL_TEX_FILE, "\n\\[\n");
    tex_dump_node(node);
    fprintf(GLOBAL_TEX_FILE, "\n\\]\n");

    return;
}


error_code_e close_tex() {
    fprintf(GLOBAL_TEX_FILE,
            "\\end{document}");

    if (fclose(GLOBAL_TEX_FILE) == EOF) {
        system("pdflatex -output-directory=/Users/tamerlannagaev/prodva/THE_DIFFERENTIATOR/logs/tex /Users/tamerlannagaev/prodva/THE_DIFFERENTIATOR/logs/tex/log.tex");
        LOG_ERROR("cant close tex file");
        return ERROR_DURING_CLOSE;
    }

    system("pdflatex -output-directory=/Users/tamerlannagaev/prodva/THE_DIFFERENTIATOR/logs/tex /Users/tamerlannagaev/prodva/THE_DIFFERENTIATOR/logs/tex/log.tex");

    return SUCCESS;
}


void tex_dump_const(node_s* node) {
    assert(node != NULL);

    if (node->data >= 0) fprintf(GLOBAL_TEX_FILE, "%lg",   node->data);
    else                 fprintf(GLOBAL_TEX_FILE, "(%lg)", node->data);

    return;
}


void tex_dump_var(node_s* node) {
    assert(node != NULL);

    fprintf(GLOBAL_TEX_FILE, "x");

    return;
}


void tex_dump_op(node_s* node) {
    assert(node != NULL);

    op_info_arr[(size_t)node->data].tex_dump(node);
}


void tex_dump_plus(node_s* node) {
    assert(node != NULL);

    fprintf(GLOBAL_TEX_FILE, "(");

    if (node->left != NULL) {
        tex_dump_node(node->left);
    } else {
        LOG_ERROR("plus dont have left son");
        return;
    }

    fprintf(GLOBAL_TEX_FILE, " + ");

    if (node->right != NULL) {
        tex_dump_node(node->right);
    } else {
        LOG_ERROR("plus dont have right son");
        return;
    }

    fprintf(GLOBAL_TEX_FILE, ")");

    return;
}


void tex_dump_minus(node_s* node) {
    assert(node != NULL);

    fprintf(GLOBAL_TEX_FILE, "(");

    if (node->left != NULL) {
        tex_dump_node(node->left);
    } else {
        LOG_ERROR("minus dont have left son");
        return;
    }

    fprintf(GLOBAL_TEX_FILE, " - ");

    if (node->right != NULL) {
        tex_dump_node(node->right);
    } else {
        LOG_ERROR("minus dont have right son");
        return;
    }

    fprintf(GLOBAL_TEX_FILE, ")");

    return;
}


void tex_dump_mul(node_s* node) {
    assert(node != NULL);

    if (node->left != NULL) {
        tex_dump_node(node->left);
    } else {
        LOG_ERROR("mul dont have left son");
        return;
    }

    if (node->right != NULL) {
        tex_dump_node(node->right);
    } else {
        LOG_ERROR("mul dont have right son");
        return;
    }

    return;
}


void tex_dump_div(node_s* node) {
    assert(node != NULL);

    fprintf(GLOBAL_TEX_FILE, "(frac{");

    if (node->left != NULL) {
        tex_dump_node(node->left);
    } else {
        LOG_ERROR("div dont have left son");
        return;
    }

    fprintf(GLOBAL_TEX_FILE, "}{");

    if (node->right != NULL) {
        tex_dump_node(node->right);
    } else {
        LOG_ERROR("div dont have right son");
        return;
    }

    fprintf(GLOBAL_TEX_FILE, "})");

    return;
}


void tex_dump_cos(node_s* node) {
    assert(node != NULL);

    fprintf(GLOBAL_TEX_FILE, "\\cos{(");

    if (node->left != NULL) {
        tex_dump_node(node->left);
    } else {
        LOG_ERROR("cos dont have left son");
        return;
    }

    fprintf(GLOBAL_TEX_FILE, ")}");

    return;
}


void tex_dump_sin(node_s* node) {
    assert(node != NULL);

    fprintf(GLOBAL_TEX_FILE, "\\sin{(");

    if (node->left != NULL) {
        tex_dump_node(node->left);
    } else {
        LOG_ERROR("sin dont have left son");
        return;
    }

    fprintf(GLOBAL_TEX_FILE, ")}");

    return;
}


ssize_t op_hash_bin_psk(unsigned long hash) {
    ssize_t l = 0, r = sizeof(op_info_arr) / sizeof(op_info_arr[0]) - 1;

    while (l < r) {
        assert(l <= r);
        ssize_t m = (l + r) / 2;

        if (op_info_arr[m].hash == hash) {
            return (size_t)op_info_arr[m].number;
        } else if (op_info_arr[m].hash < hash) {
            l = m + 1;
        } else {
            r = m - 1;
        }
    }

    if (op_info_arr[l].hash == hash) {
        return (size_t)op_info_arr[l].number;
    }

    return -1;
}

