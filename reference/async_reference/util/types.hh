#pragma once

#include <vector>

#include "ecsact/runtime/core.h"
#include "ecsact/runtime/async.h"

namespace types {
struct callback_info {
	ecsact_event        event;
	ecsact_entity_id    entity_id;
	ecsact_component_id component_id;
};

struct async_error {
	ecsact_async_error error = ECSACT_ASYNC_OK;
	int32_t            request_id;
};

struct cpp_execution_component {
	ecsact_entity_id    entity_id;
	ecsact_component_id _id;
};

struct cpp_execution_options {
	std::vector<cpp_execution_component> adds;
	std::vector<cpp_execution_component> updates;
	std::vector<cpp_execution_component> removes;
	std::vector<ecsact_action>           actions;
};
} // namespace types
