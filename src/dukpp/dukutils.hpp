/*
Templated utility functions for Duktape.

Heavily based on LuaWrapper 
https://bitbucket.org/alexames/luawrapper/src
*/

/*
Duktape type handling structure.
See below for implementations for common types.
*/

#ifndef _DUKPP_HPP_
#error __FILE__ ## " is not intended for standalone use."
#endif

template <typename T>
struct dukpp_Impl {
	static T dukpp_require(duk_context *ctx, duk_idx_t index);
	static T dukpp_get(duk_context *ctx, duk_idx_t index);
	static void dukpp_push(duk_context *ctx, const T& value);
};

/*
-----------------------------------------------------------------
Globablized functions to get/push Duktape values from/to the stack.
*/
template<typename T> T dukpp_require(duk_context *ctx, duk_idx_t index) {
	return dukpp_Impl<T>::dukpp_require(ctx, index);
}

template<typename T> T dukpp_get(duk_context *ctx, duk_idx_t index) {
	return dukpp_Impl<T>::dukpp_get(ctx, index);
}

template<typename T> void dukpp_push(duk_context *ctx, const T& index) {
	return dukpp_Impl<T>::dukpp_push(ctx, index);
}

/*
-----------------------------------------------------------------
Utility functions using the main Dukpp functions above.
*/
template<typename T> T dukpp_opt(duk_context *ctx, duk_idx_t index, const T& fallback = T()) {
	if (duk_is_null_or_undefined(ctx, index)) {
		return fallback;
	} else {
		return dukpp_require<T>(ctx, index);
	}
}

/*
-----------------------------------------------------------------
Implementations of the Duktape type handling structure
for the most common data types.
*/
template<>
struct dukpp_Impl<bool> {
	static bool dukpp_require(duk_context *ctx, duk_idx_t index) { return duk_require_boolean(ctx, index); }
	static bool dukpp_get(duk_context *ctx, duk_idx_t index) { return duk_get_boolean(ctx, index); }
	static void dukpp_push(duk_context *ctx, const bool& value) { duk_push_boolean(ctx, value); }
};

template<>
struct dukpp_Impl<const char *> {
	static const char* dukpp_require(duk_context *ctx, duk_idx_t index) { return duk_require_string(ctx, index); }
	static const char* dukpp_get(duk_context *ctx, duk_idx_t index) { return duk_get_string(ctx, index); }
	static void dukpp_push(duk_context *ctx, const char* const& value) { duk_push_string(ctx, value); }
};

template<>
struct dukpp_Impl<unsigned int> {
	static unsigned int dukpp_require(duk_context *ctx, duk_idx_t index) { return static_cast<unsigned int>(duk_require_uint(ctx, index)); }
	static unsigned int dukpp_get(duk_context *ctx, duk_idx_t index) { return static_cast<unsigned int>(duk_get_uint(ctx, index)); }
	static void dukpp_push(duk_context *ctx, const unsigned int& value) { duk_push_uint(ctx, value); }
};

template<>
struct dukpp_Impl<int> {
	static int dukpp_require(duk_context *ctx, duk_idx_t index) { return static_cast<int>(duk_require_int(ctx, index)); }
	static int dukpp_get(duk_context *ctx, duk_idx_t index) { return static_cast<int>(duk_get_int(ctx, index)); }
	static void dukpp_push(duk_context *ctx, const int& value) { duk_push_int(ctx, value); }
};

template<>
struct dukpp_Impl<unsigned char> {
	static unsigned char dukpp_require(duk_context *ctx, duk_idx_t index) { return static_cast<unsigned char>(duk_require_int(ctx, index)); }
	static unsigned char dukpp_get(duk_context *ctx, duk_idx_t index) { return static_cast<unsigned char>(duk_get_int(ctx, index)); }
	static void dukpp_push(duk_context *ctx, const unsigned char& value) { duk_push_int(ctx, value); }
};

template<>
struct dukpp_Impl<char> {
	static char dukpp_require(duk_context *ctx, duk_idx_t index) { return static_cast<char>(duk_require_int(ctx, index)); }
	static char dukpp_get(duk_context *ctx, duk_idx_t index) { return static_cast<char>(duk_get_int(ctx, index)); }
	static void dukpp_push(duk_context *ctx, const char& value) { duk_push_int(ctx, value); }
};

template<>
struct dukpp_Impl<float> {
	static float dukpp_require(duk_context *ctx, duk_idx_t index) { return static_cast<float>(duk_require_number(ctx, index)); }
	static float dukpp_get(duk_context *ctx, duk_idx_t index) { return static_cast<float>(duk_get_number(ctx, index)); }
	static void dukpp_push(duk_context *ctx, const float& value) { duk_push_number(ctx, value); }
};

template<>
struct dukpp_Impl<double> {
	static double dukpp_require(duk_context *ctx, duk_idx_t index) { return static_cast<double>(duk_require_number(ctx, index)); }
	static double dukpp_get(duk_context *ctx, duk_idx_t index) { return static_cast<double>(duk_get_number(ctx, index)); }
	static void dukpp_push(duk_context *ctx, const double& value) { duk_push_number(ctx, value); }
};

#ifndef DUKPP_NO_STL

template<>
struct dukpp_Impl<std::string> {
	static std::string dukpp_require(duk_context *ctx, duk_idx_t index) { return std::string(duk_require_string(ctx, index)); }
	static std::string dukpp_get(duk_context *ctx, duk_idx_t index) { return std::string(duk_get_string(ctx, index)); }
	static void dukpp_push(duk_context *ctx, const std::string& value) { duk_push_string(ctx, value.c_str()); }
};

#endif