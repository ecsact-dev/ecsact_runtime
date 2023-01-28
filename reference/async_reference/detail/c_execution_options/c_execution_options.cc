#include "c_execution_options.hh"

using namespace ecsact::async_reference::detail;

ecsact_execution_options c_execution_options::c() {
	for(auto& action_info : actions_info) {
		ecsact_action action;
		action.action_data = action_info.data.data();
		action.action_id = action_info.id;

		actions.push_back(action);
	}

	for(auto& add_info : adds_info) {
		auto component = ecsact_component{};

		component.component_id = add_info.id;
		component.component_data = add_info.data.data();

		adds.push_back(component);
	}

	for(auto& update_info : updates_info) {
		auto component = ecsact_component{};

		component.component_id = update_info.id;
		component.component_data = update_info.data.data();

		updates.push_back(component);
	}

	options.actions = actions.data();
	options.actions_length = actions.size();

	options.add_components = adds.data();
	options.add_components_entities = adds_entities.data();
	options.add_components_length = adds.size();

	options.update_components = updates.data();
	options.update_components_entities = updates_entities.data();
	options.update_components_length = updates.size();

	options.remove_components = remove_ids.data();
	options.remove_components_entities = removes_entities.data();
	options.remove_components_length = remove_ids.size();

	return options;
}
