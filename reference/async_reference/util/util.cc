#include "util.hh"

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
	const ecsact_registry_id&    registry_id
) {
	ecsact_execution_options c_options{};

	if(options.actions.size() > 0) {
		c_options.actions = options.actions.data();
		c_options.actions_length = options.actions.size();
	}
	if(options.adds.size() > 0) {
		auto adds_range = std::ranges::views::all(options.adds);

		auto                          entities = util::get_cpp_entities(adds_range);
		std::vector<ecsact_entity_id> entity_list;
		entity_list.insert(entity_list.end(), entities.begin(), entities.end());

		std::vector<ecsact_component> component_list;

		for(int i = 0; i < options.adds.size(); i++) {
			ecsact_component component;
			auto             component_info = options.adds[i];
			const void*      component_data = ecsact_get_component(
        registry_id,
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
	const ecsact_execution_options options
) {
	types::cpp_execution_options cpp_options;

	if(options.add_components != nullptr) {
		for(int i = 0; i < options.add_components_length; i++) {
			types::cpp_execution_component add_component;

			add_component._id = options.add_components[i].component_id;
			add_component.entity_id = options.add_components_entities[i];
			cpp_options.adds.insert(cpp_options.adds.end(), add_component);
		}
	}

	if(options.update_components != nullptr) {
		for(int i = 0; i < options.update_components_length; i++) {
			types::cpp_execution_component update_component;

			update_component._id = options.update_components[i].component_id;
			update_component.entity_id = options.update_components_entities[i];
			cpp_options.updates.insert(cpp_options.updates.end(), update_component);
		}
	}

	if(options.remove_components != nullptr) {
		for(int i = 0; i < options.remove_components_length; i++) {
			types::cpp_execution_component remove_component;

			remove_component._id = options.remove_components[i];
			remove_component.entity_id = options.remove_components_entities[i];
			cpp_options.removes.insert(cpp_options.removes.end(), remove_component);
		}
	}

	if(options.actions != nullptr) {
		cpp_options.actions.insert(
			cpp_options.actions.end(),
			options.actions,
			options.actions + options.actions_length
		);
	}
	return cpp_options;
}

ecsact_async_error util::validate_options(types::cpp_execution_options& options
) {
	ecsact_async_error error = ECSACT_ASYNC_OK;

	if(options.adds.size() > 0) {
		error = validate_instructions(options.adds);
	}

	if(error != ECSACT_ASYNC_OK) {
		return error;
	}

	if(options.updates.size() > 0) {
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