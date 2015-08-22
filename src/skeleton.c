#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "duktape.h"

duk_ret_t pmain (duk_context *ctx); /* duktape main function */

int main(int argc, char const *argv[]) {
	duk_context *ctx = NULL;

	ctx = duk_create_heap_default();
    if (!ctx) {
        printf("Failed to create a Duktape heap.\n");
        exit(1);
    }

    duk_push_c_function(ctx, pmain, 2);
	duk_push_int(ctx, argc);
	duk_push_pointer(ctx, argv);
	if (duk_pcall(ctx, 2) != DUK_EXEC_SUCCESS) {
		printf("%s: %s\n", argv[0], duk_to_string(ctx, -1));
		exit(1);
	}

    duk_destroy_heap(ctx);

	return 0;
}

static void duk_push_argv (duk_context *ctx, int argc, char *argv[]) {
	duk_idx_t arg_array_index; int i;

	arg_array_index = duk_push_array(ctx);

	for (i = 0; i < argc; i++) {
		duk_push_string(ctx, argv[i]);
		duk_put_prop_index(ctx, arg_array_index, i);
	}

	duk_put_global_string(ctx, "argv");
}

static duk_ret_t pmain (duk_context *ctx) {

	/* register argv in program */
	int argc; char** argv;
	argc = duk_require_int(ctx, 0);
	argv = duk_require_pointer(ctx, 1);
	srduk_push_argv(ctx, argc, argv);

	// program logic here

	return 0;
}