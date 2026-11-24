#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "../include/bin_tree.h"
#include "../include/stack.h"

const size_t STR_OP_SIZE = 30;

error_code_e op_tree_read       (      bin_tree_s*         bin_tree, char**  buffer);
error_code_e op_tree_init       (      bin_tree_s*         bin_tree, char**  buffer);
error_code_e string_to_node_elem(const char*               string,   node_s* node);
ssize_t      read_to_buffer     (const char*         const file,     char*   buffer);
node_s*      diff               (      node_s*             node);
node_s*      new_node           (      elem_type_e         type,     double  value,        node_s* left, node_s* right);
node_s*      copy_node          (      node_s*             node);
// TODO: op_tree_init

node_s* op_plus (node_s* first_node, node_s* second_node);
node_s* op_minus(node_s* first_node, node_s* second_node);
node_s* op_mul  (node_s* first_node, node_s* second_node);
node_s* op_div  (node_s* first_node, node_s* second_node);
node_s* op_sin  (node_s* node);
node_s* op_cos  (node_s* node);

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

const char* const tex_file_name   = "./logs/tex/log.tex";
      FILE*       GLOBAL_TEX_FILE = NULL;

int main() {
    SAFE_CALL( log_start(), )
    SAFE_CALL( open_tex(),  )


    char* buffer        = NULL;
    bin_tree_s bin_tree = {};
    bin_tree_assign(&bin_tree);
    SAFE_CALL( op_tree_read(&bin_tree, &buffer), );

    bin_tree_s diff_tree = {};

    diff_tree.root = diff(bin_tree.root);
    assert(diff_tree.root != NULL);

    diff_tree.count_nodes = 5;
    bin_tree_dmp(&diff_tree);

    tex_dump_new_formula(bin_tree.root);
    tex_dump_new_formula(diff_tree.root);

    log_end();
    close_tex();
    return 0;
}


error_code_e op_tree_read(bin_tree_s* bin_tree, char** buffer) {
    assert(bin_tree != NULL);

    const char* data_base_name = "./src/data.txt";

    size_t  size       = 0;
    ssize_t count_read = -1;
    SAFE_CALL( stat_get_size(data_base_name, &size), )

    (*buffer) = (char*)calloc(size, sizeof(char));
    if ((*buffer) == NULL) {
        LOG_ERROR("error during buffer allocation");
        return ALLOCATION_ERROR;
    }

    int data_base = open(data_base_name, O_RDONLY);

    if (data_base == -1) {
        free((*buffer));
        LOG_ERROR("cant open data_base");
        return ERROR_DURING_OPEN;
    }

    if ((count_read = read(data_base, (*buffer), size - 1)) == -1) {
        LOG_ERROR("cant read data_base");
        return ERROR_DURING_READ;
    }

    assert(count_read > 0 && (size_t)count_read < size);

    SAFE_CALL( op_tree_init(bin_tree, buffer), );

    if (close(data_base) == -1) {
        LOG_ERROR("error during close data_base");
        return ERROR_DURING_CLOSE;
    }

    bin_tree_dmp(bin_tree);

    LOG_OK("akinator_read successfully");
    return SUCCESS;
}


error_code_e op_tree_init(bin_tree_s* bin_tree, char** buffer) {
    assert(bin_tree != NULL);

    ssize_t count_read = read_to_buffer("./src/data.txt", *buffer);

    if (count_read == -1) {
        LOG_ERROR("error during read data.txt");
        return ERROR_DURING_READ;
    }

    ssize_t  data_ind = 1;
    stack_s  parents  = {};
    node_s** node     = &bin_tree->root;
    SAFE_CALL ( bin_tree_insert(bin_tree, node, NAN, NAT), free((*buffer)))
    STACK_INIT( parents, 2);
    SAFE_CALL ( stack_push(&parents, node), free((*buffer)))

    for (ssize_t ind = 1; ind < count_read; ind++) {

        if ((*buffer)[ind] == '(') {

            (*buffer)[ind] = '\0';

            if (isnan((*node)->data)) {
                string_to_node_elem(&(*buffer)[data_ind], *node);
            }

            data_ind = ind + 1;
            SAFE_CALL( stack_push(&parents, node), free((*buffer)))

            if ((*node)->left == NULL) {
                SAFE_CALL( bin_tree_insert(bin_tree, &(*node)->left, NAN, NAT), free((*buffer)))
                node = &(*node)->left;

            } else if ((*node)->right == NULL) {
                SAFE_CALL( bin_tree_insert(bin_tree, &(*node)->right, NAN, NAT), free((*buffer)))
                node = &(*node)->right;

            } else {
                LOG_ERROR("tread son");
                return INCORRECT_PARAM;
            }

        } else if ((*buffer)[ind] == ')') {

            (*buffer)[ind] = '\0';

            if (isnan((*node)->data)) {
                string_to_node_elem(&(*buffer)[data_ind], *node);
            }

            data_ind = ind + 1;
            SAFE_CALL( stack_pop(&parents, &node), free((*buffer)))

        }
    }

    bin_tree_dmp(bin_tree);

    LOG_OK("akinator_read successfully");
    return SUCCESS;
}


ssize_t read_to_buffer(const char* const file_name, char* buffer) {
    assert(file_name != NULL);
    assert(buffer    != NULL);

    size_t  size = 0;
    ssize_t count_read = -1;
    SAFE_CALL( stat_get_size(file_name, &size), )

    buffer = (char*)calloc(size, sizeof(char));
    if (buffer == NULL) {
        LOG_ERROR("error during buffer allocation");
        return ALLOCATION_ERROR;
    }

    int file = open(file_name, O_RDONLY);

    if (file == -1) {
        free(buffer);
        LOG_ERROR("cant open data_base");
        return -1;
    }

    if ((count_read = read(file, buffer, size - 1)) == -1) {
        LOG_ERROR("cant read data_base");
        return -1;
    }

    if (count_read <= 0 && (size_t)count_read > size) {
        LOG_ERROR("count_read > size or count_read <= 0");
        close(file);
        return -1;
    }

    if (close(file) == -1) {
        LOG_ERROR("error during close data_base");
        return ERROR_DURING_CLOSE;
    }

    return count_read;
}


error_code_e string_to_node_elem(const char* string, node_s* node) {
    assert(string != NULL);

    unsigned long hash = djb2_str_hash((const unsigned char*)string);

    if (sscanf(string, "%lg", &node->data) == 1) {
        node->type = TYPE_CONST;
        return SUCCESS;

    } else if (math_hash_to_char(hash) != '\0') {
        node->data = hash;
        node->type = TYPE_OP;
        return SUCCESS;

    } else if (strcmp(string, "x") == 0) {
        node->data = 'x';
        node->type = TYPE_VAR;
        return SUCCESS;
    } else {
        return INCORRECT_PARAM;
    }
}


node_s* diff(node_s* node) {
    assert(node != NULL);

    switch (node->type) {

        case TYPE_CONST:
            return new_node(TYPE_CONST, 0, NULL, NULL);

        case TYPE_VAR:
            return new_node(TYPE_CONST, 1, NULL, NULL);

        case TYPE_OP:

            switch ((math_hash_e)(node->data)) {

                case TYPE_PLUS:
                    return diff_plus(node);

                case TYPE_MINUS:
                    return diff_minus(node);

                case TYPE_MUL:
                    return diff_mul(node);

                case TYPE_DIV:
                    return diff_div(node);

                case TYPE_SIN:
                    return diff_sin(node);

                case TYPE_COS:
                    return diff_cos(node);

                default:
                    LOG_ERROR("incorrect param: node->data");
                    return NULL;

            }

        case NAT:
        default:
            LOG_ERROR("incorrect type: node->type");
            return NULL;

    }

    return NULL;
}


node_s* new_node(elem_type_e type, double value, node_s* left, node_s* right) {

    node_s* node = (node_s*)calloc(1, sizeof(node_s));
    if (node == NULL) {
        LOG_ERROR("allocation error in calloc new node");
        return NULL;
    }

    node->data  = value;
    node->left  = left;
    node->right = right;
    node->type  = type;

    return node;
}


node_s* copy_node(node_s* node) {
    assert(node != NULL);

    node_s* node_copy = (node_s*)calloc(1, sizeof(node_s));
    if (node == NULL) {
        LOG_ERROR("allocation error in calloc node copy");
        return NULL;
    }

    node_copy->data  = node->data;
    node_copy->left  = node->left;
    node_copy->right = node->right;
    node_copy->type  = node->type;

    return node_copy;
}


node_s* op_plus(node_s* first_node, node_s* second_node) {
    assert(first_node != NULL); assert(second_node != NULL);
    return new_node(TYPE_OP, TYPE_PLUS, first_node, second_node);
}


node_s* op_minus(node_s* first_node, node_s* second_node) {
    assert(first_node != NULL); assert(second_node != NULL);
    return new_node(TYPE_OP, TYPE_MINUS, first_node, second_node);
}


node_s* op_mul(node_s* first_node, node_s* second_node) {
    assert(first_node != NULL); assert(second_node != NULL);
    return new_node(TYPE_OP, TYPE_MUL, first_node, second_node);
}


node_s* op_div(node_s* first_node, node_s* second_node) {
    assert(first_node != NULL); assert(second_node != NULL);
    return new_node(TYPE_OP, TYPE_DIV, first_node, second_node);
}


node_s* op_sin(node_s* node) {
    assert(node != NULL);
    return new_node(TYPE_OP, TYPE_SIN, node, NULL);
}


node_s* op_cos(node_s* node) {
    assert(node != NULL);
    return new_node(TYPE_OP, TYPE_COS, node, NULL);
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
    node_s* copy_left  = copy_node(node->left);
    node_s* copy_right = copy_node(node->right);

    return op_plus(op_mul(diff_left, copy_right), op_mul(diff_right, copy_left));
}


node_s* diff_div(node_s* node) {

    node_s* diff_left  = diff(node->left);
    node_s* diff_right = diff(node->right);
    node_s* copy_left  = copy_node(node->left);
    node_s* copy_right = copy_node(node->right);

    return op_div(op_minus(op_mul(diff_left, copy_right), op_mul(copy_left, diff_right)), op_mul(copy_right, copy_right));
}


node_s* diff_sin(node_s* node) {
    assert(node != NULL);

    return op_cos(copy_node(node->left));
}


node_s* diff_cos(node_s* node) {
    assert(node != NULL);

    return op_mul(new_node(TYPE_CONST, -1, NULL, NULL), op_sin(copy_node(node->left)));
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

    switch ((math_hash_e)node->data) {

        case TYPE_PLUS:
            tex_dump_plus(node);
            return;

        case TYPE_MINUS:
            tex_dump_minus(node);
            return;

        case TYPE_MUL:
            tex_dump_mul(node);
            return;

        case TYPE_DIV:
            tex_dump_div(node);
            return;

        case TYPE_COS:
            tex_dump_cos(node);
            return;

        case TYPE_SIN:
            tex_dump_sin(node);
            return;

        default:
            LOG_ERROR("Incorrect param");
            return;
    }
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

    fprintf(GLOBAL_TEX_FILE, "(");

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

    fprintf(GLOBAL_TEX_FILE, ")");

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

