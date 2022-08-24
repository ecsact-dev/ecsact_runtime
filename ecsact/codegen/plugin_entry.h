#ifndef ECSACT_CODEGEN_PLUGIN_ENTRY_H
#define ECSACT_CODEGEN_PLUGIN_ENTRY_H

#include "ecsact/runtime/meta.h"

#ifndef ECSACT_CODEGEN_PLUGIN_API
#	ifdef __cplusplus
#		define ECSACT_CODEGEN_PLUGIN_API extern "C" __attribute__((visibility("default")))
#	else
#		define ECSACT_CODEGEN_PLUGIN_API extern __attribute__((visibility("default")))
# endif
#endif // ECSACT_CODEGEN_PLUGIN_API

ECSACT_CODEGEN_PLUGIN_API const char* ecsact_codegen_plugin_name();

ECSACT_CODEGEN_PLUGIN_API void ecsact_codegen_plugin
	( ecsact_package_id package_id
	);

#undef ECSACT_CODEGEN_PLUGIN_API
#endif//ECSACT_CODEGEN_PLUGIN_ENTRY_H
