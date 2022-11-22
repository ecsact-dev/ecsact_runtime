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
		is_connected = true;
		execute_systems();
	} else if(connect_str == "bad") {
		async_error.error = ECSACT_ASYNC_ERR_PERMISSION_DENIED;
		async_error.request_id = request_id;
	}

	return static_cast<ecsact_async_request_id>(
		request_id.fetch_add(0, std::memory_order_relaxed)
	);
}

ecsact_async_request_id async_reference::enqueue_execution_options(
	const ecsact_execution_options options
) {
	std::unique_lock lk(execute_m, std::defer_lock);
	// Convert to new cpp execution options

	auto cpp_options = c_to_cpp_execution_options(options);

	if(lk.try_lock()) {
		// This adds the options without considering when it was sent and where
		// it was sent for. Not safe, desync!?
		tick_map.insert(std::pair(tick, cpp_options));
	} else {
		temp_tick_map.insert(std::pair(tick + 1, cpp_options));
	}
	return static_cast<ecsact_async_request_id>(
		request_id.fetch_add(1, std::memory_order_relaxed)
	);
}

void async_reference::execute_systems() {
	execution_thread = std::thread([this] {
		while(is_connected == true) {
			std::unique_lock lk(execute_m);
			auto             cpp_options = tick_map.at(tick);
			lk.unlock();

			ecsact_execution_events_collector collector;
			collector.init_callback = &async_reference::init_callback;
			collector.update_callback = &async_reference::update_callback;
			collector.remove_callback = &async_reference::remove_callback;

			collector.init_callback_user_data = nullptr;
			collector.update_callback_user_data = nullptr;
			collector.remove_callback_user_data = nullptr;

			ecsact_execution_options* options =
				cpp_to_c_execution_options(cpp_options);

			// Consider using span for the options
			auto systems_error =
				ecsact_execute_systems(registry_id, 1, options, &collector);

			if(systems_error == ECSACT_EXEC_SYS_ERR_ACTION_ENTITY_INVALID) {
				sys_error = systems_error;
			}

			if(systems_error == ECSACT_EXEC_SYS_ERR_ACTION_ENTITY_CONSTRAINT_BROKEN) {
				sys_error = systems_error;
			}

			lk.lock();
			tick_map.erase(tick);
			tick++;
			lk.unlock();

			for(auto& [key, val] : temp_tick_map) {
				if(tick_map.contains(key)) {
					auto& tick_options = tick_map.at(key);
					auto& temp_options = temp_tick_map.at(key);
					merge_tick_options(tick_options, temp_options);
				} else {
					tick_map.insert(std::pair(key, val));
				}
			}
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
	lk.unlock();

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
	is_connected = false;
	if(execution_thread.joinable()) {
		execution_thread.join();
	}
}

void async_reference::merge_tick_options(
	cpp_execution_options& tick_options,
	cpp_execution_options& other_options
) {
	if(other_options.actions) {
		tick_options.actions->insert(
			tick_options.actions->end(),
			other_options.actions->begin(),
			other_options.actions->end()
		);
	}

	if(other_options.add_components) {
		tick_options.add_components->insert(
			tick_options.add_components->end(),
			other_options.add_components->begin(),
			other_options.add_components->end()
		);
		tick_options.add_components_entities->insert(
			tick_options.add_components_entities->end(),
			other_options.add_components_entities->begin(),
			other_options.add_components_entities->end()
		);
	}

	if(other_options.update_components) {
		tick_options.update_components->insert(
			tick_options.update_components->end(),
			other_options.update_components->begin(),
			other_options.update_components->end()
		);
		tick_options.update_components_entities->insert(
			tick_options.update_components_entities->end(),
			other_options.update_components_entities->begin(),
			other_options.update_components_entities->end()
		);
	}

	if(other_options.remove_components) {
		tick_options.remove_components->insert(
			tick_options.remove_components->end(),
			other_options.remove_components->begin(),
			other_options.remove_components->end()
		);

		tick_options.remove_components_entities->insert(
			tick_options.remove_components_entities->end(),
			other_options.remove_components_entities->begin(),
			other_options.remove_components_entities->end()
		);
	}
}

cpp_execution_options async_reference::c_to_cpp_execution_options(
	const ecsact_execution_options options
) {
	cpp_execution_options cpp_options;

	if(options.add_components_length > 0) {
		cpp_options.add_components.emplace();
		cpp_options.add_components->insert(
			cpp_options.add_components->end(),
			options.add_components,
			options.add_components + options.add_components_length
		);

		cpp_options.add_components_entities.emplace();
		cpp_options.add_components_entities->insert(
			cpp_options.add_components_entities->end(),
			options.add_components_entities,
			options.add_components_entities + options.add_components_length
		);
	}

	if(options.update_components_length > 0) {
		cpp_options.update_components.emplace();
		cpp_options.update_components->insert(
			cpp_options.update_components->end(),
			options.update_components,
			options.update_components + options.update_components_length
		);

		cpp_options.update_components_entities.emplace();
		cpp_options.update_components_entities->insert(
			cpp_options.update_components_entities->end(),
			options.update_components_entities,
			options.update_components_entities + options.update_components_length
		);
	}

	if(options.remove_components_length > 0) {
		cpp_options.remove_components.emplace();
		cpp_options.remove_components->insert(
			cpp_options.remove_components->end(),
			options.remove_components,
			options.remove_components + options.remove_components_length
		);

		cpp_options.remove_components_entities.emplace();
		cpp_options.remove_components_entities->insert(
			cpp_options.remove_components_entities->end(),
			options.remove_components_entities,
			options.remove_components_entities + options.remove_components_length
		);
	}

	if(options.actions_length > 0) {
		cpp_options.actions.emplace();
		cpp_options.actions->insert(
			cpp_options.actions->end(),
			options.actions,
			options.actions + options.actions_length
		);
	}
	return cpp_options;
}

ecsact_execution_options* async_reference::cpp_to_c_execution_options(
	cpp_execution_options options
) {
	ecsact_execution_options* c_options = new ecsact_execution_options;

	if(options.actions) {
		c_options->actions = options.actions->data();
		c_options->actions_length = options.actions->size();
	}
	if(options.add_components) {
		c_options->add_components = options.add_components->data();
		c_options->add_components_entities =
			options.add_components_entities->data();
		c_options->add_components_length = options.add_components->size();
	}
	if(options.update_components) {
		c_options->update_components = options.update_components->data();
		c_options->update_components_entities =
			options.update_components_entities->data();
		c_options->update_components_length = options.update_components->size();
	}
	if(options.remove_components) {
		c_options->remove_components = options.remove_components->data();
		c_options->remove_components_entities =
			options.remove_components_entities->data();
		c_options->remove_components_length = options.remove_components->size();
	}
	return c_options;
}
