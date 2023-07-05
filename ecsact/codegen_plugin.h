#ifndef ECSACT_CODEGEN_PLUGIN_H
#define ECSACT_CODEGEN_PLUGIN_H

#include "ecsact/runtime/common.h"

#ifndef ECSACT_CODEGEN_PLUGIN_API
#	ifdef __cplusplus
#		ifdef _WIN32
#			define ECSACT_CODEGEN_PLUGIN_API extern "C" __declspec(dllexport)
#		else
#			define ECSACT_CODEGEN_PLUGIN_API \
				extern "C" __attribute__((visibility("default")))
#		endif
#	else
#		ifdef _WIN32
#			define ECSACT_CODEGEN_PLUGIN_API extern __declspec(dllexport)
#		else
#			define ECSACT_CODEGEN_PLUGIN_API \
				extern __attribute__((visibility("default")))
#		endif
#	endif
#endif // ECSACT_CODEGEN_PLUGIN_API

/**
 * Characters passed to this function are written to file or stdout. Characters
 * that go beyond @p str_len are not read.
 *
 * @NOTE: it is _NOT_ assumed that @p str is null-terminated, you must set
 * @p str_len properly.
 *
 * @param str - array of characters of length @p str_len
 * @param str_len - length of array of characters @p str
 */
typedef void (*ecsact_codegen_write_fn_t)( //
	const char* str,
	int32_t     str_len
);

ECSACT_CODEGEN_PLUGIN_API const char* ecsact_codegen_plugin_name();

/**
 * Ecsact codegen plugin entrypoint. Plugin developers implement this function
 * to write code generation output via the `write_fn` parameter.
 *
 * It is expected that the implementation uses the runtime meta module
 * functions.
 *
 * @param package_id package the codegen plugin writes for
 * @param write_fn implementation calls this function when code generation
 *        output
 */
ECSACT_CODEGEN_PLUGIN_API void ecsact_codegen_plugin( //
	ecsact_package_id         package_id,
	ecsact_codegen_write_fn_t write_fn
);

#endif // ECSACT_CODEGEN_PLUGIN_H
