#include <stdio.h>
#include <stdlib.h>
#include "duk.hpp"

const char script[] = "print('my_bool: ', my_bool, toString.call(my_bool)); print('my_string: ', my_string, toString.call(my_string)); " \
						"print('my_int: ', my_int, toString.call(my_int)); " \
						"print('my_float: ', my_float, toString.call(my_float)); print('my_char: ', my_char, toString.call(my_char)); " \
						"var script_number = 14.3, script_string = 'such amaze'; script_bool = true; " \
						"var script_object = {'str': 'wow', 'num': 123.4};";

void test_push(duk_context *ctx) {
	dukpp_push<bool>(ctx, true);
	(void) duk_put_global_string(ctx, "my_bool");

	dukpp_push<const char*>(ctx, "There is no spppon!");
	(void) duk_put_global_string(ctx, "my_string");

	dukpp_push(ctx, 123);
	(void) duk_put_global_string(ctx, "my_int");

	dukpp_push(ctx, 4.56);
	(void) duk_put_global_string(ctx, "my_float");

	dukpp_push(ctx, 'z');
	(void) duk_put_global_string(ctx, "my_char");
}

void test_get(duk_context *ctx) {
	duk_get_global_string(ctx, "script_number");
	printf("script_number (as double): %f\n", dukpp_get<double>(ctx, -1));
	printf("script_number (as int): %d\n", dukpp_get<int>(ctx, -1));
	printf("script_number (as bool): %d\n", dukpp_to<bool>(ctx, -1));
	duk_pop(ctx);

	duk_get_global_string(ctx, "script_string");
	printf("script_string: %s\n", dukpp_get<const char*>(ctx, -1));
	printf("script_string (as bool): %d\n", dukpp_to<bool>(ctx, -1));
	duk_pop(ctx);

	duk_get_global_string(ctx, "script_bool");
	printf("script_bool: %d\n", dukpp_get<bool>(ctx, -1));
	printf("script_bool (as string): %s\n", dukpp_to<const char*>(ctx, -1));
	printf("script_bool (as int): %d\n", dukpp_to<int>(ctx, -1));
	duk_pop(ctx);

	duk_get_global_string(ctx, "does_not_exist");
	printf("does_not_exist (fallback): %d\n", dukpp_opt<int>(ctx, -1, 1337));
	duk_pop(ctx);
}

void test_object(duk_context *ctx) {
	duk_get_global_string(ctx, "script_object");

	printf("script_object.num (as double): %f\n", dukpp_getfield<double>(ctx, -1, "num"));
	printf("script_object.num (as int): %d\n", dukpp_getfield<int>(ctx, -1, "num"));
	printf("script_object.str: %s\n", dukpp_getfield<std::string>(ctx, -1, "str").c_str());

	duk_pop(ctx);
}

int main(int argc, char const *argv[]) {
	duk_context *ctx = duk_create_heap_default();
	
	if (!ctx) {
		printf("Failed to create a Duktape heap.\n");
		exit(1);
	}

	test_push(ctx);

	if (duk_peval_string(ctx, script) != 0) {
		printf("ERROR: %s\n", duk_safe_to_string(ctx, -1));
	}

	test_get(ctx);
	test_object(ctx);

	duk_destroy_heap(ctx);
	return 0;
}