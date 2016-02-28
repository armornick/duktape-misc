
#ifndef _DUKPP_HPP_
#error __FILE__ ## " is not intended for standalone use."
#endif

/*
Wrapper around a Duktape stack value.
*/
class DukValue 
{
private:
	duk_context *_ctx;
	duk_idx_t _index;
	
public:

	/* hold duktape value based on Duktape stack index */
	DukValue(duk_context *ctx, duk_idx_t index) : _ctx(ctx)
	{
		if (index >= 0)
		{
			_index = index;
		}
		else
		{
			_index = duk_require_normalize_index(ctx, index);
		}
	}

	duk_int_t type()
	{
		return duk_get_type(_ctx, _index);
	}

	/* remove value from the stack */
	void pop()
	{
		duk_remove(_ctx, _index);
	}

	/* duplicate value on the stack */
	DukValue dup()
	{
		duk_dup(_ctx, _index);
		return DukValue(_ctx, -1);
	}

	template<typename T>
	bool is()
	{
		return dukpp_is<T>(_ctx, _index);
	}

	/* get the value on the stack as a value of type T */
	template<typename T>
	T get()
	{
		return dukpp_get<T>(_ctx, _index);
	}

	/* check that the value on the stack is type T and return it, or throw an error */
	template<typename T>
	T require()
	{
		return dukpp_require<T>(_ctx, _index);
	}

	/* convert value on the stack to integer (destructive) */
	duk_int_t to_integer()
	{
		return duk_to_int(_ctx, _index);
	}

	/* convert the value on the stack to integer (non-destructive) */
	duk_int_t as_integer()
	{
		duk_dup(_ctx, _index);
		duk_int_t result = duk_to_int(_ctx, -1);
		duk_pop(_ctx);
		return result;
	}

	/* convert value on the stack to integer (destructive) */
	duk_double_t to_number()
	{
		return duk_to_number(_ctx, _index);
	}

	/* convert the value on the stack to integer (non-destructive) */
	duk_double_t as_number()
	{
		duk_dup(_ctx, _index);
		duk_double_t result = duk_to_number(_ctx, -1);
		duk_pop(_ctx);
		return result;
	}

	/* convert value on the stack to boolean (destructive) */
	duk_bool_t  to_boolean()
	{
		return duk_to_boolean(_ctx, _index);
	}

	/* convert the value on the stack to boolean (non-destructive) */
	duk_bool_t  as_boolean()
	{
		duk_dup(_ctx, _index);
		duk_bool_t result = duk_to_boolean(_ctx, -1);
		duk_pop(_ctx);
		return result;
	}

	/* convert value on the stack to integer (destructive) */
	const char* to_string()
	{
		return duk_to_string(_ctx, _index);
	}

	/* get property based on string key */
	DukValue prop(const char *key)
	{
		duk_get_prop_string(_ctx, _index, key);
		return DukValue(_ctx, -1);
	}

	/* get property based on numeric key */
	DukValue prop(int key)
	{
		duk_get_prop_index(_ctx, _index, (duk_idx_t)key);
		return DukValue(_ctx, -1);
	}

	/* get property based on value */
	template<typename K>
	DukValue prop(K key)
	{
		dukpp_push(_ctx, key);
		duk_get_prop(_ctx, _index);
		return DukValue(_ctx, -1);
	}

	/* set property based on string key */
	template<typename V>
	duk_bool_t prop(const char *key, V value)
	{
		dukpp_push(_ctx, value);
		return duk_put_prop_string(_ctx, _index, key);
	}

	/* set property based on numeric key */
	template<typename V>
	duk_bool_t prop(int key, V value)
	{
		dukpp_push(_ctx, value);
		return duk_put_prop_index(_ctx, _index, (duk_idx_t)key);
	}

	/* set property based on value key */
	template<typename K, typename V>
	duk_bool_t prop(K key, V value)
	{
		dukpp_push(_ctx, key);
		dukpp_push(_ctx, value);
		return duk_put_prop(_ctx, _index);
	}

	operator bool() const
	{
		return (bool) !duk_is_null_or_undefined(_ctx, _index);
	}

};