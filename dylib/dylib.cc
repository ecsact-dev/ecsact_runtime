#include <cstring>
#include <cassert>
#include "ecsact/runtime/dylib.h"

#ifdef ECSACT_ASYNC_API_LOAD_AT_RUNTIME
#	include "ecsact/runtime/async.h"
#endif

#ifdef ECSACT_CORE_API_LOAD_AT_RUNTIME
#	include "ecsact/runtime/core.h"
#endif

#ifdef ECSACT_DYNAMIC_API_LOAD_AT_RUNTIME
#	include "ecsact/runtime/dynamic.h"
#endif

#ifdef ECSACT_META_API_LOAD_AT_RUNTIME
#	include "ecsact/runtime/meta.h"
#endif

#ifdef ECSACT_SERIALIZE_API_LOAD_AT_RUNTIME
#	include "ecsact/runtime/serialize.h"
#endif

#include "./dylib-util.hh"

#ifdef ECSACT_ASYNC_API_LOAD_AT_RUNTIME
FOR_EACH_ECSACT_ASYNC_API_FN(ECSACT_DYLIB_UTIL_FN_PTR_DEFN);
#endif

#ifdef ECSACT_CORE_API_LOAD_AT_RUNTIME
FOR_EACH_ECSACT_CORE_API_FN(ECSACT_DYLIB_UTIL_FN_PTR_DEFN);
#endif

#ifdef ECSACT_DYNAMIC_API_LOAD_AT_RUNTIME
FOR_EACH_ECSACT_DYNAMIC_API_FN(ECSACT_DYLIB_UTIL_FN_PTR_DEFN);
#endif

#ifdef ECSACT_META_API_LOAD_AT_RUNTIME
FOR_EACH_ECSACT_META_API_FN(ECSACT_DYLIB_UTIL_FN_PTR_DEFN);
#endif

#ifdef ECSACT_SERIALIZE_API_LOAD_AT_RUNTIME
FOR_EACH_ECSACT_SERIALIZE_API_FN(ECSACT_DYLIB_UTIL_FN_PTR_DEFN);
#endif

#define HAS_FN_CHECK(fn_name, target_fn_name) \
	if(std::strcmp(target_fn_name, #fn_name) == 0) return true

#define ASSIGN_FN_IF(fn_name, target_fn_name, fn_ptr)          \
	if(std::strcmp(#fn_name, target_fn_name) == 0) {             \
		::fn_name = reinterpret_cast<decltype(::fn_name)>(fn_ptr); \
	}                                                            \
	static_assert(true, "macro requires ;")

bool ecsact_dylib_has_fn(const char* fn_name) {
#ifdef ECSACT_ASYNC_API_LOAD_AT_RUNTIME
	FOR_EACH_ECSACT_ASYNC_API_FN(HAS_FN_CHECK, fn_name);
#endif

#ifdef ECSACT_CORE_API_LOAD_AT_RUNTIME
	FOR_EACH_ECSACT_CORE_API_FN(HAS_FN_CHECK, fn_name);
#endif

#ifdef ECSACT_DYNAMIC_API_LOAD_AT_RUNTIME
	FOR_EACH_ECSACT_DYNAMIC_API_FN(HAS_FN_CHECK, fn_name);
#endif

#ifdef ECSACT_META_API_LOAD_AT_RUNTIME
	FOR_EACH_ECSACT_META_API_FN(HAS_FN_CHECK, fn_name);
#endif

#ifdef ECSACT_SERIALIZE_API_LOAD_AT_RUNTIME
	FOR_EACH_ECSACT_SERIALIZE_API_FN(HAS_FN_CHECK, fn_name);
#endif

	return false;
}

void ecsact_dylib_set_fn_addr(const char* fn_name, void (*fn_ptr)()) {
#ifndef NDEBUG
	if(std::strncmp(fn_name, "ecsact_", 7) != 0) {
		assert(false && "Cannot load non ecsact fn");
		return;
	}
#endif // NDEBUG

#ifdef ECSACT_ASYNC_API_LOAD_AT_RUNTIME
	FOR_EACH_ECSACT_ASYNC_API_FN(ASSIGN_FN_IF, fn_name, fn_ptr);
#endif

#ifdef ECSACT_CORE_API_LOAD_AT_RUNTIME
	FOR_EACH_ECSACT_CORE_API_FN(ASSIGN_FN_IF, fn_name, fn_ptr);
#endif

#ifdef ECSACT_DYNAMIC_API_LOAD_AT_RUNTIME
	FOR_EACH_ECSACT_DYNAMIC_API_FN(ASSIGN_FN_IF, fn_name, fn_ptr);
#endif

#ifdef ECSACT_META_API_LOAD_AT_RUNTIME
	FOR_EACH_ECSACT_META_API_FN(ASSIGN_FN_IF, fn_name, fn_ptr);
#endif

#ifdef ECSACT_SERIALIZE_API_LOAD_AT_RUNTIME
	FOR_EACH_ECSACT_SERIALIZE_API_FN(ASSIGN_FN_IF, fn_name, fn_ptr);
#endif
}
