/**
 * Convenience header to include all runtime modules
 */

#ifndef ECSACT_RUNTIME_H
#define ECSACT_RUNTIME_H

#include "ecsact/runtime/core.h"
#include "ecsact/runtime/dynamic.h"
#include "ecsact/runtime/static.h"
#include "ecsact/runtime/meta.h"
#include "ecsact/runtime/serialize.h"

#define FOR_EACH_ECSACT_API_FN(fn, ...)            \
	FOR_EACH_ECSACT_CORE_API_FN(fn, __VA_ARGS__);    \
	FOR_EACH_ECSACT_DYNAMIC_API_FN(fn, __VA_ARGS__); \
	FOR_EACH_ECSACT_META_API_FN(fn, __VA_ARGS__);    \
	FOR_EACH_ECSACT_STATIC_API_FN(fn, __VA_ARGS__);  \
	FOR_EACH_ECSACT_SERIALIZE_API_FN(fn, __VA_ARGS__)

#endif // ECSACT_RUNTIME_H
