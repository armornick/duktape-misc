#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "duktape.h"

int main(int argc, char const *argv[]) {
	duk_context *ctx = NULL;

	ctx = duk_create_heap_default();
    if (!ctx) {
        printf("Failed to create a Duktape heap.\n");
        exit(1);
    }

    // program logic here

    duk_destroy_heap(ctx);

	return 0;
}