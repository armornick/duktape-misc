/*
OS Library for Duktape
Based on the loslib.c file of the Lua 5.2.4 source.

NOTE: I'm skipping the date functions for now because
ES5 has built-in Date objects.

define BUILD_AS_DLL to build as a duktape module.
*/

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <duktape.h>


#define duk_tmpnam(b,e)         { e = (tmpnam(b) == NULL); }


static duk_ret_t dukos_execute(duk_context *ctx) {
	if (duk_is_string(ctx, 0)) {

		const char *cmd = duk_require_string(ctx, 0);
		int stat = system(cmd);
		duk_push_int(ctx, stat);
		return 1;

	} else {

		int stat = system(NULL);
		duk_push_boolean(ctx, stat);
		return 1;

	}
}

static duk_ret_t dukos_remove(duk_context *ctx) {
	const char *filename = duk_require_string(ctx, 0);
	int stat = remove(filename);
	duk_push_boolean(ctx, stat == 0);
	return 1;
}

static duk_ret_t dukos_rename(duk_context *ctx) {
	const char *fromname = duk_require_string(ctx, 0);
	const char *toname = duk_require_string(ctx, 1);
	duk_push_boolean(ctx, rename(fromname, toname) == 0);
	return 1;
}

static duk_ret_t dukos_getenv(duk_context *ctx) {
	const char *envvar = duk_require_string(ctx, 0);
	char *val = getenv(envvar);
	duk_push_string(ctx, val);
	return 1;
}

/*
NOTE: might be a terrible idea to just exit but I suppose
the OS will clean up all resources. Might want to set
close to true by default?
*/
static duk_ret_t dukos_exit(duk_context *ctx) {
	int status, close;
	duk_bool_t success = duk_require_boolean(ctx, 0);
	status = (success ? EXIT_SUCCESS : EXIT_FAILURE);
	close = (duk_is_boolean(ctx, 1) ? duk_require_boolean(ctx, 1) : 0);

	if (close) { duk_destroy_heap(ctx); }
	if (ctx) exit(status); /* 'if' to avoid warnings for unreachable 'return' */
	return 0;
}

static duk_ret_t dukos_tmpname(duk_context *ctx) {
	char buff[L_tmpnam];
	int err;

	duk_tmpnam(buff, err);

	if (err) { duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "unable to generate a unique filename"); }

	duk_push_string(ctx, buff);
	return 1;
}

static const duk_function_list_entry dukos_module[] = {
	{ "execute", dukos_execute, 1 },
	{ "remove", dukos_remove, 1 },
	{ "rename", dukos_rename, 2 },
	{ "getenv", dukos_getenv, 1 },
	{ "exit", dukos_exit, 2 },
	{ "tmpname", dukos_tmpname, 0},
	{ NULL, NULL, 0}
};


static int dukos_core(duk_context *ctx) {
	int mod = duk_push_object(ctx);
	duk_put_function_list(ctx, -1, dukos_module);
	return mod;
}


#ifdef BUILD_AS_DLL

#if defined(_WIN32)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

DLL_EXPORT duk_ret_t dukopen_os(duk_context *ctx) {
	dukos_core(ctx);
	return 1;
}

#else

void register_dukos(duk_context *ctx) {
	dukos_core(ctx);
	duk_put_global_string(ctx, "os");
}

#endif