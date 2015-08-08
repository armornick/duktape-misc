/*
I/O Library for Duktape (based on stdio)
Based on the liolib.c file of the Lua 5.2.3 source.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <duktape.h>

#define DEBUG /* activate debug mode */

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
	#ifdef DEBUG
	printf("%s\n", "calling dukio file finalizer");
	#endif

	FILE *f = dukio_require_file(ctx, 0);
	fclose(f);
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
		filemode = "r";
	}

	FILE *f = fopen(filename, filemode);
	if (f == NULL) {
		duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "could not open file '%s'", filename);
		return -1;
	}

	dukio_push(ctx, f, filename);
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

/* NOTE: finalizer will be called for stdin and stdout */
static void dukio_push_std(duk_context *ctx, int index) {
	dukio_push(ctx, stdin, "in");
	duk_put_prop_string(ctx, index, "stdin");

	dukio_push(ctx, stdout, "out");
	duk_put_prop_string(ctx, index, "stdout");
}

static const duk_function_list_entry dukio_file_prototype[] = {
	{ "gets", dukio_gets, 0 },
	{ "puts", dukio_puts, 1},
	{ NULL, NULL, 0 }
};

static int dukio_core(duk_context *ctx) {
	int mod = duk_push_object(ctx);

	/* create prototype */
	duk_push_object(ctx);
	duk_push_c_function(ctx, dukio_file_finalizer, 0);
	duk_set_finalizer(ctx, -2);
	duk_put_function_list(ctx, -1, dukio_file_prototype);
	duk_put_global_string(ctx, DUKIOFILE_PROTOTYPE);

	dukio_push_std(ctx, mod);

	duk_push_c_function(ctx, dukio_open, 2);
	duk_put_prop_string(ctx, mod, "open");

	return mod;
}

duk_ret_t dukopen_io(duk_context *ctx) {
	dukio_core(ctx);
	return 1;
}

void register_dukio(duk_context *ctx) {
	dukio_core(ctx);
	duk_put_global_string(ctx, "io");
}