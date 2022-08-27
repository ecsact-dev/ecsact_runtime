#include "ecsact/codegen_plugin.h"

const char* ecsact_codegen_plugin_name() {
	return "empty";
}

void ecsact_codegen_plugin
	( ecsact_package_id
	, ecsact_codegen_write_fn_t
	)
{
}
