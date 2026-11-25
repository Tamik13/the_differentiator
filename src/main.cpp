#include <assert.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>

#include "../include/differentiator.h"
#include "../include/stack.h"

error_code_e op_tree_init       (      bin_tree_s*         bin_tree, char**  buffer);
error_code_e string_to_node_elem(const char*               string,   node_s* node);
ssize_t      read_to_buffer     (const char*         const file,     char**  buffer);
// TODO: op_tree_destroy

int main() {
    SAFE_CALL( log_start(), )
    SAFE_CALL( open_tex(),  )


    char* buffer        = NULL;
    bin_tree_s bin_tree = {};
    bin_tree_assign(&bin_tree);
    SAFE_CALL( op_tree_init(&bin_tree, &buffer), );

    bin_tree_s diff_tree = {};

    diff_tree.root = diff(bin_tree.root);
    assert(diff_tree.root != NULL);

    diff_tree.count_nodes = 5;
    bin_tree_dmp(&diff_tree);

    tex_dump_new_formula(bin_tree.root);

    SAFE_CALL( branch_simplify(&bin_tree, bin_tree.root), )

    tex_dump_new_formula(bin_tree.root);
    tex_dump_new_formula(diff_tree.root);

    SAFE_CALL( branch_simplify(&diff_tree, diff_tree.root), )

    tex_dump_new_formula(diff_tree.root);

    log_end();
    close_tex();
    return 0;
}


error_code_e op_tree_init(bin_tree_s* bin_tree, char** buffer) {
    ssize_t count_read = read_to_buffer("./src/data.txt", buffer);

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

    LOG_OK("op_tree_read successfully");
    return SUCCESS;
}


ssize_t read_to_buffer(const char* const file_name, char** buffer) {
    assert(file_name != NULL);

    size_t  size = 0;
    ssize_t count_read = -1;
    SAFE_CALL( stat_get_size(file_name, &size), )

    *buffer = (char*)calloc(size, sizeof(char));
    if (buffer == NULL) {
        LOG_ERROR("error during buffer allocation");
        return ALLOCATION_ERROR;
    }

    int file = open(file_name, O_RDONLY);

    if (file == -1) {
        free(*buffer);
        LOG_ERROR("cant open data_base");
        return -1;
    }

    if ((count_read = read(file, *buffer, size - 1)) == -1) {
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

    unsigned long hash   = djb2_str_hash((const unsigned char*)string);
    ssize_t       number = op_hash_bin_psk(hash);

    if (number != -1) {
        node->data = number;
        node->type = TYPE_OP;
        return SUCCESS;

    } else if (sscanf(string, "%lg", &node->data) == 1) {
        node->type = TYPE_CONST;
        return SUCCESS;

    } else if (strcmp(string, "x") == 0) {  //TODO: ARR WITH VARS
        node->data = 'x';
        node->type = TYPE_VAR;
        return SUCCESS;

    } else {
        return INCORRECT_PARAM;
    }
}




