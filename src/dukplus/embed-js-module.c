#include <duktape.h>

#if defined(_WIN32)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

/*
NOTE: doesn't work if you put 'return' at the start of the embedded script.
    Also, putting a literal like "{ name: 'Billy B. Bobson' }" didn't seem to work either
*/
const char _mod_src[] = "(function() { " \
                        " return { name: 'Billy B. Bobson' } " \
                        "})()";

DLL_EXPORT duk_ret_t dukopen_jstest (duk_context *ctx) {
	duk_eval_string(ctx, _mod_src);
	return 1;
}