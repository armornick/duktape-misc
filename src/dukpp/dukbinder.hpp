/*
Duktape templated class binding.

Based on LuaWrapper
https://bitbucket.org/alexames/luawrapper/src
*/

#ifndef _DUKPP_HPP_
#error __FILE__ ## " is not intended for standalone use."
#endif

/*
-----------------------------------------------------------------
Constants used to bind classes to Duktape.
*/
#define DUKBINDER_HANDLE "$$data"

/*
-----------------------------------------------------------------
Default allocator and deallocator function.
*/
template<class T>
T* dukbinder_allocator_default(duk_context *ctx) {
	return new T();
}

template<class T>
void dukbinder_deallocator_default(duk_context *ctx, T* obj) {
	delete obj;
}

/*
-----------------------------------------------------------------
Main Duktape binding class used to generate the constructor and
finalizer.
*/
template<class T>
struct dukbinder_Impl {
	static const char *className;
	static const char *prototypeName;
	static T* (*allocator)(duk_context *ctx);
	static void (*deallocator)(duk_context *ctx, T* obj);

	static T* get_instance(duk_context *ctx, duk_idx_t index) {
		T* result;

		duk_get_prop_string(ctx, index, DUKBINDER_HANDLE);
		result = static_cast<T*>(duk_get_pointer(ctx, -1));
		duk_pop(ctx);

		return result;
	}

	static T* get_instance_from_this(duk_context *ctx) {
		T* result;
		
		duk_push_this(ctx);
		result = get_instance(ctx, -1);
		duk_pop(ctx);

		return result;
	}

	static duk_bool_t check_prototype(duk_context *ctx, duk_idx_t index) {
		duk_bool_t result;

		duk_get_prototype(ctx, index);
		duk_get_global_string(ctx, prototypeName);
		result = duk_equals(ctx, -1, -2);
		duk_pop_2(ctx);

		return result;
	}

	static void push_instance(duk_context *ctx, T* obj, bool object_ready) {
		/* create new object if none is prepared in advance */
		if (!object_ready) {
			duk_push_object(ctx);
		}

		/* set the prototype */
		duk_get_global_string(ctx, prototypeName);
		duk_set_prototype(ctx, -2);

		/* set the handle to the pointer */
		duk_push_pointer(ctx, obj);
		duk_put_prop_string(ctx, -2, DUKBINDER_HANDLE);
	}

	static duk_ret_t constructor(duk_context *ctx) {
		T* obj = allocator(ctx);

		/* push either 'this' or a new object */
		if (duk_is_constructor_call(ctx)) {
			duk_push_this(ctx);
		} else {
			duk_push_object(ctx);
		}

		push_instance(ctx, obj, true);

		return 1;
	}

	static duk_ret_t finalizer(duk_context *ctx) {
		T* obj = get_instance(ctx, 0);
		deallocator(ctx, obj);
		return 0;
	}

private:
	dukbinder_Impl();

};

template<class T> const char * dukbinder_Impl<T>::className;
template<class T> const char * dukbinder_Impl<T>::prototypeName;
template<class T> T* (*dukbinder_Impl<T>::allocator)(duk_context *ctx);
template<class T> void (*dukbinder_Impl<T>::deallocator)(duk_context *ctx, T* obj);

/*
-----------------------------------------------------------------
Utility functions to get/push an instance from Duktape.
*/
template <class T>
T* dukbinder_get(duk_context *ctx, duk_idx_t index) {
	return dukbinder_Impl<T>::get_instance(ctx, index);
}

template <class T>
duk_bool_t dukbinder_is(duk_context *ctx, duk_idx_t index) {
	return dukbinder_Impl<T>::check_prototype(ctx, index);
}

template <class T>
T* dukbinder_check(duk_context *ctx, duk_idx_t index) {
	if (!dukbinder_is<T>(ctx, index)) {
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "Expected %s at index %d", dukbinder_Impl<T>::className, index);
		return NULL;
	}
	return dukbinder_get<T>(ctx, index);
}

template<class T>
T* dukbinder_get_from_this(duk_context *ctx) {
	return dukbinder_Impl<T>::get_instance_from_this(ctx);
}

template<class T>
void dukbinder_push(duk_context *ctx, T* obj) {
	dukbinder_Impl<T>::push_instance(ctx, obj, false);
}

/*
-----------------------------------------------------------------
Main function to register a class to Duktape.
*/
template<class T>
void dukbinder_register(duk_context *ctx, const char *className, const char *prototypeName, 
	const duk_function_list_entry *prototype, T* (*allocator)(duk_context *ctx) = dukbinder_allocator_default<T>, 
	void (*deallocator)(duk_context *ctx, T* obj) = dukbinder_deallocator_default<T>)
{
	dukbinder_Impl<T>::className = className;
	dukbinder_Impl<T>::prototypeName = prototypeName;
	dukbinder_Impl<T>::allocator = allocator;
	dukbinder_Impl<T>::deallocator = deallocator;

	duk_push_object(ctx);
	duk_push_c_function(ctx, dukbinder_Impl<T>::finalizer, 1);
	duk_set_finalizer(ctx, -2);
	duk_put_function_list(ctx, -1, prototype);
	duk_put_global_string(ctx, prototypeName);

	duk_push_c_function(ctx, dukbinder_Impl<T>::constructor, DUK_VARARGS);
	duk_put_global_string(ctx, className);
}