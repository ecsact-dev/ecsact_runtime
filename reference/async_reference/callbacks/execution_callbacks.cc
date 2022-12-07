#include "execution_callbacks.hh"

std::vector<types::callback_info> execution_callbacks::init_callbacks_info = {};
std::vector<types::callback_info> execution_callbacks::update_callbacks_info =
	{};
std::vector<types::callback_info> execution_callbacks::remove_callbacks_info =
	{};

std::mutex execution_callbacks::execution_m = {};

void execution_callbacks::invoke(
	const ecsact_execution_events_collector& execution_events,
	ecsact_registry_id                       registry_id
) {
	std::vector<types::callback_info> init_callbacks;
	std::vector<types::callback_info> update_callbacks;
	std::vector<types::callback_info> remove_callbacks;

	std::unique_lock lk(execution_m);

	init_callbacks = std::move(init_callbacks_info);
	update_callbacks = std::move(update_callbacks_info);
	remove_callbacks = std::move(remove_callbacks_info);

	init_callbacks_info.clear();
	update_callbacks_info.clear();
	remove_callbacks_info.clear();

	lk.unlock();

	for(auto& component_info : init_callbacks) {
		const void* component_data = ecsact_get_component(
			registry_id,
			component_info.entity_id,
			component_info.component_id
		);

		execution_events.init_callback(
			component_info.event,
			component_info.entity_id,
			component_info.component_id,
			component_data,
			execution_events.init_callback_user_data
		);
	}

	for(auto& component_info : update_callbacks) {
		const void* component_data = ecsact_get_component(
			registry_id,
			component_info.entity_id,
			component_info.component_id
		);

		execution_events.update_callback(
			component_info.event,
			component_info.entity_id,
			component_info.component_id,
			component_data,
			execution_events.update_callback_user_data
		);
	}

	for(auto& component_info : remove_callbacks) {
		const void* component_data = ecsact_get_component(
			registry_id,
			component_info.entity_id,
			component_info.component_id
		);

		execution_events.remove_callback(
			component_info.event,
			component_info.entity_id,
			component_info.component_id,
			component_data,
			execution_events.remove_callback_user_data
		);
	}
}

void execution_callbacks::init_callback(
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
	std::unique_lock lk(execution_m);
	init_callbacks_info.insert(init_callbacks_info.end(), info);
}

void execution_callbacks::update_callback(
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
	std::unique_lock lk(execution_m);
	update_callbacks_info.insert(init_callbacks_info.end(), info);
}

void execution_callbacks::remove_callback(
	ecsact_event        event,
	ecsact_entity_id    entity_id,
	ecsact_component_id component_id,
	const void*         component_data,
	void*               callback_user_data
) {
	types::callback_info info;

	std::unique_lock lk(execution_m);
	for(int i = 0; i < init_callbacks_info.size(); i++) {
		if(init_callbacks_info[i].component_id == component_id) {
			init_callbacks_info.erase(init_callbacks_info.begin() + i);
			break;
		}
	}
	lk.unlock();

	info.event = event;
	info.entity_id = entity_id;
	info.component_id = component_id;
	lk.lock();
	remove_callbacks_info.insert(init_callbacks_info.end(), info);
}
