#include <stdio.h>
#include "duktape.h"

#define CLIBS		"$$CLIBS"

duk_ret_t duk_loadlib (duk_context *ctx);

void prepare_duk_env(duk_context *ctx);

int main(int argc, const char *argv[]) {
    duk_context *ctx = NULL;

    ctx = duk_create_heap_default();
    if (!ctx) {
        printf("Failed to create a Duktape heap.\n");
        exit(1);
    }

    prepare_duk_env(ctx);

    if (duk_peval_file(ctx, "main.js") != 0) {
        printf("Error: %s\n", duk_safe_to_string(ctx, -1));
        goto finished;
    }
    duk_pop(ctx);  /* ignore result */

 finished:
    duk_destroy_heap(ctx);

    exit(0);
}

void prepare_duk_env(duk_context *ctx) {
	duk_get_global_string(ctx, "Duktape");
    duk_push_object(ctx); 
  	duk_put_prop_string(ctx, -2, CLIBS);

    duk_push_global_object(ctx);
    duk_push_c_function(ctx, duk_loadlib, 1 /*nargs*/);
    duk_put_prop_string(ctx, -2, "$loadlib");
}