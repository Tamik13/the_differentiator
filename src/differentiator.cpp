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


int main() {
    log_start();

    char* buffer = NULL;
    bin_tree_s bin_tree = {};
    bin_tree_assign(&bin_tree);
    SAFE_CALL( op_tree_read(&bin_tree, &buffer), );

    bin_tree_s diff_tree = {};

    diff_tree.root = diff(bin_tree.root);
    assert(diff_tree.root != NULL);

    diff_tree.count_nodes = 5;
    bin_tree_dmp(&diff_tree);

    log_end();
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

    } else {
        node->data = hash;
        node->type = TYPE_VAR;
        return SUCCESS;
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
                    return new_node(TYPE_OP, TYPE_PLUS, diff(node->left), diff(node->right));

                case TYPE_MINUS:
                    return new_node(TYPE_OP, TYPE_MINUS, diff(node->left), diff(node->right));

                case TYPE_MUL: {
                    node_s* diff_left  = diff(node->left);
                    node_s* diff_right = diff(node->right);
                    node_s* copy_left  = copy_node(node->left);
                    node_s* copy_right = copy_node(node->right);
                    return new_node(TYPE_OP, TYPE_PLUS,
                                    new_node(TYPE_OP, TYPE_MUL, diff_left, copy_right),
                                    new_node(TYPE_OP, TYPE_MUL, diff_right, copy_left));
                }

                case TYPE_DIV:
                case TYPE_SIN:
                case TYPE_COS:
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

