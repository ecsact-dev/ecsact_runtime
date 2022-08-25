#include <cstring>
#include <cassert>
#include "ecsact/runtime/dylib.h"

#ifdef ECSACT_META_API_LOAD_AT_RUNTIME
#include "ecsact/runtime/meta.h"
#endif

#define HAS_FN_CHECK(fn_name, target_fn_name)\
	if(std::strcmp(target_fn_name, #fn_name) == 0) return true

#define ASSIGN_FN_IF(fn_name, target_fn_name, fn_ptr)\
	if(std::strcmp(#fn_name, target_fn_name) == 0)\
		decltype(::fn_name) (::fn_name) =\
			reinterpret_cast<decltype(::fn_name)>(fn_ptr)

bool ecsact_dylib_has_fn
	( const char* fn_name
	)
{
#ifdef ECSACT_META_API_LOAD_AT_RUNTIME
	FOR_EACH_ECSACT_META_API_FN(HAS_FN_CHECK, fn_name);
#endif
	return false;
}

void ecsact_dylib_set_fn_addr
	( const char* fn_name
	, void(*fn_ptr)()
	)
{
	if(std::strncmp(fn_name, "ecsact_", 7) != 0) {
		assert(false && "Cannot load non ecsact fn");
		return;
	}

#ifdef ECSACT_META_API_LOAD_AT_RUNTIME
	// FOR_EACH_ECSACT_META_API_FN(ASSIGN_FN_IF, fn_name, fn_ptr);
#endif
}
