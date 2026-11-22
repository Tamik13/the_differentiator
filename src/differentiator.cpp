#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "../include/bin_tree.h"
#include "../include/stack.h"

const size_t STR_OP_SIZE = 30;

enum math_hash_e {
    TYPE_PLUS  = 177616,
    TYPE_MINUS = 177618,
    TYPE_MUL   = 177615,
    TYPE_DIV   = 177620,
    TYPE_SIN   = 193505807,
    TYPE_COS   = 193488586
};

error_code_e read_tree          (bin_tree_s* bin_tree, char**  buffer);
error_code_e op_tree_init       (bin_tree_s* bin_tree,     char**  buffer);
error_code_e string_to_node_elem(const char* string,   node_s* node);
const char*  op_hash_to_string  (unsigned long hash);


int main() {
    log_start();

    char* buffer = NULL;
    bin_tree_s bin_tree = {};
    bin_tree_assign(&bin_tree);
    SAFE_CALL( read_tree(&bin_tree, &buffer), );

    log_end();
    return 0;
}


error_code_e read_tree(bin_tree_s* bin_tree, char** buffer) {
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

    const char* data_base_name = "./src/data.txt";

    size_t  size = 0;
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

    ssize_t  data_ind = 1;
    stack_s  parents  = {};
    node_s** node     = &bin_tree->root;
    SAFE_CALL ( bin_tree_insert(bin_tree, node, NULL, NAN, NAT), free((*buffer)); close(data_base))
    STACK_INIT( parents, 2);
    SAFE_CALL ( stack_push(&parents, node), free((*buffer)); close(data_base))


    for (ssize_t ind = 1; ind < count_read; ind++) {

        if ((*buffer)[ind] == '(') {
            $ANCHOR;

            (*buffer)[ind] = '\0';

            if (isnan((*node)->data)) {
                $ANCHOR;
                string_to_node_elem(&(*buffer)[data_ind], *node);
            }

            $ANCHOR;
            data_ind = ind + 1;
            SAFE_CALL( stack_push(&parents, node), free((*buffer)); close(data_base))

            if ((*node)->left == NULL) {
                $ANCHOR;
                SAFE_CALL( bin_tree_insert(bin_tree, &(*node)->left, *node, NAN, NAT), free((*buffer)); close(data_base))
                node = &(*node)->left;

            } else if ((*node)->right == NULL) {
                SAFE_CALL( bin_tree_insert(bin_tree, &(*node)->right, *node, NAN, NAT), free((*buffer)); close(data_base))
                node = &(*node)->right;
                $ANCHOR;

            } else {
                LOG_ERROR("tread son");
                $ANCHOR;
                return INCORRECT_PARAM;
            }

        } else if ((*buffer)[ind] == ')') {

            (*buffer)[ind] = '\0';
            $ANCHOR;

            if (isnan((*node)->data)) {
                $ANCHOR;
                string_to_node_elem(&(*buffer)[data_ind], *node);
            }

            data_ind = ind + 1;
            $ANCHOR;
            SAFE_CALL( stack_pop(&parents, &node), free((*buffer)); close(data_base))

        }
    }

    if (close(data_base) == -1) {
        LOG_ERROR("error during close data_base");
        return ERROR_DURING_CLOSE;
    }

    bin_tree_dmp(bin_tree);

    LOG_OK("akinator_read successfully");
    return SUCCESS;
}


error_code_e string_to_node_elem(const char* string, node_s* node) {
    assert(string != NULL);

    unsigned long hash = djb2_str_hash((unsigned char*)string);
    $(string)

    if (sscanf(string, "%lg", &node->data) == 1) {
        $(node->data)
        node->type = TYPE_CONST;
        return SUCCESS;

    } else if (op_hash_to_string(hash) != NULL) {
        node->data = hash;
        $(node->data)
        node->type = TYPE_OP;
        return SUCCESS;

    } else {
        node->data = hash;
        $(node->data)
        node->type = TYPE_VAR;
        return SUCCESS;
    }
}


const char* op_hash_to_string(unsigned long hash) {

    switch (hash) {

    case TYPE_PLUS:
        return "+";

    case TYPE_MINUS:
        return "-";

    case TYPE_MUL:
        return "*";

    case TYPE_DIV:
        return "/";

    case TYPE_SIN:
        return "sin";

    case TYPE_COS:
        return "cos";

    default:
        return NULL;
    }

}

