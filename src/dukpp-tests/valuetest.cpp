#include <stdio.h>
#include <stdlib.h>
#include "duk.hpp"

void register_tests(duk_context *ctx);

const char script[] = "var o = {}; test(12); test('hello'); test(o); fill(o); print(JSON.stringify(o)); fill(34);";

int main(int argc, char const *argv[]) {
	duk_context *ctx = duk_create_heap_default();
	
	if (!ctx) {
		printf("Failed to create a Duktape heap.\n");
		exit(1);
	}

	register_tests(ctx);

	if (duk_peval_string(ctx, script) != 0) {
		printf("ERROR: %s\n", duk_safe_to_string(ctx, -1));
	}

	duk_destroy_heap(ctx);
	return 0;
}



static duk_ret_t test_object(duk_context *ctx)
{
	DukValue param1(ctx, 0);

	if (param1)
	{
		if (param1.is<long>())
		{
			printf("param1 is a number (int %ld)\n", param1.get<long>());
		}
		else if (param1.type() == DUK_TYPE_OBJECT)
		{
			printf("param1 is an Object\n");
		}
		else
		{
			printf("param1 is not a number or an Object\n");
		}
	}
	else
	{
		printf("parameter 1 was not given\n");
	}

	return 0;
}

static duk_ret_t fill_object(duk_context *ctx)
{
	DukValue param1(ctx, 0);

	if (param1.type() == DUK_TYPE_OBJECT)
	{
		param1.prop("test_int", 15);
		param1.prop("test_string", "there is no spoon!");
		param1.prop("test_bool", true);
		param1.prop("test_double", (double)12.4);
	}
	else
	{
		printf("invalid argument to fill_object\n");
		return DUK_RET_TYPE_ERROR;
	}

	return 0;
}

void register_tests(duk_context *ctx)
{
	duk_push_c_function(ctx, test_object, 1);
	duk_put_global_string(ctx, "test");

	duk_push_c_function(ctx, fill_object, 1);
	duk_put_global_string(ctx, "fill");
}