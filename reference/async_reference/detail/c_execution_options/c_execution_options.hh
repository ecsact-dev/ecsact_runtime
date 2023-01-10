#pragma once

#include <vector>
#include <optional>

#include "ecsact/runtime/core.h"
#include "ecsact/runtime/async.h"

namespace detail {

template<typename T>
struct data_info {
	T id;

	std::vector<std::byte> data;
};

struct c_execution_options {
	/**
	 * Holds the data lifetime used to reconstruct components and actions
	 * Do NOT use this, it's an implementation detail
	 */
	std::vector<data_info<ecsact_action_id>>    actions_info;
	std::vector<data_info<ecsact_component_id>> adds_info;
	std::vector<ecsact_entity_id>               adds_entities;
	std::vector<data_info<ecsact_component_id>> updates_info;
	std::vector<ecsact_entity_id>               updates_entities;
	std::vector<ecsact_component_id>            remove_ids;
	std::vector<ecsact_entity_id>               removes_entities;

	ecsact_execution_options c();

private:
	ecsact_execution_options options;

	std::vector<ecsact_action>    actions;
	std::vector<ecsact_component> adds;
	std::vector<ecsact_component> updates;
};
} // namespace detail
