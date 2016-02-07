#include <stdio.h>
#include <stdlib.h>
#include "duk.hpp"

#include "file.hpp"

const char script[] = "print('writing to file'); var outputf = new File('test.txt', 'wb'); " \
						"outputf.writeLine('Hello, World!'); outputf = null; " \
						"print('reading from file'); var inputf = new File('test.txt', 'rb'); " \
						"var inputs = inputf.readLine(); print('input from file: ', inputs)";

void test_register(duk_context *ctx) {
	dukbinder_register<File>(ctx, "File", "$FilePrototype", file_prototype, file_allocator);
}

int main(int argc, char const *argv[]) {
	duk_context *ctx = duk_create_heap_default();
	
	if (!ctx) {
		printf("Failed to create a Duktape heap.\n");
		exit(1);
	}

	test_register(ctx);

	if (duk_peval_string(ctx, script) != 0) {
		printf("ERROR: %s\n", duk_safe_to_string(ctx, -1));
	}

	duk_destroy_heap(ctx);
	return 0;
}