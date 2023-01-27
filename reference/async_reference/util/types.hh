#pragma once

#include <vector>
#include <optional>
#include <variant>
#include <cstddef>

#include "ecsact/runtime/core.h"
#include "ecsact/runtime/async.h"

namespace types {

struct callback_info {
	ecsact_event        event;
	ecsact_entity_id    entity_id;
	ecsact_component_id component_id;
};

struct async_error {
	ecsact_async_error                   error;
	std::vector<ecsact_async_request_id> request_ids;
};

struct entity {
	std::optional<ecsact_entity_id> entity_id;
	ecsact_async_request_id         request_id;
};

struct cpp_execution_component {
	ecsact_entity_id    entity_id;
	ecsact_component_id _id;

	std::vector<std::byte> data;
};

struct action_info {
	ecsact_action_id       action_id;
	std::vector<std::byte> serialized_data;
};

struct cpp_execution_options {
	std::vector<cpp_execution_component> adds;
	std::vector<cpp_execution_component> updates;
	std::vector<cpp_execution_component> removes;
	std::vector<action_info>             actions;

	bool has_value() {
		if(adds.size() > 0) {
			return true;
		}
		if(updates.size() > 0) {
			return true;
		}
		if(removes.size() > 0) {
			return true;
		}
		if(actions.size() > 0) {
			return true;
		}
		return false;
	}
};

struct pending_execution_options {
	ecsact_async_request_id      request_id;
	types::cpp_execution_options options;
};

using async_requests =
	std::variant<ecsact_execute_systems_error, async_error, entity>;

} // namespace types
