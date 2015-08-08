/*
Dynamic library loader for Duktape.

Heavily based on the loadlib.c file of Lua 5.2.3.
*/

// #define DEBUG /* activate debug mode */

/*
** if needed, includes windows header before everything else
*/
#if defined(_WIN32)
#include <windows.h>

#ifndef strdup
#define strdup _strdup
#endif

#endif

#include <stdlib.h>
#include <string.h>

#ifdef DEBUG

	#include <stdio.h>

	#define dbg(v) printf("%s\n", v)

#else
	
	#define dbg(v) (void)v

#endif

#include <duktape.h>

/*
** DUK_PATH and DUK_CPATH are the names of the environment
** variables that Lua check to set its paths.
*/
#if !defined(DUK_PATH)
#define DUK_PATH	"DUK_PATH"
#endif

#if !defined(DUK_CPATH)
#define DUK_CPATH	"DUK_CPATH"
#endif

/*
** DUK_PATH_SEP is the character that separates templates in a path.
** DUK_PATH_MARK is the string that marks the substitution points in a
** template.
** DUK_EXEC_DIR in a Windows path is replaced by the executable's
** directory.
** DUK_IGMARK is a mark to ignore all before it when building the
** dukopen_ function name.
*/
#if !defined (DUK_PATH_SEP)
#define DUK_PATH_SEP		";"
#endif
#if !defined (DUK_PATH_MARK)
#define DUK_PATH_MARK		"?"
#endif
#if !defined (DUK_EXEC_DIR)
#define DUK_EXEC_DIR		"!"
#endif
#if !defined (DUK_IGMARK)
#define DUK_IGMARK		"-"
#endif

/*
** LUA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/

// naively assume all valid platforms can handle forward slash as separator
#define DUK_DIRSEP "/"

#if !defined(DUK_CSUBSEP)
#define DUK_CSUBSEP		DUK_DIRSEP
#endif

#if !defined(DUK_LSUBSEP)
#define DUK_LSUBSEP		DUK_DIRSEP
#endif

/* prefix for open functions in C libraries */
#define DUK_POF		"dukopen_"

/* separator for open functions in C libraries */
#define DUK_OFSEP	"_"

/* error codes for duk_loadfunc */
#define ERRLIB		1
#define ERRFUNC		2

/* table (in the registry) that keeps handles for all loaded C libraries */
#define CLIBS		"$$CLIBS"


/*
** system-dependent functions
*/
static void duk_unloadlib (void *lib);
static void *duk_load (duk_context *ctx, const char *path, int seeglb);
static duk_c_function duk_sym (duk_context *ctx, void *lib, const char *sym);

#if defined(_WIN32)
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

/*
** optional flags for LoadLibraryEx
*/
#if !defined(DUK_LLE_FLAGS)
#define DUK_LLE_FLAGS	0
#endif

static void duk_unloadlib (void *lib) {
  FreeLibrary((HMODULE)lib);
}

static void *duk_load (duk_context *ctx, const char *path, int seeglb) {
  HMODULE lib = LoadLibraryExA(path, NULL, DUK_LLE_FLAGS);
  (void)(seeglb);  /* not used: symbols are 'global' by default */
  if (lib == NULL) duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not load library: %s", path);
  return lib;
}


static duk_c_function duk_sym (duk_context *ctx, void *lib, const char *sym) {
  #ifdef DEBUG
  	printf("inside duk_sym: { lib: %x, sym: %s }\n", lib, sym);
  #endif
  duk_c_function f = (duk_c_function)GetProcAddress((HMODULE)lib, sym);
  if (f == NULL) duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not load library function: %s", sym);
  return f;
}

#else
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface.
** The dlfcn interface is available in Linux, SunOS, Solaris, IRIX, FreeBSD,
** NetBSD, AIX 4.2, HPUX 11, and  probably most other Unix flavors, at least
** as an emulation layer on top of native functions.
** =========================================================================
*/

// armornick: I do not have a UNIX system so this part is untested.

#include <dlfcn.h>

static void duk_unloadlib (void *lib) {
  dlclose(lib);
}


static void *duk_load (lua_State *L, const char *path, int seeglb) {
  void *lib = dlopen(path, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  if (lib == NULL) duk_error(ctx, DUK_ERR_INTERNAL_ERROR, dlerror());
  return lib;
}


static duk_c_function duk_sym (lua_State *L, void *lib, const char *sym) {
  duk_c_function f = (duk_c_function)dlsym(lib, sym);
  if (f == NULL) duk_error(ctx, DUK_ERR_INTERNAL_ERROR, dlerror());
  return f;
}

/* }====================================================== */
#endif



/*
** {======================================================
** 'modSearch' function
** =======================================================
*/

static void *duk_checkclib (duk_context *ctx, const char *path) {
  dbg("checking library existence");

  void *plib;
  duk_get_global_string(ctx, "Duktape");
  duk_get_prop_string(ctx, -1, CLIBS);
  duk_get_prop_string(ctx, -1, path);
  plib = duk_get_pointer(ctx, -1);  /* plib = CLIBS[path] */
  duk_pop_3(ctx);  /* pop Duktape table, CLIBS table and 'plib' */
  return plib;
}


static void duk_addtoclib (duk_context *ctx, const char *path, void *plib) {
  dbg("adding library to library registry");
  duk_get_global_string(ctx, "Duktape");
  duk_get_prop_string(ctx, -1, CLIBS);
  duk_push_pointer(ctx, plib);
  duk_put_prop_string(ctx, -2, path);  /* CLIBS[path] = plib */
  duk_pop_2(ctx);  /* pop Duktape and CLIBS table */
}

static int duk_loadfunc (duk_context *ctx, const char *path, const char *sym) {
  void *reg = duk_checkclib(ctx, path);  /* check loaded C libraries */
  if (reg == NULL) {  /* must load library? */
  	dbg("attempting to load library");
    reg = duk_load(ctx, path, *sym == '*');
    if (reg == NULL) return ERRLIB;  /* unable to load library */
    duk_addtoclib(ctx, path, reg);
  }

    duk_c_function f = duk_sym(ctx, reg, sym);
    if (f == NULL)
      return ERRFUNC;  /* unable to find function */
    duk_push_c_function(ctx, f, 0);  /* else create new function */
    return 0;  /* no errors */
}

static const char * duk_build_sym(duk_context *ctx) {
	dbg("attempting to build init function");
	duk_push_string(ctx, DUK_POF);
	duk_dup(ctx, 0);
	duk_concat(ctx, 2);

	const char *sym = strdup(duk_get_string(ctx, -1));
	duk_pop(ctx);

	return sym;
}

duk_ret_t duk_loadlib (duk_context *ctx) {
  dbg("entering loader");
  const char *path = duk_require_string(ctx, 0);
  const char *init = duk_build_sym(ctx);
  int stat = duk_loadfunc(ctx, path, init);
  
  (void)free((void*)init); /* free result from strdup */

  if (stat == 0)  /* no errors? */
    return 1;  /* return the loaded function */
  else {  /* error; error message is on stack top */
    duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not load library: %s", path);
  }
}
