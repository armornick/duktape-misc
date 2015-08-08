#include <duktape.h>

#if defined(_WIN32)
#define DLL_EXPORT __declspec(dllexport)
#endif

static duk_ret_t my_addtwo(duk_context *ctx) {
    double a, b;

    /* Here one can expect that duk_get_top(ctx) == 2, because nargs
     * for duk_push_c_function() is 2.
     */

    a = duk_get_number(ctx, 0);
    b = duk_get_number(ctx, 1);
    duk_push_number(ctx, a + b);
    return 1;   /*  1 = return value at top
                 *  0 = return 'undefined'
                 * <0 = throw error (use DUK_RET_xxx constants)
                 */
}

static duk_ret_t my_hello(duk_context *ctx) {
	duk_push_string(ctx, "hello, world!");
	return 1;
}

const duk_function_list_entry test_module_functions[] = {
	{ "hello", my_hello, 0 },
	{ "addTwo", my_addtwo, 2 },
	{ NULL, NULL, 0 }
};

DLL_EXPORT duk_ret_t dukopen_test (duk_context *ctx) {
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, test_module_functions);
	return 1;
}