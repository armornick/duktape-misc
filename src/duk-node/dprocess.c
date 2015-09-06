/*
Process object for Duktape.
Implements a subset of the process object of Node.js
see: https://nodejs.org/api/process.html

Notes:

* depends on dfstream.c

*/

#include "duknode.h"

/*
------------------------------------------------------------------------------------
*/

#if DUKNODE_PLATFORM_WINDOWS 

int setenv(const char *envname, const char *envval, int overwrite) {
	int bufsiz = strlen(envname) + strlen(envval) + 2;
	// char *envstring = malloc(bufsiz);
	char envstring[bufsiz];
	snprintf(envstring, bufsiz, "%s=%s", envname, envval);
	return _putenv(envstring);
}

#endif

/*
------------------------------------------------------------------------------------
*/

static void dprocess_push_execpath(duk_context *ctx) {
	char exepath[DUKNODE_MAX_PATH];
	memset(exepath, 0, DUKNODE_MAX_PATH);

#if DUKNODE_PLATFORM_WINDOWS 
	if (!GetModuleFileName(NULL, exepath, DUKNODE_MAX_PATH)) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "Unable to get executable path");
	}
#else
	/*
	see: https://stackoverflow.com/questions/933850/how-to-find-the-location-of-the-executable-in-c

	This is Linux-only (BSD and Solaris have other paths to the current exe)
	Still looking for a pure POSIX way to get current executable
	Might create a CURRENT_EXE_PATH define based on the OS
	*/
	readlink("/proc/self/exe", exepath, DUKNODE_MAX_PATH);
	exepath[DUKNODE_MAX_PATH-1] = '\0'; /* readlink not necessarily null-terminated */
#endif

	duk_push_string(ctx, exepath);
}

static duk_ret_t dprocess_chdir(duk_context *ctx) {
	const char *path = duk_require_string(ctx, 0);

	if( chdir( path ) < 0 ) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "Unable to set working directory");
		return -1;
	}

	return 0;
}

static duk_ret_t dprocess_cwd(duk_context *ctx) {
	char path[DUKNODE_MAX_PATH];

	if (!getcwd(path, DUKNODE_MAX_PATH)) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "Unable to get cwd");
		return -1;
	}

	duk_push_string(ctx, path);
	return 1;
}

static duk_ret_t dprocess_exit(duk_context *ctx) {
	int code = duk_is_number(ctx, 0) ? duk_require_int(ctx, 0) : 0;
	exit(code);
	return 0;
}

static duk_ret_t dprocess_getenv(duk_context *ctx) {
	const char *key = duk_require_string(ctx, 0);
	char *value;

	value = getenv(key);
	duk_push_string(ctx, value);

	return 1;
}

static duk_ret_t dprocess_setenv(duk_context *ctx) {
	const char *key = duk_require_string(ctx, 0);
	const char *value = duk_require_string(ctx, 1);

	setenv(key, value, 1);

	return 0;
}

/*
------------------------------------------------------------------------------------
*/

static const duk_function_list_entry process_methods[] = {
	{ "chdir", dprocess_chdir, 1 },
	{ "cwd", dprocess_cwd, 0 },
	{ "exit", dprocess_exit, 1 },
	{ "getenv", dprocess_getenv, 1 },
	{ "setenv", dprocess_setenv, 2 },
	{ NULL, NULL, 0}
};

void register_dprocess(duk_context *ctx) {
	duk_push_object(ctx);

	duk_put_function_list(ctx, -1, process_methods);

	duk_push_string(ctx, DUKNODE_PLATFORM_NAME);
	duk_put_prop_string(ctx, -2, "platform");

	duk_push_string(ctx, DUKNODE_ARCH_NAME);
	duk_put_prop_string(ctx, -2, "arch");

	dprocess_push_execpath(ctx);
	duk_put_prop_string(ctx, -2, "execPath");

	push_dofstream(ctx, stdout);
	duk_put_prop_string(ctx, -2, "stdout");

	push_dofstream(ctx, stderr);
	duk_put_prop_string(ctx, -2, "stderr");

	push_difstream(ctx, stdin);
	duk_put_prop_string(ctx, -2, "stdin");

	duk_put_global_string(ctx, "process");
}