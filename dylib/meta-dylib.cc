#include "ecsact/runtime/meta.h"

#include "./dylib-util.hh"

#ifdef ECSACT_META_API_LOAD_AT_RUNTIME
FOR_EACH_ECSACT_META_API_FN(ECSACT_DYLIB_UTIL_FN_PTR_DEFN);
#else
#	error meta-dylib can only used with ECSACT_META_API_LOAD_AT_RUNTIME
#endif
