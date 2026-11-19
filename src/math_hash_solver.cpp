#include "../include/debugging.h"

int main() {

    const char* math_elems[] = {
        "+",
        "-",
        "*",
        "/",
        "sin",
        "cos"
    };

    for (size_t ind = 0; ind < sizeof(math_elems) / sizeof(math_elems[0]); ind++) {
        printf("%s_hash = %lu\n", math_elems[ind], djb2_str_hash((const unsigned char*)math_elems[ind]));
    }

    return 0;
}
