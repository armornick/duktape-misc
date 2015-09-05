#include "duknode.h"

/*
------------------------------------------------------------------------------------
*/

static void prepare_duk_env(duk_context *ctx);
static void duknode_push_argv(duk_context *ctx, int argc, const char *argv[]);

/*
------------------------------------------------------------------------------------
*/

static duk_ret_t pmain (duk_context *ctx) {
	int argc; const char** argv;
	argc = duk_require_int(ctx, 0);
	argv = duk_require_pointer(ctx, 1);

	prepare_duk_env(ctx);
    duknode_push_argv(ctx, argc, argv);

    if (duk_peval_file(ctx, "node-main.js") != 0) {
        duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "%s\n", duk_safe_to_string(ctx, -1));
        return -1;
    }
    duk_pop(ctx);  /* ignore result */

    return 0;
}


int main(int argc, const char *argv[]) {
    duk_context *ctx = NULL;
    int code = 0;

    ctx = duk_create_heap_default();
    if (!ctx) {
        printf("Failed to create a Duktape heap.\n");
        exit(1);
    }

    duk_push_c_function(ctx, pmain, 2);
	duk_push_int(ctx, argc);
	duk_push_pointer(ctx, argv);
	if (duk_pcall(ctx, 2) != DUK_EXEC_SUCCESS) {
		printf("%s\n", duk_safe_to_string(ctx, -1));
		code = 1;
	}

    duk_destroy_heap(ctx);
    
    return code;
}

/*
------------------------------------------------------------------------------------
*/

static void prepare_duk_env(duk_context *ctx) {
	register_dfstream(ctx);
	register_mod_search(ctx);

	register_dconsole(ctx);
	register_dprocess(ctx);

	preload_dos(ctx);
	preload_dfs(ctx);
}

static void duknode_push_argv(duk_context *ctx, int argc, const char *argv[]) {
	duk_get_global_string(ctx, "process");

	duk_idx_t arg_array_index; int i;

	arg_array_index = duk_push_array(ctx);
	for (i = 0; i < argc; i++) {
		duk_push_string(ctx, argv[i]);
		duk_put_prop_index(ctx, arg_array_index, i);
	}

	duk_put_prop_string(ctx, -2, "argv");
	duk_pop(ctx);
}