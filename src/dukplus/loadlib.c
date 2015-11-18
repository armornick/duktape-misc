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

#if !defined(DUK_PATH_DEFAULT)
#define DUK_PATH_DEFAULT "./?;./?.js"
#endif

#if !defined(DUK_CPATH)
#define DUK_CPATH	"DUK_CPATH"
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

/* table containing the 'preloaded' (internal in the application) modules */
#define PRELOAD_TABLE "$PRELOAD"

/* handle for dynamic library objects */
#define DUK_LIB_HANDLE "$data"


/*
** system-dependent functions
*/
static void duk_unloadlib (void *lib);
static void *duk_load (duk_context *ctx, const char *path, int seeglb);
static duk_c_function duk_sym (duk_context *ctx, void *lib, const char *sym);

#if defined(_WIN32)

#if !defined(DUK_CPATH_DEFAULT)
#define DUK_CPATH_DEFAULT "./?.dll"
#endif

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

#if !defined(DUK_CPATH_DEFAULT)
#define DUK_CPATH_DEFAULT "./?.so"
#endif

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

static void *duk_checkclib (duk_context *ctx, const char *path) {
  dbg("checking library existence");

  void *plib;
  duk_get_global_string(ctx, "package");
  duk_get_prop_string(ctx, -1, CLIBS);
  duk_get_prop_string(ctx, -1, path);
  plib = duk_get_pointer(ctx, -1);  /* plib = CLIBS[path] */
  duk_pop_3(ctx);  /* pop Duktape table, CLIBS table and 'plib' */
  return plib;
}

static duk_ret_t duk_lib_finalizer(duk_context *ctx) {
  dbg("dynamic library finalizer");

  duk_get_prop_string(ctx, 0, DUK_LIB_HANDLE);

  void *lib = duk_get_pointer(ctx, -1);
  duk_unloadlib(lib);
}


static void duk_addtoclib (duk_context *ctx, const char *path, void *plib) {
  dbg("adding library to library registry");
  duk_get_global_string(ctx, "package");
  duk_get_prop_string(ctx, -1, CLIBS);

  duk_push_object(ctx);
  duk_push_pointer(ctx, plib);
  duk_put_prop_string(ctx, -2, DUK_LIB_HANDLE);
  duk_push_c_function(ctx, duk_lib_finalizer, 1);
  duk_set_finalizer(ctx, -2);
  

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

/*
** {======================================================
** 'Function to load dll and get dukopen_*
** =======================================================
*/


duk_ret_t duk_loadlib (duk_context *ctx) {
  dbg("entering loader");
  const char *path = duk_require_string(ctx, 1);
  const char *init = duk_build_sym(ctx);
  int stat = duk_loadfunc(ctx, path, init);
  
  (void)free((void*)init); /* free result from strdup */

  if (stat == 0)  /* no errors? */
    return 1;  /* return the loaded function */
  else {  /* error; error message is on stack top */
    duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not load library: %s", path);
  }
}


/*
** {======================================================
** 'modSearch' function
** =======================================================
*/

static int readable (const char *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  if (f == NULL) return 0;  /* open failed */
  fclose(f);
  return 1;
}

static duk_ret_t duk_readable (duk_context *ctx) {
  const char *filename = duk_require_string(ctx, 0);
  duk_push_boolean(ctx, readable(filename));
  return 1;
}

static duk_ret_t duk_read_file (duk_context *ctx) {

  FILE *f = fopen(duk_require_string(ctx, 0), "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char inbuff[fsize + 1];
  fread(inbuff, fsize, 1, f);
  fclose(f);
  inbuff[fsize] = 0;

  duk_push_string(ctx, inbuff);
  return 1;
}

static const char *default_jpath() {
  char *jpath = getenv(DUK_PATH);
  if (jpath) { return jpath; }
  return DUK_PATH_DEFAULT;
}

static const char *default_cpath() {
  char *cpath = getenv(DUK_CPATH);
  if (cpath) { return cpath; }
  return DUK_CPATH_DEFAULT;
}



static char _modsearch_src[] = "Duktape.modSearch = function (id, require, exports, module) {" "function extend(from, to) {" \
    "for (var key in from) { if (from.hasOwnProperty(key) && !to.hasOwnProperty(key)) { to[key] = from[key] } } " \
  "}" "function searchPath(pathlist) {"   "pathlist = pathlist.replace(/\\?/g, id); " \
    "pathlist = pathlist.split(';');"   "for (var i = 0; i < pathlist.length; i++) {" \
      "var path = pathlist[i]; if (package.exists(path)) { return path; }"     "}"     "return null;" \
  "}"     "if (package.preload[id]) { var pr = package.preload[id]; if (typeof pr == 'string') { return pr; } else { extend(package.preload[id], exports); return undefined;} }" \
    "var found = searchPath(package.cpath);"     "if (found) {"       "var mod = package.loadlib(id, found)();" \
      "extend(mod, exports); return undefined;"     "}" \
    "var found = searchPath(package.path);"     "if (found) {"       "return package.readFile(found);" \
    "}"     "throw new Error('module not found: ' + id);" "}";

void register_mod_search(duk_context *ctx) {

  duk_idx_t pkg_obj;
  pkg_obj = duk_push_object(ctx);

  duk_push_string(ctx, default_jpath()); /* default_jpath */
  duk_put_prop_string(ctx, pkg_obj, "path");

  duk_push_string(ctx, default_cpath()); /* default_cpath */
  duk_put_prop_string(ctx, pkg_obj, "cpath");

  duk_push_object(ctx); /* preload table (empty) */
  duk_put_prop_string(ctx, pkg_obj, "preload");

  duk_push_object(ctx); /* c library cache (empty) */
  duk_put_prop_string(ctx, pkg_obj, CLIBS);

  duk_push_c_function(ctx, duk_readable, 1); /* exists function */
  duk_put_prop_string(ctx, pkg_obj, "exists");

  duk_push_c_function(ctx, duk_loadlib, 2); /* loadlib function */
  duk_put_prop_string(ctx, pkg_obj, "loadlib");

  duk_push_c_function(ctx, duk_read_file, 1); /* readFile function */
  duk_put_prop_string(ctx, pkg_obj, "readFile");

  duk_put_global_string(ctx, "package"); /* register package table */

  // -----------------------------------------------------------------------------------------------
  duk_eval_string_noresult(ctx, _modsearch_src);

}