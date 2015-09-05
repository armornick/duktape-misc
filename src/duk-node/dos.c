/*
OS module for Duktape.
Implements a subset of the os module of Node.js
see: https://nodejs.org/api/os.html
*/

#include "duknode.h"

/*
------------------------------------------------------------------------------------
*/


/*
https://stackoverflow.com/questions/8087805/how-to-get-system-or-user-temp-folder-in-unix-and-windows
*/
static duk_ret_t dos_tmpdir(duk_context *ctx) {
#if DUKNODE_PLATFORM_WINDOWS 
	char tmpdir[DUKNODE_MAX_PATH];
	memset(tmpdir, 0, DUKNODE_MAX_PATH);

	GetTempPath(DUKNODE_MAX_PATH, tmpdir);

	duk_push_string(ctx, tmpdir);
	return 1;
#else
	char *tmpdir;

	if (getenv("TMPDIR")) {
		tmpdir = getenv("TMPDIR");
	} else if (getenv("TMP")) {
		tmpdir = getenv("TMP");
	} else if (getenv("TEMP")) {
		tmpdir = getenv("TEMP");
	} else if (getenv("TEMPDIR")) {
		tmpdir = getenv("TEMPDIR");
	} else {
		tmpdir = "/tmp";
	}

	duk_push_string(ctx, tmpdir);
	return 1;
#endif
}

static duk_ret_t dos_platform(duk_context *ctx) {
	duk_push_string(ctx, DUKNODE_PLATFORM_NAME);
	return 1;
}

static duk_ret_t dos_arch(duk_context *ctx) {
	duk_push_string(ctx, DUKNODE_ARCH_NAME);
	return 1;
}

static duk_ret_t dos_type(duk_context *ctx) {
	duk_push_string(ctx, DUKNODE_PLATFORM_TYPE);
	return 1;
}


/*
------------------------------------------------------------------------------------
*/

static const duk_function_list_entry dos_module[] = {
	{ "tmpdir", dos_tmpdir, 0 },
	{ "platform", dos_platform, 0 },
	{ "arch", dos_arch, 0 },
	{ "type", dos_type, 0 },
	{ NULL, NULL, 0}
};

static void dos_core(duk_context *ctx) {
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, dos_module);
}

#ifdef BUILD_AS_DLL

DLL_EXPORT duk_ret_t dukopen_os(duk_context *ctx) {
	dos_core(ctx);
	return 1;
}

#else

void register_dos(duk_context *ctx) {
	dos_core(ctx);
	duk_put_global_string(ctx, "os");
}

void preload_dos(duk_context *ctx) {
	duk_get_global_string(ctx, "package");
	duk_get_prop_string(ctx, -1, "preload");
	dos_core(ctx);
	duk_put_prop_string(ctx, -2, "os");
	duk_pop_2(ctx);
}

#endif