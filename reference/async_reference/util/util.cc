#include "util.hh"

#include "ecsact/runtime/serialize.hh"

ecsact_async_error validate_instructions(
	std::vector<types::cpp_execution_component>& components
) {
	auto components_range = std::ranges::views::all(components);
	auto entities = util::get_cpp_entities(components_range);
	auto has_duplicates = util::check_entity_duplicates(entities);

	if(!has_duplicates) {
		return ECSACT_ASYNC_OK;
	}

	for(auto& component : components) {
		auto view = std::views::filter(
			components_range,
			[&components_range,
			 &component](types::cpp_execution_component other_component) {
				return component._id == other_component._id;
			}
		);
		int same_component_count = 0;
		int same_entity_count = 0;
		for(auto itr = view.begin(); itr != view.end(); itr++) {
			same_component_count++;
			if(component.entity_id == itr->entity_id) {
				same_entity_count++;
			}
			if(same_component_count > 1 && same_entity_count > 1) {
				return ECSACT_ASYNC_ERR_EXECUTION_MERGE_FAILURE;
				break;
			}
		}
	}
	return ECSACT_ASYNC_OK;
}

ecsact_async_error validate_merge_instructions(
	std::vector<types::cpp_execution_component>& components,
	std::vector<types::cpp_execution_component>& other_components
) {
	if(other_components.size() > 0) {
		auto components_range = std::ranges::views::all(components);
		auto entities = util::get_cpp_entities(components_range);

		auto other_components_range = std::ranges::views::all(other_components);
		auto other_entities = util::get_cpp_entities(other_components_range);

		auto empty_view = std::ranges::empty_view<int>{};

		auto has_duplicates =
			util::check_entity_merge_duplicates(entities, other_entities);

		if(!has_duplicates) {
			return ECSACT_ASYNC_OK;
		}

		for(auto& component : components) {
			auto view = std::views::filter(
				other_components_range,
				[&other_components_range,
				 &component](types::cpp_execution_component other_component) {
					return component._id == other_component._id;
				}
			);
			int same_component_count = 0;
			int same_entity_count = 0;
			for(auto itr = view.begin(); itr != view.end(); itr++) {
				same_component_count++;
				if(component.entity_id == itr->entity_id) {
					same_entity_count++;
				}
				if(same_component_count > 0 && same_entity_count > 0) {
					return ECSACT_ASYNC_ERR_EXECUTION_MERGE_FAILURE;
					break;
				}
			}
		}
	}

	return ECSACT_ASYNC_OK;
}

ecsact_execution_options util::cpp_to_c_execution_options(
	types::cpp_execution_options options,
	const ecsact_registry_id&    registry_id,
	const ecsact_registry_id&    pending_registry_id
) {
	ecsact_execution_options c_options{};

	if(options.actions.size() > 0) {
		std::vector<ecsact_action> deserialized_actions;
		deserialized_actions.resize(options.actions.size());
		for(auto& action_info : options.actions) {
			ecsact_action action;

			ecsact_deserialize_action(
				action_info.action_id,
				reinterpret_cast<uint8_t*>(action_info.serialized_data.data()),
				&action
			);

			deserialized_actions.insert(deserialized_actions.end(), action);
		}

		c_options.actions = deserialized_actions.data();
		c_options.actions_length = deserialized_actions.size();
	}
	if(options.adds.size() > 0) {
		auto adds_range = std::ranges::views::all(options.adds);
		auto entities = util::get_cpp_entities(adds_range);

		std::vector<ecsact_entity_id> entity_list;
		entity_list.insert(entity_list.end(), entities.begin(), entities.end());

		std::vector<ecsact_component> component_list;

		for(int i = 0; i < options.adds.size(); i++) {
			ecsact_component component;
			auto             component_info = options.adds[i];

			const void* component_data = ecsact_get_component(
				pending_registry_id,
				component_info.entity_id,
				component_info._id
			);

			ecsact_add_component(
				registry_id,
				component_info.entity_id,
				component_info._id,
				component_data
			);

			ecsact_remove_component(
				pending_registry_id,
				component_info.entity_id,
				component_info._id
			);

			component.component_id = component_info._id;
			component.component_data = component_data;
			component_list.insert(component_list.end(), component);
		}

		c_options.add_components = component_list.data();
		c_options.add_components_entities = entity_list.data();
		c_options.add_components_length = options.adds.size();
	}
	if(options.updates.size() > 0) {
		auto updates_range = std::ranges::views::all(options.updates);

		std::vector<ecsact_entity_id> entity_list;
		auto entities = util::get_cpp_entities(updates_range);
		entity_list.insert(entity_list.end(), entities.begin(), entities.end());

		std::vector<ecsact_component> component_list;

		for(int i = 0; i < options.updates.size(); i++) {
			ecsact_component component;
			auto             component_info = options.updates[i];
			const void*      component_data = ecsact_get_component(
        registry_id,
        component_info.entity_id,
        component_info._id
      );

			auto pending_component = ecsact_get_component(
				pending_registry_id,
				component_info.entity_id,
				component_info._id
			);

			ecsact_update_component(
				registry_id,
				component_info.entity_id,
				component_info._id,
				pending_component
			);

			ecsact_remove_component(
				pending_registry_id,
				component_info.entity_id,
				component_info._id
			);

			component.component_id = component_info._id;
			component.component_data = component_data;
			component_list.insert(component_list.end(), component);
		}

		c_options.add_components = component_list.data();
		c_options.add_components_entities = entity_list.data();
		c_options.add_components_length = options.updates.size();
	}
	if(options.removes.size() > 0) {
		auto removes_range = std::ranges::views::all(options.removes);

		auto entities = util::get_cpp_entities(removes_range);
		std::vector<ecsact_entity_id> entity_list;
		entity_list.insert(entity_list.end(), entities.begin(), entities.end());

		auto component_ids = util::get_cpp_component_ids(removes_range);
		std::vector<ecsact_component_id> component_list;
		component_list
			.insert(component_list.end(), component_ids.begin(), component_ids.end());

		c_options.remove_components = component_list.data();
		c_options.remove_components_entities = entity_list.data();
		c_options.remove_components_length = options.removes.size();
	}
	return c_options;
}

types::cpp_execution_options util::c_to_cpp_execution_options(
	const ecsact_execution_options options,
	const ecsact_registry_id&      pending_registry_id
) {
	types::cpp_execution_options cpp_options;

	// Check if there's duplicates

	if(options.add_components != nullptr) {
		for(int i = 0; i < options.add_components_length; i++) {
			types::cpp_execution_component add_component;

			auto component = options.add_components[i];
			auto entity_id = options.add_components_entities[i];

			ecsact_ensure_entity(pending_registry_id, entity_id);

			ecsact_add_component(
				pending_registry_id,
				entity_id,
				component.component_id,
				component.component_data
			);

			add_component._id = component.component_id;
			add_component.entity_id = entity_id;
			cpp_options.adds.insert(cpp_options.adds.end(), add_component);
		}
	}

	if(options.update_components != nullptr) {
		for(int i = 0; i < options.update_components_length; i++) {
			types::cpp_execution_component update_component;

			auto component = options.update_components[i];
			auto entity_id = options.update_components_entities[i];

			update_component._id = component.component_id;
			update_component.entity_id = entity_id;

			// ecsact_ensure_entity(pending_registry_id, entity_id);

			bool has_component = ecsact_has_component(
				pending_registry_id,
				entity_id,
				component.component_id
			);

			if(has_component) {
				ecsact_update_component(
					pending_registry_id,
					entity_id,
					component.component_id,
					component.component_data
				);
			} else {
				ecsact_add_component(
					pending_registry_id,
					entity_id,
					component.component_id,
					component.component_data
				);
			}

			cpp_options.updates.insert(cpp_options.updates.end(), update_component);
		}
	}

	if(options.remove_components != nullptr) {
		for(int i = 0; i < options.remove_components_length; i++) {
			types::cpp_execution_component remove_component;

			auto component_id = options.remove_components[i];
			auto entity_id = options.remove_components_entities[i];

			bool has_component =
				ecsact_has_component(pending_registry_id, entity_id, component_id);

			if(has_component) {
			}

			remove_component._id = component_id;
			remove_component.entity_id = entity_id;
			cpp_options.removes.insert(cpp_options.removes.end(), remove_component);
		}
	}

	if(options.actions != nullptr) {
		for(int i = 0; i < options.actions_length; i++) {
			auto action = options.actions[i];

			types::action_info action_info;
			action_info.action_id = action.action_id;

			std::vector<std::byte> out_action;
			ecsact_serialize_action(
				action.action_id,
				action.action_data,
				reinterpret_cast<uint8_t*>(out_action.data())
			);

			action_info.serialized_data = out_action;

			cpp_options.actions.insert(cpp_options.actions.begin(), action_info);
		}
	}
	return cpp_options;
}

ecsact_async_error util::validate_options(types::cpp_execution_options& options
) {
	ecsact_async_error error = ECSACT_ASYNC_OK;

	if(options.adds.size() > 0) {
		// NOTE: There's currently no tolerance for 2 users adding a component to an
		// entity on the same tick
		error = validate_instructions(options.adds);
	}

	if(error != ECSACT_ASYNC_OK) {
		return error;
	}

	if(options.updates.size() > 0) {
		// NOTE: There's currently no tolerance for updates on the same tick
		validate_instructions(options.updates);
	}

	if(error != ECSACT_ASYNC_OK) {
		return error;
	}

	if(options.removes.size() > 0) {
		validate_instructions(options.removes);
	}

	return error;
}

ecsact_async_error util::validate_merge_options(
	types::cpp_execution_options& options,
	types::cpp_execution_options& other_options
) {
	ecsact_async_error error = ECSACT_ASYNC_OK;
	if(other_options.adds.size() > 0) {
		validate_merge_instructions(options.adds, other_options.adds);
	}

	if(error != ECSACT_ASYNC_OK) {
		return error;
	}

	if(options.updates.size() > 0) {
		validate_merge_instructions(options.updates, other_options.updates);
	}

	if(error != ECSACT_ASYNC_OK) {
		return error;
	}

	if(options.removes.size() > 0) {
		validate_merge_instructions(options.removes, other_options.removes);
	}

	return error;
}

void util::merge_options(
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

void util::merge_options(
	types::pending_execution_options& pending,
	types::pending_execution_options& other_pending
) {
	if(other_pending.options.actions.size() > 0) {
		pending.options.actions.insert(
			pending.options.actions.end(),
			other_pending.options.actions.begin(),
			other_pending.options.actions.end()
		);
	}

	if(other_pending.options.adds.size() > 0) {
		pending.options.adds.insert(
			pending.options.adds.end(),
			other_pending.options.adds.begin(),
			other_pending.options.adds.end()
		);
	}

	if(other_pending.options.updates.size() > 0) {
		pending.options.updates.insert(
			pending.options.updates.end(),
			other_pending.options.updates.begin(),
			other_pending.options.updates.end()
		);
	}

	if(other_pending.options.removes.size() > 0) {
		pending.options.removes.insert(
			pending.options.removes.end(),
			other_pending.options.removes.begin(),
			other_pending.options.removes.end()
		);
	}
}
