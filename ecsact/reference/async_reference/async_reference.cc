#include <span>

#include "async_reference.hh"

std::vector<callback_info> async_reference::init_callbacks_info = {};
std::vector<callback_info> async_reference::update_callbacks_info = {};
std::vector<callback_info> async_reference::remove_callbacks_info = {};

ecsact_async_request_id async_reference::connect(const char* connection_string
) {
	std::string connect_str(connection_string);
	request_id += 1;

	if(connect_str == "good") {
		execute_systems();
	} else if(connect_str == "bad") {
		async_error.error = ECSACT_ASYNC_ERR_PERMISSION_DENIED;
		async_error.request_id = request_id;
	}

	return static_cast<ecsact_async_request_id>(request_id);
}

ecsact_async_request_id async_reference::enqueue_execution_options(
	const ecsact_execution_options options
) {
	// Check if tick is active?
	// Insert at that point

	request_id += 1;
	return static_cast<ecsact_async_request_id>(request_id);
}

void async_reference::execute_systems() {
	execution_thread = std::thread([this] {
		for(;;) {
			std::unique_lock lk(execute_m);
			// If tick_is_busy()
			// add to the following tick
			auto options = tick_map.at(tick);

			ecsact_execution_events_collector collector;
			collector.init_callback = &async_reference::init_callback;
			collector.update_callback = &async_reference::update_callback;
			collector.remove_callback = &async_reference::remove_callback;

			collector.init_callback_user_data = nullptr;
			collector.update_callback_user_data = nullptr;
			collector.remove_callback_user_data = nullptr;

			// Consider using span for the options
			auto systems_error =
				ecsact_execute_systems(registry_id, 1, &options, &collector);

			if(systems_error == ECSACT_EXEC_SYS_ERR_ACTION_ENTITY_INVALID) {
				sys_error = systems_error;
			}

			if(systems_error == ECSACT_EXEC_SYS_ERR_ACTION_ENTITY_CONSTRAINT_BROKEN) {
				sys_error = systems_error;
			}

			tick_map.erase(tick);
			tick++;
		}
	});
}

void async_reference::flush_events(
	const ecsact_execution_events_collector* execution_events,
	const ecsact_async_events_collector*     async_events
) {
	std::unique_lock lk(execute_m);

	if(execution_events != nullptr) {
		for(int i = 0; i < init_callbacks_info.size(); i++) {
			auto component_info = init_callbacks_info[i];

			const void* component_data = ecsact_get_component(
				registry_id,
				component_info.entity_id,
				component_info.component_id
			);

			execution_events->init_callback(
				component_info.event,
				component_info.entity_id,
				component_info.component_id,
				component_data,
				execution_events->init_callback_user_data
			);
		}

		for(int i = 0; i < update_callbacks_info.size(); i++) {
			auto component_info = init_callbacks_info[i];

			const void* component_data = ecsact_get_component(
				registry_id,
				component_info.entity_id,
				component_info.component_id
			);

			execution_events->update_callback(
				component_info.event,
				component_info.entity_id,
				component_info.component_id,
				component_data,
				execution_events->update_callback_user_data
			);
		}

		for(int i = 0; i < remove_callbacks_info.size(); i++) {
			auto component_info = init_callbacks_info[i];

			const void* component_data = ecsact_get_component(
				registry_id,
				component_info.entity_id,
				component_info.component_id
			);

			execution_events->remove_callback(
				component_info.event,
				component_info.entity_id,
				component_info.component_id,
				component_data,
				execution_events->remove_callback_user_data
			);
		}
	}
	init_callbacks_info.clear();
	update_callbacks_info.clear();
	remove_callbacks_info.clear();

	if(async_error.error != ECSACT_ASYNC_OK) {
		async_events->async_error_callback(
			async_error.error,
			static_cast<ecsact_async_request_id>(async_error.request_id),
			async_events->async_error_callback_user_data
		);
		disconnect();
	}

	if(sys_error != ECSACT_EXEC_SYS_OK) {
		async_events->system_error_callback(
			sys_error,
			async_events->async_error_callback_user_data
		);
		disconnect();
	}
}

void async_reference::init_callback(
	ecsact_event        event,
	ecsact_entity_id    entity_id,
	ecsact_component_id component_id,
	const void*         component_data,
	void*               callback_user_data
) {
	callback_info info;

	info.event = event;
	info.entity_id = entity_id;
	info.component_id = component_id;
	init_callbacks_info.insert(init_callbacks_info.end(), info);
}

void async_reference::update_callback(
	ecsact_event        event,
	ecsact_entity_id    entity_id,
	ecsact_component_id component_id,
	const void*         component_data,
	void*               callback_user_data
) {
	callback_info info;

	info.event = event;
	info.entity_id = entity_id;
	info.component_id = component_id;
	update_callbacks_info.insert(init_callbacks_info.end(), info);
}

void async_reference::remove_callback(
	ecsact_event        event,
	ecsact_entity_id    entity_id,
	ecsact_component_id component_id,
	const void*         component_data,
	void*               callback_user_data
) {
	callback_info info;

	for(int i = 0; i < init_callbacks_info.size(); i++) {
		if(init_callbacks_info[i].component_id == component_id) {
			// Remove init if remove is occurring before flush
			init_callbacks_info.erase(init_callbacks_info.begin() + i);
			break;
		}
	}

	info.event = event;
	info.entity_id = entity_id;
	info.component_id = component_id;
	remove_callbacks_info.insert(init_callbacks_info.end(), info);
}

void async_reference::disconnect() {
	execution_thread.join();
}
