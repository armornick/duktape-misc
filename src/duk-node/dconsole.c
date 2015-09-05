/*
Console object for Duktape.
Implements a subset of the console object of Node.js
see: https://nodejs.org/api/console.html
*/

#include "duknode.h"


static duk_ret_t dconsole_log(duk_context *ctx) {
	int n = duk_get_top(ctx);
	int i = 0;

	for (i = 0; i < n; i++) {
		printf("%s", duk_to_string(ctx, i));
	}

	printf("\n");
	return 0;
}

static const duk_function_list_entry console_methods[] = {
	{ "log", dconsole_log, DUK_VARARGS },
	{ "info", dconsole_log, DUK_VARARGS },
	{ "error", dconsole_log, DUK_VARARGS },
	{ "warn", dconsole_log, DUK_VARARGS },
	{ NULL, NULL, 0}
};

void register_dconsole(duk_context *ctx) {
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, console_methods);
	duk_put_global_string(ctx, "console");
}