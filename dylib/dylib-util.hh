#pragma once

#define ECSACT_DYLIB_UTIL_FN_PTR_DEFN(fn_name, unused)\
	decltype(::fn_name) (::fn_name) = nullptr

#define ECSACT_DYLIB_UTIL_FN_PTR_ASSIGN(fn_name)\
	(::fn_name) = nullptr
