#include <memory>

#include "util.hh"

#include "ecsact/runtime/serialize.hh"

ecsact_async_error validate_instructions(
	const std::vector<types::cpp_execution_component>& components
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
	const std::vector<types::cpp_execution_component>& components,
	const std::vector<types::cpp_execution_component>& other_components
) {
	if(components.empty() || other_components.empty()) {
		return ECSACT_ASYNC_OK;
	}

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

	return ECSACT_ASYNC_OK;
}

ecsact_execution_options util::cpp_to_c_execution_options(
	types::cpp_execution_options& options,
	const ecsact_registry_id&     registry_id
) {
	auto c_options = ecsact_execution_options{};

	if(options.actions.size() > 0) {
		std::vector<ecsact_action> deserialized_actions;
		deserialized_actions.resize(options.actions.size());
		for(auto& action_info : options.actions) {
			auto action =
				ecsact::deserialize(action_info.action_id, action_info.serialized_data);

			deserialized_actions.push_back(action);
		}

		c_options.actions = deserialized_actions.data();
		c_options.actions_length = deserialized_actions.size();
	}
	if(options.adds.size() > 0) {
		auto adds_range = std::ranges::views::all(options.adds);
		auto entities_view = util::get_cpp_entities(adds_range);

		std::vector entities(entities_view.begin(), entities_view.end());

		std::vector<ecsact_component> component_list;

		for(int i = 0; i < options.adds.size(); i++) {
			auto& component_info = options.adds[i];

			auto component =
				ecsact::deserialize(component_info._id, component_info.data);

			component_list.push_back(component);
		}

		c_options.add_components = component_list.data();
		c_options.add_components_entities = entities.data();
		c_options.add_components_length = options.adds.size();
	}
	if(options.updates.size() > 0) {
		auto updates_range = std::ranges::views::all(options.updates);
		auto entities_view = util::get_cpp_entities(updates_range);

		std::vector entities(entities_view.begin(), entities_view.end());

		std::vector<ecsact_component> component_list;

		for(int i = 0; i < options.updates.size(); i++) {
			auto component_info = options.updates[i];

			auto component =
				ecsact::deserialize(component_info._id, component_info.data);

			component_list.push_back(component);
		}

		c_options.add_components = component_list.data();
		c_options.add_components_entities = entities.data();
		c_options.add_components_length = options.updates.size();
	}
	if(options.removes.size() > 0) {
		auto removes_range = std::ranges::views::all(options.removes);

		auto        entities_view = util::get_cpp_entities(removes_range);
		std::vector entities(entities_view.begin(), entities_view.end());

		auto component_ids = util::get_cpp_component_ids(removes_range);
		std::vector<ecsact_component_id> component_list;
		component_list
			.insert(component_list.end(), component_ids.begin(), component_ids.end());

		c_options.remove_components = component_list.data();
		c_options.remove_components_entities = entities.data();
		c_options.remove_components_length = options.removes.size();
	}
	return c_options;
}

types::cpp_execution_options util::c_to_cpp_execution_options(
	const ecsact_execution_options options
) {
	auto cpp_options = types::cpp_execution_options{};

	if(options.add_components != nullptr) {
		for(int i = 0; i < options.add_components_length; i++) {
			auto add_component = types::cpp_execution_component{};

			auto component = options.add_components[i];
			auto entity_id = options.add_components_entities[i];

			auto serialized_component = ecsact::serialize(component);

			add_component._id = component.component_id;
			add_component.entity_id = entity_id;
			add_component.data = serialized_component;

			cpp_options.adds.push_back(add_component);
		}
	}

	if(options.update_components != nullptr) {
		for(int i = 0; i < options.update_components_length; i++) {
			auto update_component = types::cpp_execution_component{};

			auto component = options.update_components[i];
			auto entity_id = options.update_components_entities[i];

			update_component._id = component.component_id;
			update_component.entity_id = entity_id;

			auto serialized_component = ecsact::serialize(component);

			update_component.data = serialized_component;

			cpp_options.updates.push_back(update_component);
		}
	}

	if(options.remove_components != nullptr) {
		for(int i = 0; i < options.remove_components_length; i++) {
			auto remove_component = types::cpp_execution_component{};

			auto component_id = options.remove_components[i];
			auto entity_id = options.remove_components_entities[i];

			remove_component._id = component_id;
			remove_component.entity_id = entity_id;
			cpp_options.removes.push_back(remove_component);
		}
	}

	if(options.actions != nullptr) {
		for(int i = 0; i < options.actions_length; i++) {
			auto action = options.actions[i];

			auto action_info = types::action_info{};
			action_info.action_id = action.action_id;

			auto serialized_action = ecsact::serialize(action);

			action_info.serialized_data = serialized_action;

			cpp_options.actions.push_back(action_info);
		}
	}
	return cpp_options;
}

ecsact_async_error util::validate_options(types::cpp_execution_options& options
) {
	ecsact_async_error error = ECSACT_ASYNC_OK;

	if(options.adds.size() > 0) {
		error = validate_instructions(options.adds);
		if(error != ECSACT_ASYNC_OK) {
			return error;
		}
	}

	if(options.updates.size() > 0) {
		error = validate_instructions(options.updates);
		if(error != ECSACT_ASYNC_OK) {
			return error;
		}
	}

	if(options.removes.size() > 0) {
		error = validate_instructions(options.removes);
		if(error != ECSACT_ASYNC_OK) {
			return error;
		}
	}

	return error;
}

ecsact_async_error util::validate_merge_options(
	const types::cpp_execution_options& options,
	const types::cpp_execution_options& other_options
) {
	ecsact_async_error error = ECSACT_ASYNC_OK;

	if(other_options.adds.size() > 0) {
		error = validate_merge_instructions(options.adds, other_options.adds);
		if(error != ECSACT_ASYNC_OK) {
			return error;
		}
	}

	if(options.updates.size() > 0) {
		error = validate_merge_instructions(options.updates, other_options.updates);
		if(error != ECSACT_ASYNC_OK) {
			return error;
		}
	}

	if(options.removes.size() > 0) {
		error = validate_merge_instructions(options.removes, other_options.removes);
		if(error != ECSACT_ASYNC_OK) {
			return error;
		}
	}

	return error;
}

void util::merge_options(
	types::cpp_execution_options&       tick_options,
	const types::cpp_execution_options& other_options
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
	types::pending_execution_options&       pending,
	const types::pending_execution_options& other_pending
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
