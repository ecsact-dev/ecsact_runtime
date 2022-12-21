#pragma once

#include <vector>
#include <mutex>

#include "ecsact/runtime/core.hh"
#include "reference/async_reference/util/types.hh"

class execution_callbacks {
public:
	execution_callbacks();

	void invoke(
		const ecsact_execution_events_collector* execution_events,
		ecsact_registry_id                       registry_id
	);

	ecsact_execution_events_collector* get_collector();

	inline auto lock() -> std::unique_lock<std::mutex> {
		return std::unique_lock(execution_m);
	}

private:
	ecsact_execution_events_collector collector;
	std::mutex                        execution_m;

	std::vector<types::callback_info> init_callbacks_info;
	std::vector<types::callback_info> update_callbacks_info;
	std::vector<types::callback_info> remove_callbacks_info;

	static void init_callback(
		ecsact_event        event,
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data,
		void*               callback_user_data
	);

	static void update_callback(
		ecsact_event        event,
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data,
		void*               callback_user_data
	);

	static void remove_callback(
		ecsact_event        event,
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data,
		void*               callback_user_data
	);
};
