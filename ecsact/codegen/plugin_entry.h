#ifndef ECSACT_CODEGEN_PLUGIN_ENTRY_H
#define ECSACT_CODEGEN_PLUGIN_ENTRY_H

#include "ecsact/runtime/meta.h"

extern "C"
__attribute__((visibility("default")))
void ecsact_codegen_plugin
	( ecsact_package_id package_id
	);

#endif//ECSACT_CODEGEN_PLUGIN_ENTRY_H
