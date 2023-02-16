#include "execution_callbacks.hh"

using namespace ecsact::async_reference::detail;

execution_callbacks::execution_callbacks() {
	collector.init_callback = &execution_callbacks::init_callback;
	collector.update_callback = &execution_callbacks::update_callback;
	collector.remove_callback = &execution_callbacks::remove_callback;
	collector.entity_created_callback =
		&execution_callbacks::entity_created_callback;
	collector.entity_destroyed_callback =
		&execution_callbacks::entity_destroyed_callback;
	collector.init_callback_user_data = this;
	collector.update_callback_user_data = this;
	collector.remove_callback_user_data = this;
	collector.entity_created_callback_user_data = this;
	collector.entity_destroyed_callback_user_data = this;
}

ecsact_execution_events_collector* execution_callbacks::get_collector() {
	return &collector;
}

void execution_callbacks::invoke(
	const ecsact_execution_events_collector* execution_events,
	ecsact_registry_id                       registry_id
) {
	if(!has_callbacks()) {
		return;
	}

	if(execution_events == nullptr) {
		if(has_callbacks()) {
			std::unique_lock lk(execution_m);
			init_callbacks_info.clear();
			update_callbacks_info.clear();
			remove_callbacks_info.clear();
			create_entity_callbacks_info.clear();
			destroy_entity_callbacks_info.clear();
		}
		return;
	}

	std::vector<types::callback_info> init_callbacks;
	std::vector<types::callback_info> update_callbacks;
	std::vector<types::callback_info> remove_callbacks;

	std::vector<types::entity_callback_info> entity_created_callbacks;
	std::vector<types::entity_callback_info> entity_destroyed_callbacks;

	std::unique_lock lk(execution_m);

	init_callbacks = std::move(init_callbacks_info);
	update_callbacks = std::move(update_callbacks_info);
	remove_callbacks = std::move(remove_callbacks_info);

	entity_created_callbacks = std::move(create_entity_callbacks_info);
	entity_destroyed_callbacks = std::move(destroy_entity_callbacks_info);

	init_callbacks_info.clear();
	update_callbacks_info.clear();
	remove_callbacks_info.clear();

	create_entity_callbacks_info.clear();
	destroy_entity_callbacks_info.clear();

	lk.unlock();

	if(execution_events->init_callback != nullptr) {
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
	}

	if(execution_events->update_callback != nullptr) {
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
	}

	if(execution_events->remove_callback != nullptr) {
		for(auto& component_info : remove_callbacks) {
			for(auto itr = removed_execute_components.begin();
					itr != removed_execute_components.end();) {
				auto& execute_component = *itr;

				if(execute_component.entity_id != component_info.entity_id && execute_component._id != component_info.component_id) {
					++itr;
					continue;
				}

				auto deserialized_component =
					ecsact::deserialize(execute_component._id, execute_component.data);

				execution_events->remove_callback(
					component_info.event,
					component_info.entity_id,
					component_info.component_id,
					deserialized_component.data(),
					execution_events->remove_callback_user_data
				);

				itr = removed_execute_components.erase(itr);
			}
		}
	}

	if(execution_events->entity_created_callback != nullptr) {
		for(auto& info : entity_created_callbacks) {
			execution_events->entity_created_callback(
				info.event,
				info.entity_id,
				info.index,
				execution_events->entity_created_callback_user_data
			);
		}
	}

	if(execution_events->entity_destroyed_callback != nullptr) {
		for(auto& info : entity_destroyed_callbacks) {
			execution_events->entity_destroyed_callback(
				info.event,
				info.entity_id,
				info.index,
				execution_events->entity_destroyed_callback_user_data
			);
		}
	}
}

bool execution_callbacks::has_callbacks() {
	if(init_callbacks_info.size() > 0) {
		return true;
	}

	if(update_callbacks_info.size() > 0) {
		return true;
	}

	if(remove_callbacks_info.size() > 0) {
		return true;
	}

	if(create_entity_callbacks_info.size() > 0) {
		return true;
	}

	if(destroy_entity_callbacks_info.size() > 0) {
		return true;
	}

	return false;
}

void execution_callbacks::init_callback(
	ecsact_event        event,
	ecsact_entity_id    entity_id,
	ecsact_component_id component_id,
	const void*         component_data,
	void*               callback_user_data
) {
	auto self = static_cast<execution_callbacks*>(callback_user_data);

	auto result =
		std::erase_if(self->remove_callbacks_info, [&](auto& remove_cb_info) {
			return remove_cb_info.component_id == component_id &&
				remove_cb_info.entity_id == entity_id;
		});

	std::erase_if(self->update_callbacks_info, [&](auto& update_cb_info) {
		return update_cb_info.component_id == component_id &&
			update_cb_info.entity_id == entity_id;
	});

	if(result > 0) {
		for(int i = 0; i < self->removed_execute_components.size(); ++i) {
			auto& execute_component = self->removed_execute_components[i];
			if(execute_component._id == component_id) {
				self->removed_execute_components.erase(
					self->removed_execute_components.begin() + i
				);
			}
		}
	}

	auto info = detail::types::callback_info{};

	info.event = event;
	info.entity_id = entity_id;
	info.component_id = component_id;
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

	auto info = detail::types::callback_info{};

	info.event = event;
	info.entity_id = entity_id;
	info.component_id = component_id;
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

	std::erase_if(self->init_callbacks_info, [&](auto& init_cb_info) {
		return init_cb_info.component_id == component_id &&
			init_cb_info.entity_id == entity_id;
	});

	std::erase_if(self->update_callbacks_info, [&](auto& update_cb_info) {
		return update_cb_info.component_id == component_id &&
			update_cb_info.entity_id == entity_id;
	});

	auto info = types::callback_info{};
	info.event = event;
	info.entity_id = entity_id;
	info.component_id = component_id;

	auto component_to_serialize = ecsact_component{
		.component_id = component_id,
		.component_data = component_data};

	auto serialized_component = ecsact::serialize(component_to_serialize);

	self->removed_execute_components.push_back(types::cpp_execution_component{
		.entity_id = entity_id,
		._id = component_id,
		.data = serialized_component,
	});

	// Same for remove, I could store data here
	// maybe...?
	self->remove_callbacks_info.push_back(info);
}

void execution_callbacks::entity_created_callback(
	ecsact_event     event,
	ecsact_entity_id entity_id,
	int32_t          index,
	void*            callback_user_data
) {
	auto self = static_cast<execution_callbacks*>(callback_user_data);

	auto info = types::entity_callback_info{};

	info.event = event;
	info.entity_id = entity_id;

	self->create_entity_callbacks_info.push_back(info);
}

void execution_callbacks::entity_destroyed_callback(
	ecsact_event     event,
	ecsact_entity_id entity_id,
	int32_t          index,
	void*            callback_user_data
) {
	auto self = static_cast<execution_callbacks*>(callback_user_data);

	auto info = types::entity_callback_info{};

	info.event = event;
	info.entity_id = entity_id;

	self->destroy_entity_callbacks_info.push_back(info);
}
