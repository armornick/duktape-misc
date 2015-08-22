#include <stdio.h>
#include "duktape.h"

void register_mod_search(duk_context *ctx);
void register_dukio(duk_context *ctx);

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

/* 
    testing embedding javascript functions in C (also see embed-js-module.c)
*/
duk_ret_t empty_object(duk_context *ctx) {
    static const char *_empty_impl_src = "(function () { " \
        "return function (o) { for (var k in o) { delete o[k] } } " \
        "})()";

    if (duk_is_object(ctx, 0)) {

        duk_eval_string(ctx, _empty_impl_src);
        if (!duk_is_function(ctx, -1)) {
            duk_error(ctx, DUK_ERR_EVAL_ERROR, "could not create helper function");
        }

        duk_dup(ctx, 0);
        duk_call(ctx, 1);
    }
    
    return 0;
}

void prepare_duk_env(duk_context *ctx) {
    register_mod_search(ctx);

    duk_push_global_object(ctx);
    duk_push_c_function(ctx, empty_object, 1 /*nargs*/);
    duk_put_prop_string(ctx, -2, "empty");

    register_dukio(ctx);
}