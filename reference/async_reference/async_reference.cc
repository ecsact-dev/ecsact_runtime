#include <span>
#include <memory>
#include <ranges>
#include <algorithm>
#include <iterator>

#include "async_reference.hh"

std::vector<types::callback_info> async_reference::init_callbacks_info = {};
std::vector<types::callback_info> async_reference::update_callbacks_info = {};
std::vector<types::callback_info> async_reference::remove_callbacks_info = {};

ecsact_async_request_id async_reference::connect(const char* connection_string
) {
	std::string connect_str(connection_string);

	if(connect_str == "good") {
		is_connected = true;
		registry_id = ecsact_create_registry("async_reference_impl_reg");
		execute_systems();
	} else if(connect_str == "bad") {
		async_error.error = ECSACT_ASYNC_ERR_PERMISSION_DENIED;
		async_error.request_id = request_id;
	}

	return static_cast<ecsact_async_request_id>(
		request_id.fetch_add(1, std::memory_order_relaxed)
	);
}

ecsact_async_request_id async_reference::enqueue_execution_options(
	const ecsact_execution_options& options
) {
	if(is_connected == false) {
		// Need to return a request ID?
	}

	if(async_error.error == ECSACT_ASYNC_OK) {
		std::unique_lock lk(execute_m, std::defer_lock);

		auto cpp_options = util::c_to_cpp_execution_options(options);
		async_error.error = util::validate_options(cpp_options);

		if(async_error.error != ECSACT_ASYNC_OK) {
			increment_request_id();
			async_error.request_id = request_id;
			disconnect();
			return convert_request_id(async_error.request_id);
		}

		if(lk.try_lock()) {
			if(!tick_map.contains(tick)) {
				tick_map.insert(std::pair(tick, cpp_options));
			} else {
				auto existing_options = tick_map.at(tick);
				async_error.error =
					util::validate_merge_options(existing_options, cpp_options);
				if(async_error.error == ECSACT_ASYNC_OK) {
					merge_tick_options(existing_options, cpp_options);
				} else {
					increment_request_id();
					async_error.request_id = request_id;
					return convert_request_id(async_error.request_id);
				}
			}
		} else {
			temp_tick_map.insert(std::pair(tick + 1, cpp_options));
		}
		return increment_request_id();
	} else {
		return convert_request_id(async_error.request_id);
	}
}

void async_reference::execute_systems() {
	execution_thread = std::thread([this] {
		while(is_connected == true) {
			std::unique_lock                            lk(execute_m);
			std::optional<types::cpp_execution_options> cpp_options;
			if(tick_map.contains(tick)) {
				cpp_options = tick_map.at(tick);
			}

			lk.unlock();

			ecsact_execution_events_collector collector;
			collector.init_callback = &async_reference::init_callback;
			collector.update_callback = &async_reference::update_callback;
			collector.remove_callback = &async_reference::remove_callback;

			collector.init_callback_user_data = nullptr;
			collector.update_callback_user_data = nullptr;
			collector.remove_callback_user_data = nullptr;

			std::unique_ptr<ecsact_execution_options> options = nullptr;

			if(cpp_options) {
				options = std::make_unique<ecsact_execution_options>(
					util::cpp_to_c_execution_options(*cpp_options, *registry_id)
				);
			}

			auto systems_error =
				ecsact_execute_systems(*registry_id, 1, options.get(), &collector);

			if(systems_error != ECSACT_EXEC_SYS_OK) {
				sys_error = systems_error;
				disconnect();
				return;
			}

			lk.lock();
			tick_map.erase(tick);
			tick++;

			// NOTE(Kelwan): NOT THREAD SAFE RIGHT NOW
			for(auto& [key, val] : temp_tick_map) {
				if(tick_map.contains(key)) {
					auto& tick_options = tick_map.at(key);
					auto& temp_options = temp_tick_map.at(key);
					merge_tick_options(tick_options, temp_options);
					temp_tick_map.erase(key);
				} else {
					tick_map.insert(std::pair(key, val));
				}
			}
			lk.unlock();

			std::unique_lock entity_lk(entity_m);
			for(int i = 0; i < pending_entity_requests.size(); i++) {
				auto entity_request_id = pending_entity_requests[i];

				auto entity = ecsact_create_entity(*registry_id);

				types::notified_entities notified_entity{
					.request_id = entity_request_id,
					.entity_id = entity};

				created_entities.insert(created_entities.end(), notified_entity);
			}
			pending_entity_requests.clear();
		}
	});
}

void async_reference::flush_events(
	const ecsact_execution_events_collector* execution_events,
	const ecsact_async_events_collector*     async_events
) {
	if(async_error.error != ECSACT_ASYNC_OK || sys_error != ECSACT_EXEC_SYS_OK) {
		if(async_events == nullptr) {
			disconnect();
			return;
		}

		if(async_events->async_error_callback != nullptr) {
			async_events->async_error_callback(
				async_error.error,
				static_cast<ecsact_async_request_id>(async_error.request_id),
				async_events->async_error_callback_user_data
			);
		}
		if(async_events->system_error_callback != nullptr) {
			async_events->system_error_callback(
				sys_error,
				async_events->async_error_callback_user_data
			);
		}
		disconnect();
		return;
	}

	std::unique_lock lk(execute_m);
	if(execution_events != nullptr) {
		for(int i = 0; i < init_callbacks_info.size(); i++) {
			auto component_info = init_callbacks_info[i];

			const void* component_data = ecsact_get_component(
				*registry_id,
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
			auto component_info = update_callbacks_info[i];

			const void* component_data = ecsact_get_component(
				*registry_id,
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
			auto component_info = remove_callbacks_info[i];

			const void* component_data = ecsact_get_component(
				*registry_id,
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

	std::unique_lock entity_lk(entity_m);
	for(auto& created_entity : created_entities) {
		if(async_events->async_entity_callback != nullptr) {
			async_events->async_entity_callback(
				created_entity.entity_id,
				created_entity.request_id,
				async_events->async_entity_error_callback_user_data
			);
		}
	}
	created_entities.clear();
}

ecsact_async_request_id async_reference::create_entity_request() {
	increment_request_id();
	std::unique_lock lk(entity_m);
	pending_entity_requests.insert(
		pending_entity_requests.end(),
		get_request_id()
	);
	return get_request_id();
}

void async_reference::init_callback(
	ecsact_event        event,
	ecsact_entity_id    entity_id,
	ecsact_component_id component_id,
	const void*         component_data,
	void*               callback_user_data
) {
	types::callback_info info;

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
	types::callback_info info;

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
	types::callback_info info;

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
	types::cpp_execution_options& tick_options,
	types::cpp_execution_options& other_options
) {
	if(other_options.actions.size() > 0) {
		tick_options.actions.insert(
			tick_options.actions.end(),
			other_options.actions.begin(),
			other_options.actions.end()
		);
	}

	auto range = std::ranges::views::all(tick_options.actions);
	// NOTE(Kelwan) Still need to confirm the merge is safe
	if(other_options.adds.size() > 0) {
		tick_options.adds.insert(
			tick_options.adds.end(),
			other_options.adds.begin(),
			other_options.adds.end()
		);
	}

	if(other_options.updates.size() > 0) {
		tick_options.updates.insert(
			tick_options.updates.end(),
			other_options.updates.begin(),
			other_options.updates.end()
		);
	}

	if(other_options.removes.size() > 0) {
		tick_options.removes.insert(
			tick_options.removes.end(),
			other_options.removes.begin(),
			other_options.removes.end()
		);
	}
}

ecsact_async_request_id async_reference::increment_request_id() {
	return static_cast<ecsact_async_request_id>(
		request_id.fetch_add(1, std::memory_order_relaxed)
	);
}

ecsact_async_request_id async_reference::get_request_id() {
	return static_cast<ecsact_async_request_id>(
		request_id.fetch_add(0, std::memory_order_relaxed)
	);
}

ecsact_async_request_id async_reference::convert_request_id(int32_t id) {
	return static_cast<ecsact_async_request_id>(id);
}
