#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "../include/bin_tree.h"
#include "../include/stack.h"


error_code_e bin_tree_assign (bin_tree_s* const bin_tree) {
    assert(bin_tree != NULL);

    bin_tree->count_nodes = 0;
    bin_tree->root        = NULL;

    LOG_OK("bin_tree_assign successfully");
    return SUCCESS;
}


error_code_e bin_tree_destroy(bin_tree_s* const bin_tree) {
    assert(bin_tree != NULL);

    if (bin_tree->root == NULL) {
        LOG_OK("bin_tree_dfs successfully");
        return SUCCESS;
    }

    ssize_t count_nodes = bin_tree->count_nodes;
    ON_DBG(ssize_t  cnt = 0;)
    node_s** node       = NULL;
    stack_s  stack      = {};
    STACK_INIT(stack, count_nodes);
    SAFE_CALL( stack_push(&stack, &bin_tree->root), stack_destroy(&stack));

    while (stack.size > 0) {
        ON_DBG(
        if (cnt++ > count_nodes * 3) {
            stack_destroy(&stack);
            LOG_ERROR("infinite cycle");
            return INCORRECT_SIZE;
        }
        )

        SAFE_CALL( stack_pop(&stack, &node), stack_destroy(&stack));

        if ((*node)->left != NULL && (*node)->right != NULL) {

            SAFE_CALL( stack_push(&stack, node),            stack_destroy(&stack))
            SAFE_CALL( stack_push(&stack, &(*node)->left),  stack_destroy(&stack))
            SAFE_CALL( stack_push(&stack, &(*node)->right), stack_destroy(&stack))

        } else if ((*node)->left != NULL) {

            SAFE_CALL( stack_push(&stack, node),            stack_destroy(&stack))
            SAFE_CALL( stack_push(&stack, &(*node)->left),  stack_destroy(&stack))

        } else if ((*node)->right != NULL) {

            SAFE_CALL( stack_push(&stack, node),            stack_destroy(&stack))
            SAFE_CALL( stack_push(&stack, &(*node)->right), stack_destroy(&stack))

        } else {
            SAFE_CALL( bin_tree_del_node(bin_tree, node), )
        }
    }

    SAFE_CALL(stack_destroy(&stack), );
    LOG_OK("bin_tree_destroy successfully");
    return SUCCESS;
}


error_code_e bin_tree_insert(bin_tree_s* const bin_tree, node_s** const node, bin_tree_elem elem, elem_type_e elem_type) {
    assert(bin_tree != NULL);
    assert(node != NULL);

    SAFE_CALL( bin_tree_verify(bin_tree), );

    *node = (node_s*)calloc(1, sizeof(node_s));

    if (*node == NULL) {
        LOG_ERROR("cant calloc *node");
        return ALLOCATION_ERROR;
    }
    (*node)->type   = elem_type,
    (*node)->left   = NULL;
    (*node)->right  = NULL;
    (*node)->data   = elem;
    bin_tree->count_nodes++;

    LOG_OK("bin_tree_insert successfully elem = " BNT_MDF, elem);
    return SUCCESS;
}


error_code_e bin_tree_del_node(bin_tree_s* const bin_tree, node_s** const node) {
    assert(node != NULL);
    assert(bin_tree->count_nodes > 0);

    free(*node);

    LOG_OK("bin_tree_del successfully");
    return SUCCESS;
}


error_code_e bin_tree_verify(bin_tree_s* const bin_tree) {
    assert(bin_tree != NULL);

    ON_DBG( bin_tree_dmp(bin_tree);)

    return SUCCESS;
}


error_code_e bin_tree_print_preorder(bin_tree_s* const bin_tree, char** buffer) {
    assert(bin_tree != NULL);
    assert(buffer   != NULL);

    if (bin_tree->count_nodes == 0) {
        *buffer = (char*)calloc(3, sizeof(char));

        if (*buffer == NULL) {
            LOG_ERROR("allocation error");
            LOG_OK("stack_push successfully");
            return ALLOCATION_ERROR;
        }

        (*buffer)[0] = '(';
        (*buffer)[1] = ')';
        (*buffer)[2] = '\0';

        return SUCCESS;
    }

    size_t data_len    = 100;
    size_t count_nodes = (size_t)bin_tree->count_nodes;
    *buffer             = (char*)calloc((size_t)(count_nodes + 1) * data_len, sizeof(char));

    if (*buffer == NULL) {
        LOG_ERROR("allocation error");
        LOG_OK("stack_push successfully");
        return ALLOCATION_ERROR;
    }

    size_t count_write = 0;
    ON_DBG(ssize_t cnt  = 0;)
    node_s**   node     = NULL;
    stack_s    stack    = {};
    STACK_INIT(stack, bin_tree->count_nodes);
    SAFE_CALL( stack_push(&stack, &bin_tree->root), stack_destroy(&stack));

    node_s  void_node     = {};
    node_s* void_node_ptr = &void_node;

    while (stack.size > 0) {
        ON_DBG(
        if (cnt++ > bin_tree->count_nodes * 3) {
            LOG_ERROR("infinite cycle");
            return INCORRECT_SIZE;
        }
        )

        SAFE_CALL( stack_pop(&stack, &node), );

        if (node != &void_node_ptr) {
            count_write += (size_t)snprintf(*buffer + count_write, count_nodes * data_len - count_write, "(" BNT_MDF, (*node)->data);
        }

        if ((*node)->left != NULL && (*node)->right != NULL) {

            SAFE_CALL( stack_push(&stack, &void_node_ptr),  stack_destroy(&stack))
            SAFE_CALL( stack_push(&stack, &(*node)->right), stack_destroy(&stack))
            SAFE_CALL( stack_push(&stack, &(*node)->left),  stack_destroy(&stack))

        } else if ((*node)->left != NULL) {

            SAFE_CALL( stack_push(&stack, &void_node_ptr),  stack_destroy(&stack))
            SAFE_CALL( stack_push(&stack, &(*node)->left),  stack_destroy(&stack))

        } else if ((*node)->right != NULL) {

            SAFE_CALL( stack_push(&stack, &void_node_ptr),  stack_destroy(&stack))
            SAFE_CALL( stack_push(&stack, &(*node)->right), stack_destroy(&stack))

        } else {
            count_write += (size_t)snprintf(*buffer + count_write, count_nodes * data_len - count_write, ")");
        }
    }

    SAFE_CALL(stack_destroy(&stack), );

    LOG_OK("stack_postorder successfully");
    return SUCCESS;
}


void bin_tree_dmp(bin_tree_s* const bin_tree) {
    assert(bin_tree != NULL);

    char* buffer;
    bin_tree_print_preorder(bin_tree, &buffer);

    error_code_e error_code = INIT_VALUE;
    if (bin_tree->count_nodes > 0) {
        error_code = bin_tree_img(bin_tree);
    }

    LOG("bin_tree %p {",               bin_tree);
    LOG("bin_tree->count_nodes = %zd", bin_tree->count_nodes);
    LOG("PREORDER: %s", buffer);
    LOG("}");

    if (error_code == INIT_VALUE) {
        free(buffer);
        LOG_OK("void bin_tree");
        return;

    } if (error_code != SUCCESS) {
        free(buffer);
        LOG_ERROR("cant generate bin_tree img");
        return;
    }

    free(buffer);
    buffer = NULL;

    size_t size = 0;
    if (stat_get_size("./logs/dot/graph.svg", &size) != SUCCESS) {
        LOG_ERROR("STAT ERROR errno: %s", strerror(errno));
        return;
    }

    buffer = (char*)calloc(size, sizeof(char));

    int dot_file = open("./logs/dot/graph.svg", O_RDONLY);

    if (dot_file == -1) {
        LOG_ERROR("cant open dot file");
        return;
    }

    ssize_t count_read = 0;

    while ((count_read = read(dot_file, buffer, size)) > 0) {
        ssize_t new_ind = 0;

        for (ssize_t true_ind = 0; true_ind < count_read; true_ind++) {

            if (buffer[true_ind] != '\n') {
                buffer[new_ind] = buffer[true_ind];
                new_ind++;
            }
            if (true_ind >= 4 && strncmp(&buffer[true_ind - 4], "/svg>", 5) == 0) {
                break;

            }
        }

        fwrite(buffer, 1, (size_t)new_ind,  GLOBAL_LOG_FILE);
    }

    fprintf(GLOBAL_LOG_FILE, "\n");

    if (close(dot_file) == -1) {
        LOG_ERROR("CANT CLOSE dot_file");
        return;
    }

    free(buffer);
    LOG_OK("bin_tree_dmp successfully");
    return;
}


error_code_e bin_tree_img(bin_tree_s* const bin_tree) {
    assert(bin_tree != NULL);

    const char* const file_name = "./logs/dot/graph.dot";

    FILE* dot_file = fopen(file_name, "w");
    if (dot_file == NULL) {
        LOG_ERROR("cant open dot file");
        return ERROR_DURING_OPEN;
    }

    if (bin_tree->count_nodes == 0) {
        LOG_OK("void bin_tree cant be convert in img");
        return INCORRECT_SIZE;
    }

    fprintf(dot_file,   "digraph {\n"
                            "\tbgcolor=\"" BGCOLOR_HX "\";\n"
                            "\tnodesep=0.6\n"
                            "\trank_sep=1.0\n"
                            );

    ON_DBG(    ssize_t cnt         = 0;)
               node_s**   node     = NULL;
               stack_s    stack    = {};
               STACK_INIT(stack, bin_tree->count_nodes);
    SAFE_CALL( stack_push(&stack, &bin_tree->root), stack_destroy(&stack));

    while (stack.size > 0) {
        ON_DBG(
        if (cnt++ > bin_tree->count_nodes * 3) {
            LOG_ERROR("infinite cycle");
            return INCORRECT_SIZE;
        }
        )

        SAFE_CALL( stack_pop(&stack, &node), stack_destroy(&stack));
        dot_init_node(dot_file, *node);

        if ((*node)->left != NULL && (*node)->right != NULL) {

            dot_init_edge(dot_file, *node, (*node)->left,  GREEN_HX);
            dot_init_edge(dot_file, *node, (*node)->right, RED_HX);

            SAFE_CALL( stack_push(&stack, &(*node)->right), stack_destroy(&stack))
            SAFE_CALL( stack_push(&stack, &(*node)->left),  stack_destroy(&stack))

        } else if ((*node)->left != NULL) {

            dot_init_edge(dot_file, *node, (*node)->left,  GREEN_HX);
            SAFE_CALL( stack_push(&stack, &(*node)->left),  stack_destroy(&stack))

        } else if ((*node)->right != NULL) {

            dot_init_edge(dot_file, *node, (*node)->right,  RED_HX);
            SAFE_CALL( stack_push(&stack, &(*node)->right), stack_destroy(&stack))

        }
    }

    fprintf(dot_file, "}");

    if (fclose(dot_file) == EOF) {
        LOG_ERROR("cant close dot file");
        return ERROR_DURING_CLOSE;
    }

    SAFE_CALL(stack_destroy(&stack), );

    int status = system("dot -Tsvg ./logs/dot/graph.dot -o ./logs/dot/graph.svg 2>&1 "
                    "| grep -v -E 'Warning:.*(non-ASCII|libgvplugin_pango)' >&2");

    if (status == -1) {
        LOG_ERROR("status of system() = -1");
        return SYSTEM_ERROR;
    }

    LOG_OK("graph graphic dmp successfully");
    return SUCCESS;
}


void dot_init_node(FILE* const dot_file, const node_s* const node) {
    assert(dot_file != NULL);
    assert(node     != NULL);

    if      (node->type == TYPE_OP)    fprintf(dot_file, "\tnode%p [style=filled; shape=Mrecord;  fillcolor=\"" AQUA_HX "\"; label= \" data = %c | type = %s\"]\n", (void*)node, math_hash_to_char(node->data), elem_type_to_string(node->type));
    else if (node->type == TYPE_CONST) fprintf(dot_file, "\tnode%p [style=filled; shape=Mrecord;  fillcolor=\"" RED_HX "\"; label= \" data = %lg | type = %s\"]\n", (void*)node, node->data,                    elem_type_to_string(node->type));
    else                               fprintf(dot_file, "\tnode%p [style=filled; shape=Mrecord;  fillcolor=\"" GREEN_HX "\"; label= \" data = x | type = %s\"]\n", (void*)node,                                elem_type_to_string(node->type));

    return;
}


void dot_init_edge(FILE* const dot_file, const node_s* const first_node, const node_s* const second_node, const char* const color) {
    assert(dot_file   != NULL);
    assert(first_node != NULL);

    fprintf(dot_file, "\tnode%p -> node%p [color=\"%s\"]\n", (void*)first_node, (void*)second_node, color);

    return;
}


char math_hash_to_char(unsigned long hash) {

    switch (hash) {

    case OP_PLUS:
        return '+';

    case OP_MINUS:
        return '-';

    case OP_MUL:
        return '*';

    case OP_DIV:
        return '/';

    case OP_SIN:
        return 's';

    case OP_COS:
        return 'c';

    default:
        return '\0';
    }
}


const char* elem_type_to_string(elem_type_e elem_type) {

    switch (elem_type) {

        case TYPE_OP:
            return "operation";

        case TYPE_VAR:
            return "var";

        case TYPE_CONST:
            return "const";

        case NAT:
        default:
            LOG_ERROR("Incorrect param");
            return NULL;
    }
}


