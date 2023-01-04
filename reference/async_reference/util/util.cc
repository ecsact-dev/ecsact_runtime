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

	auto entities = util::get_cpp_entities(components);
	auto other_entities = util::get_cpp_entities(other_components);

	auto empty_view = std::ranges::empty_view<int>{};

	auto has_duplicates =
		util::check_entity_merge_duplicates(entities, other_entities);

	if(!has_duplicates) {
		return ECSACT_ASYNC_OK;
	}

	for(auto& component : components) {
		auto component_view = std::views::filter(
			other_components,
			[&other_components,
			 &component](types::cpp_execution_component other_component) {
				return component._id == other_component._id;
			}
		);

		int same_component_count = 0;
		int same_entity_count = 0;
		for(auto& other_component : component_view) {
			same_component_count++;
			if(component.entity_id == other_component.entity_id) {
				same_entity_count++;
			}
			if(same_component_count > 0 && same_entity_count > 0) {
				return ECSACT_ASYNC_ERR_EXECUTION_MERGE_FAILURE;
			}
		}
	}

	return ECSACT_ASYNC_OK;
}

void util::cpp_to_c_execution_options(
	detail::c_execution_options&  out_c_execution_options,
	types::cpp_execution_options& options,
	const ecsact_registry_id&     registry_id
) {
	// out_c_execution_options.actions_info.resize(options.actions.size());
	// out_c_execution_options.adds_info.resize(options.adds.size());
	// out_c_execution_options.updates_info.resize(options.updates.size());
	// out_c_execution_options.remove_ids.resize(options.removes.size());

	if(options.actions.size() > 0) {
		for(auto& action_info : options.actions) {
			out_c_execution_options.actions_info.push_back(
				detail::data_info<ecsact_action_id>{
					.id = action_info.action_id,
					.data = ecsact::deserialize(
						action_info.action_id,
						action_info.serialized_data
					),
				}
			);
		}
	}

	if(options.adds.size() > 0) {
		auto        entities_view = util::get_cpp_entities(options.adds);
		std::vector entities(entities_view.begin(), entities_view.end());

		for(int i = 0; i < options.adds.size(); i++) {
			auto& component_info = options.adds[i];

			out_c_execution_options.adds_info.push_back(
				detail::data_info<ecsact_component_id>{
					.id = component_info._id,
					.data = ecsact::deserialize(component_info._id, component_info.data),
				}
			);
		}
		out_c_execution_options.adds_entities = entities;
	}

	if(options.updates.size() > 0) {
		auto        entities_view = util::get_cpp_entities(options.updates);
		std::vector entities(entities_view.begin(), entities_view.end());

		for(int i = 0; i < options.updates.size(); i++) {
			auto component_info = options.updates[i];

			out_c_execution_options.updates_info.push_back(
				detail::data_info<ecsact_component_id>{
					.id = component_info._id,
					.data = ecsact::deserialize(component_info._id, component_info.data),
				}
			);
		}
		out_c_execution_options.updates_entities = entities;
	}

	if(options.removes.size() > 0) {
		auto        entities_view = util::get_cpp_entities(options.removes);
		std::vector entities(entities_view.begin(), entities_view.end());

		auto component_ids_view = util::get_cpp_component_ids(options.removes);
		std::vector component_ids(
			component_ids_view.begin(),
			component_ids_view.end()
		);

		out_c_execution_options.remove_ids.insert(
			out_c_execution_options.remove_ids.begin(),
			component_ids.begin(),
			component_ids.end()
		);

		out_c_execution_options.removes_entities = entities;
	}
}

types::cpp_execution_options util::c_to_cpp_execution_options(
	const ecsact_execution_options& options
) {
	auto cpp_options = types::cpp_execution_options{};
	cpp_options.adds.reserve(options.add_components_length);
	cpp_options.updates.reserve(options.update_components_length);
	cpp_options.removes.reserve(options.remove_components_length);
	cpp_options.actions.reserve(options.actions_length);

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
