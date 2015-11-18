/*
I/O Library for Duktape (based on stdio)
Based on the liolib.c file of the Lua 5.2.3 source.

define BUILD_AS_DLL to build as a duktape module.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <duktape.h>

// #define DEBUG /* activate debug mode */

#if !defined(duk_checkmode)

/*
** Check whether 'mode' matches '[rwa]%+?b?'.
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#define duk_checkmode(mode) \
	(*mode != '\0' && strchr("rwa", *(mode++)) != NULL &&	\
	(*mode != '+' || ++mode) &&  /* skip if char is '+' */	\
	(*mode != 'b' || ++mode) &&  /* skip if char is 'b' */	\
	(*mode == '\0'))
#endif

/* Object property to hold file pointer */
#define FILEHANDLE_PROP "$$file"
#define FILENAME_PROP "path"
/* Name of the dukioFile prototype */
#define DUKIOFILE_PROTOTYPE "DukioFilePrototype"
/* File reading buffer size */
#define DUKIO_BUFSIZ 512


/* get filehandle from object */
static FILE* dukio_require_file(duk_context *ctx, int index) {
	FILE *result;
	
	duk_get_prop_string(ctx, index, FILEHANDLE_PROP);
	result = duk_get_pointer(ctx, -1);
	duk_pop(ctx);
	if (!result) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "Expected dukio file at index %d", index);
		return NULL;
	}

	return result;
}

static FILE* dukio_file_from_this(duk_context *ctx) {
	FILE *f;
	duk_push_this(ctx);
	f = dukio_require_file(ctx, -1);
	duk_pop(ctx);
	return f;
}

static void dukio_push(duk_context *ctx, FILE *f, const char *filename) {
	/* create object with dukioFile prototype */
	duk_push_object(ctx);
	duk_get_global_string(ctx, DUKIOFILE_PROTOTYPE);
	duk_set_prototype(ctx, -2);

	/* set the file pointer data */
	duk_push_pointer(ctx, f);
	duk_put_prop_string(ctx, -2, FILEHANDLE_PROP);

	/* set path property */
	duk_push_string(ctx, filename);
	duk_put_prop_string(ctx, -2, FILENAME_PROP);

}

static duk_ret_t dukio_file_finalizer(duk_context *ctx) {
	#if 0
	printf("%s\n", "calling dukio file finalizer");
	#endif

	FILE *f = dukio_require_file(ctx, 0);
	fclose(f);

	#ifdef DEBUG
	printf("%s\n", "finished calling dukio file finalizer");
	#endif
	return 0;
}


/* io.open (filename, filemode) : opens new file */
static duk_ret_t dukio_open(duk_context *ctx) {
	const char *filename = duk_require_string(ctx, 0);
	const char *filemode;

	if (duk_is_string(ctx, 1)) {
		filemode = duk_require_string(ctx, 1);
		/* file mode sanity check */
		const char *md = filemode;
		if (!duk_checkmode(md)) {
			duk_error(ctx, DUK_ERR_TYPE_ERROR, "%s is not a valid file mode", filemode);
			return -1;
		}
	} else {
		/* open as readable by default */
		filemode = "rb";
	}

	FILE *f = fopen(filename, filemode);
	if (f == NULL) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not open file '%s'", filename);
		return -1;
	}

	dukio_push(ctx, f, filename);
	return 1;
}

static duk_ret_t dukio_rewind(duk_context *ctx) {
	FILE *f = dukio_file_from_this(ctx);

	rewind(f);

	return 0;
}

static duk_ret_t dukio_size(duk_context *ctx) {
	FILE *f = dukio_file_from_this(ctx);

	long current_pos = ftell(f);
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, current_pos, SEEK_SET);

	duk_push_int(ctx, size);
	return 1;
}

static duk_ret_t dukio_gets(duk_context *ctx) {
	FILE *f = dukio_file_from_this(ctx);
	char buffer[DUKIO_BUFSIZ];

	fgets(buffer, DUKIO_BUFSIZ, f);

	duk_push_string(ctx, buffer);
	return 1;
}

static duk_ret_t dukio_puts(duk_context *ctx) {
	FILE *f = dukio_file_from_this(ctx);
	const char *s = duk_require_string(ctx, 0);

	fputs(s, f);

	return 0;
}

static duk_ret_t dukio_read(duk_context *ctx) {
	FILE *f = dukio_file_from_this(ctx);
	long byten = duk_require_int(ctx, 0);

	#ifdef DEBUG
	printf("%s %d\n", "reading from file: ", byten);
	#endif

	void *bytes = duk_push_fixed_buffer(ctx, byten);
	fread(bytes, 1, byten, f);

	#ifdef DEBUG
	printf("%s\n", "finished reading");
	#endif

	return 1;
}

static duk_ret_t dukio_readall(duk_context *ctx) {
	FILE *f = dukio_file_from_this(ctx);

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	void *bytes = duk_push_fixed_buffer(ctx, size);
	fread(bytes, 1, size, f);
	rewind(f);

	return 1;
}

static duk_ret_t dukio_get(duk_context *ctx) {
	FILE *f = dukio_file_from_this(ctx);

	int c = fgetc(f);

	duk_push_int(ctx, c);
	return 1;
}

static duk_ret_t dukio_getc(duk_context *ctx) {
	FILE *f = dukio_file_from_this(ctx);
	char s[2];

	int c = fgetc(f);
	s[0] = (char) c;
	s[1] = '\0';

	duk_push_string(ctx, s);
	return 1;
}

static duk_ret_t dukio_readfile(duk_context *ctx) {
	const char *filename = duk_require_string(ctx, 0);
	FILE *f = fopen(filename, "rb");

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	void *bytes = duk_push_fixed_buffer(ctx, size);
	fread(bytes, 1, size, f);
	fclose(f);

	return 1;
}

static duk_ret_t dukio_writefile(duk_context *ctx) {
	const char *filename = duk_require_string(ctx, 0);
	FILE *f = fopen(filename, "wb+");
	
	if (duk_is_buffer(ctx, 1) || duk_is_object(ctx, 1)) {
		
		void *buffer; duk_size_t sz;
		buffer = duk_require_buffer_data(ctx, 1, &sz);
		fwrite(buffer, 1, sz, f);

	} else if (duk_is_string(ctx, 1)) {

		const char *outputs = duk_get_string(ctx, 1);
		fputs(outputs, f);

	}

	fclose(f);

	return 0;
}

static duk_ret_t dukio_exists(duk_context *ctx) {
	const char *filename = duk_require_string(ctx, 0);

	FILE *f = fopen(filename, "r");
	duk_push_boolean(ctx, (f != NULL));
	fclose(f);

	return 1;
}

static const duk_function_list_entry dukio_file_prototype[] = {
	{ "rewind", dukio_rewind, 0},
	{ "gets", dukio_gets, 0 },
	{ "puts", dukio_puts, 1},
	{ "read", dukio_read, 1},
	{ "readAll", dukio_readall, 0},
	{ "get", dukio_get, 0},
	{ "getc", dukio_getc, 0},
	{ "size", dukio_size, 0 },
	{ NULL, NULL, 0 }
};

static const duk_function_list_entry dukio_module[] = {
	{ "open", dukio_open, 2 },
	{ "readFile", dukio_readfile, 1 },
	{ "writeFile", dukio_writefile, 2},
	{ "exists", dukio_exists, 1 },
	{ NULL, NULL, 0}
};

/* NOTE: finalizer will be called for stdin and stdout */
static void dukio_push_std(duk_context *ctx, int index) {
	dukio_push(ctx, stdin, "in");
	duk_put_prop_string(ctx, index, "stdin");

	dukio_push(ctx, stdout, "out");
	duk_put_prop_string(ctx, index, "stdout");
}

static int dukio_core(duk_context *ctx) {
	int mod = duk_push_object(ctx);

	/* create prototype */
	duk_push_object(ctx);
	duk_push_c_function(ctx, dukio_file_finalizer, 1);
	duk_set_finalizer(ctx, -2);
	duk_put_function_list(ctx, -1, dukio_file_prototype);
	duk_put_global_string(ctx, DUKIOFILE_PROTOTYPE);

	dukio_push_std(ctx, mod);
	duk_put_function_list(ctx, -1, dukio_module);

	// duk_push_c_function(ctx, dukio_open, 2);
	// duk_put_prop_string(ctx, mod, "open");

	return mod;
}

#ifdef BUILD_AS_DLL

#if defined(_WIN32)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

DLL_EXPORT duk_ret_t dukopen_io(duk_context *ctx) {
	dukio_core(ctx);
	return 1;
}

#else

void register_dukio(duk_context *ctx) {
	dukio_core(ctx);
	duk_put_global_string(ctx, "io");

	duk_eval_string_noresult(ctx, "Duktape.modLoaded['io'] = io");
}

#endif