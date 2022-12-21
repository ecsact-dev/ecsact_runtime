#include "execution_callbacks.hh"

execution_callbacks::execution_callbacks() {
	collector.init_callback = &execution_callbacks::init_callback;
	collector.update_callback = &execution_callbacks::update_callback;
	collector.remove_callback = &execution_callbacks::remove_callback;
	collector.init_callback_user_data = this;
	collector.update_callback_user_data = this;
	collector.remove_callback_user_data = this;
}

ecsact_execution_events_collector* execution_callbacks::get_collector() {
	return &collector;
}

void execution_callbacks::invoke(
	const ecsact_execution_events_collector* execution_events,
	ecsact_registry_id                       registry_id
) {
	if(execution_events == nullptr) {
		std::unique_lock lk(execution_m);
		init_callbacks_info.clear();
		update_callbacks_info.clear();
		remove_callbacks_info.clear();
		return;
	}

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

		execution_events->init_callback(
			component_info.event,
			component_info.entity_id,
			component_info.component_id,
			component_data,
			execution_events->init_callback_user_data
		);
	}

	for(auto& component_info : update_callbacks) {
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

	for(auto& component_info : remove_callbacks) {
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

void execution_callbacks::init_callback(
	ecsact_event        event,
	ecsact_entity_id    entity_id,
	ecsact_component_id component_id,
	const void*         component_data,
	void*               callback_user_data
) {
	auto self = static_cast<execution_callbacks*>(callback_user_data);
	auto info = types::callback_info{};

	info.event = event;
	info.entity_id = entity_id;
	info.component_id = component_id;
	std::unique_lock lk(self->execution_m);
	self->init_callbacks_info.push_back(info);
}

void execution_callbacks::update_callback(
	ecsact_event        event,
	ecsact_entity_id    entity_id,
	ecsact_component_id component_id,
	const void*         component_data,
	void*               callback_user_data
) {
	auto self = static_cast<execution_callbacks*>(callback_user_data);
	auto info = types::callback_info{};

	info.event = event;
	info.entity_id = entity_id;
	info.component_id = component_id;
	std::unique_lock lk(self->execution_m);
	self->update_callbacks_info.push_back(info);
}

void execution_callbacks::remove_callback(
	ecsact_event        event,
	ecsact_entity_id    entity_id,
	ecsact_component_id component_id,
	const void*         component_data,
	void*               callback_user_data
) {
	auto self = static_cast<execution_callbacks*>(callback_user_data);

	std::unique_lock lk(self->execution_m);
	std::erase_if(self->init_callbacks_info, [&](auto& init_cb_info) {
		return init_cb_info.component_id == component_id;
	});
	lk.unlock();

	auto info = types::callback_info{};
	info.event = event;
	info.entity_id = entity_id;
	info.component_id = component_id;
	lk.lock();
	self->remove_callbacks_info.push_back(info);
}
