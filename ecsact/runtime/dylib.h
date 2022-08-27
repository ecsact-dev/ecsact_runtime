#ifndef ECSACT_DYLIB_H
#define ECSACT_DYLIB_H

#ifndef ECSACT_DYLIB_API
#	ifdef __cplusplus
#		ifdef _WIN32
#			define ECSACT_DYLIB_API extern "C" __declspec(dllexport)
#		else
#			define ECSACT_DYLIB_API extern "C" __attribute__((visibility("default")))
#		endif
#	else
#		ifdef _WIN32
#			define ECSACT_DYLIB_API extern __declspec(dllexport)
#		else
#			define ECSACT_DYLIB_API extern __attribute__((visibility("default")))
#		endif
# endif
#endif // ECSACT_DYLIB_API

/**
 * @returns `true` if runtime API function is settable via
 *          `ecsact_dylib_set_fn_addr`
 */
ECSACT_DYLIB_API bool ecsact_dylib_has_fn
	( const char* fn_name
	);

/**
 * Set runtime API function address
 */
ECSACT_DYLIB_API void ecsact_dylib_set_fn_addr
	( const char* fn_name
	, void(*fn_ptr)()
	);

#endif//ECSACT_DYLIB_H
