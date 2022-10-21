#ifndef ECSACT_CODEGEN_PLUGIN_H
#define ECSACT_CODEGEN_PLUGIN_H

#include "ecsact/runtime/meta.h"

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

using ecsact_codegen_write_fn_t = void (*)(const char* str, int32_t str_len);

ECSACT_CODEGEN_PLUGIN_API const char* ecsact_codegen_plugin_name();

/**
 * Ecsact codegen plugin entry pointer. Plugin developers implement this
 * function to write code generation output via the `write_fn` parameter.
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
