/*
File Stream object for Duktape.
Implements a subset of the Stream object of Node.js
see: https://nodejs.org/api/stream.html
*/

#include "duknode.h"


/*
------------------------------------------------------------------------------------
*/

/* Object property to hold file pointer */
#define DFSTREAM_HANDLE_PROP "$data"
/* Names of the File stream properties */
#define DIFSTREAM_PROTOTYPE "FileStreamReadablePrototype"
#define DOFSTREAM_PROTOTYPE "FileStreamWritablePrototype"

/*
------------------------------------------------------------------------------------
*/

FILE* dfstream_require_file(duk_context *ctx, int index) {
	FILE *result;

	duk_get_prop_string(ctx, index, DFSTREAM_HANDLE_PROP);
	result = duk_get_pointer(ctx, -1);
	duk_pop(ctx);

	if (!result) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "Expected file stream at index %d", index);
		return NULL;
	}

	return result;
}

static FILE* dfstream_file_from_this(duk_context *ctx) {
	FILE *f;
	duk_push_this(ctx);
	f = dfstream_require_file(ctx, -1);
	duk_pop(ctx);
	return f;
}

/*
------------------------------------------------------------------------------------
*/

void push_difstream(duk_context *ctx, FILE *inputf) {
	duk_push_object(ctx);
	duk_get_global_string(ctx, DIFSTREAM_PROTOTYPE);
	duk_set_prototype(ctx, -2);

	duk_push_pointer(ctx, inputf);
	duk_put_prop_string(ctx, -2, DFSTREAM_HANDLE_PROP);
}

void push_dofstream(duk_context *ctx, FILE *outputf) {
	duk_push_object(ctx);
	duk_get_global_string(ctx, DOFSTREAM_PROTOTYPE);
	duk_set_prototype(ctx, -2);

	duk_push_pointer(ctx, outputf);
	duk_put_prop_string(ctx, -2, DFSTREAM_HANDLE_PROP);
}

static duk_ret_t dfstream_finalizer(duk_context *ctx) {
	FILE *f = dfstream_require_file(ctx, 0);
	fclose(f);
	return 0;
}

/*
------------------------------------------------------------------------------------
*/

static duk_ret_t difstream_read(duk_context *ctx) {
	FILE *f = dfstream_file_from_this(ctx);

	if (duk_is_number(ctx, 0)) {

		long byten = duk_require_int(ctx, 0);
		void *bytes = duk_push_fixed_buffer(ctx, byten);
		fread(bytes, 1, byten, f);

	} else {

		fseek(f, 0, SEEK_END);
		long size = ftell(f);
		fseek(f, 0, SEEK_SET);
		void *bytes = duk_push_fixed_buffer(ctx, size);
		fread(bytes, 1, size, f);
		rewind(f);

	}

	return 1;
}

static duk_ret_t dofstream_write(duk_context *ctx) {
	FILE *f = dfstream_file_from_this(ctx);

	if (duk_is_buffer(ctx, 0)) {
		
		void *buffer; duk_size_t sz;
		buffer = duk_get_buffer(ctx, 0, &sz);
		fwrite(buffer, 1, sz, f);

	} else if (duk_is_string(ctx, 0)) {

		const char *outputs = duk_get_string(ctx, 0);
		fputs(outputs, f);

	}

	return 0;
}

/*
------------------------------------------------------------------------------------
*/

static const duk_function_list_entry difstream_prototype[] = {
	{ "read", difstream_read, 1 },
	{ NULL, NULL, 0}
};

static const duk_function_list_entry dofstream_prototype[] = {
	{ "write", dofstream_write, 1 },
	{ NULL, NULL, 0}
};


void register_dfstream(duk_context *ctx) {
	duk_push_object(ctx);
	duk_push_c_function(ctx, dfstream_finalizer, 1);
	duk_set_finalizer(ctx, -2);
	duk_put_function_list(ctx, -1, difstream_prototype);
	duk_put_global_string(ctx, DIFSTREAM_PROTOTYPE);

	duk_push_object(ctx);
	duk_push_c_function(ctx, dfstream_finalizer, 1);
	duk_set_finalizer(ctx, -2);
	duk_put_function_list(ctx, -1, dofstream_prototype);
	duk_put_global_string(ctx, DOFSTREAM_PROTOTYPE);
}