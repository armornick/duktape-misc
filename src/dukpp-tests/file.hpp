/*
Test class to bind to Duktape.
*/
#ifndef _FILE_HPP_
#define _FILE_HPP_

#include <stdlib.h>
#include <stdio.h>
#include "duk.hpp"

class File {
private:
	FILE *_handle;

public:
	File(const char *filename, const char *mode);
	~File();

	void writeLine(const char *line);
	void readLine(char *buff, int bufsize);
};

extern const duk_function_list_entry file_prototype[];

File* file_allocator(duk_context *ctx);

#endif // _FILE_HPP_