#include <stdlib.h>
#include <stdio.h>
#include "duktape.h"

void prepare_duk_env(duk_context *ctx);

void register_sock(duk_context *ctx);

int main(int argc, const char *argv[]) {
    duk_context *ctx = NULL;
    const char *filename = "main.js";

#ifdef DEBUG
    printf("loading Duktape\n");
#endif

    ctx = duk_create_heap_default();
    if (!ctx) {
        printf("Failed to create a Duktape heap.\n");
        exit(1);
    }

    prepare_duk_env(ctx);

    if (argc > 1) {
        filename = argv[1];
    }

#ifdef DEBUG
    printf("loading file %s\n", argv[1]);
#endif

    if (duk_peval_file(ctx, filename) != 0) {
        printf("!!! %s\n", duk_safe_to_string(ctx, -1));
        goto finished;
    }
    duk_pop(ctx);  /* ignore result */

 finished:
    duk_destroy_heap(ctx);

    return EXIT_SUCCESS;
}


void prepare_duk_env(duk_context *ctx) {
    register_sock(ctx);
}