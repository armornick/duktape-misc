#include "file.hpp"

/*
---------------------------------------------------
Class implementation;
*/

File::File(const char *filename, const char *mode) {
	_handle = fopen(filename, mode);
}

File::~File() {
	printf("running destructor for File\n");
	if (_handle != NULL) {
		fclose(_handle);
	}
}

void File::writeLine(const char *line) {
	if (_handle != NULL) {
		fputs(line, _handle);
	}
}

void File::readLine(char *buff, int bufsize) {
	if (_handle != NULL) {
		fgets(buff, bufsize, _handle);
	}
}

/*
---------------------------------------------------
Duktape prototype methods
*/

static duk_ret_t File_writeLine(duk_context *ctx) {
	File *f = dukbinder_get_from_this<File>(ctx);
	const char *s = duk_require_string(ctx, 0);

	f->writeLine(s);

	return 0;
}

static duk_ret_t File_readLine(duk_context *ctx) {
	File *f = dukbinder_get_from_this<File>(ctx);
	char buffer[512];

	f->readLine(buffer, 512);

	duk_push_string(ctx, buffer);
	return 1;
}

const duk_function_list_entry file_prototype[] = {
	{ "writeLine", File_writeLine, 1 },
	{ "readLine", File_readLine, 0 },
	{ NULL, NULL, 0 }
};

/*
---------------------------------------------------
Duktape class allocator
*/

File* file_allocator(duk_context *ctx) {
	const char *filename = duk_require_string(ctx, 0);
	const char *filemode = duk_require_string(ctx, 1);

	return new File(filename, filemode);
}