#pragma once

#include <type_traits>
#include <vector>
#include <functional>
#include "ecsact/runtime/core.h"

namespace ecsact::core {

class registry {
	ecsact_registry_id _id;
	bool               _owned = false;

public:
	registry(const char* name) {
		_id = ecsact_create_registry(name);
		_owned = true;
	}

	explicit registry(ecsact_registry_id existing_registry_id) {
		_id = existing_registry_id;
		_owned = false;
	}

	registry(registry&& other) : _id(other._id), _owned(other._owned) {
		other._owned = false;
	}

	~registry() {
		if(_owned) {
			ecsact_destroy_registry(_id);
		}

		_owned = false;
	}

	registry& operator=(registry&& other) {
		_id = other._id;
		_owned = other._owned;
		other._owned = false;
		return *this;
	}

	bool operator==(const registry& other) {
		return _id == other._id;
	}

	ecsact_registry_id id() const noexcept {
		return _id;
	}

	auto create_entity() {
		return ecsact_create_entity(_id);
	}

	template<typename Component>
		requires(!std::is_empty_v<Component>)
	inline const Component& get_component(ecsact_entity_id entity_id) {
		return *reinterpret_cast<const Component*>(
			ecsact_get_component(_id, entity_id, Component::id)
		);
	}

	template<typename Component>
	inline bool has_component(ecsact_entity_id entity_id) {
		return ecsact_has_component(_id, entity_id, Component::id);
	}

	template<typename Component>
		requires(std::is_empty_v<Component>)
	inline auto add_component(ecsact_entity_id entity_id) {
		return ecsact_add_component(_id, entity_id, Component::id, nullptr);
	}

	template<typename Component>
	inline auto add_component(
		ecsact_entity_id entity_id,
		const Component& component
	) {
		if constexpr(std::is_empty_v<Component>) {
			return ecsact_add_component(_id, entity_id, Component::id, nullptr);
		} else {
			return ecsact_add_component(_id, entity_id, Component::id, &component);
		}
	}

	template<typename Component>
	inline auto update_component(
		ecsact_entity_id entity_id,
		const Component& component
	) {
		return ecsact_update_component(_id, entity_id, Component::id, &component);
	}
};

class builder_entity {
public:
	template<typename C>
	inline builder_entity& add_component(C* component) {
		components.push_back(ecsact_component{
			.component_id = C::id,
			.component_data = component,
		});
		return *this;
	}

	friend class execution_options;

private:
	std::vector<ecsact_component> components;
};

class execution_options {
public:
	/**
	 * The lifetime of @p `component` must be maintained until the
	 * `ecsact::core::execution_options` destructor occurs or `clear()` occurs.
	 */
	template<typename C>
	void add_component(ecsact_entity_id entity, C* component) {
		add_components_container.push_back(
			ecsact_component{.component_id = C::id, .component_data = component}
		);
		add_entities_container.push_back(entity);
	}

	/**
	 * The lifetime of @p `component` must be maintained until the
	 * `ecsact::core::execution_options` destructor occurs or `clear()` occurs.
	 */
	template<typename C>
	void update_component(ecsact_entity_id entity, C* component) {
		update_components_container.push_back(
			ecsact_component{.component_id = C::id, .component_data = component}
		);
		update_entities_container.push_back(entity);
	}

	template<typename C>
	inline void remove_component(ecsact_entity_id entity_id) {
		remove_component_ids_container.push_back(C::id);
		remove_entities_container.push_back(entity_id);
	}

	inline void remove_component(
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id
	) {
		remove_component_ids_container.push_back(component_id);
		remove_entities_container.push_back(entity_id);
	}

	inline builder_entity& create_entity() {
		auto& builder = create_entities.emplace_back(builder_entity{});
		return builder;
	}

	inline void destroy_entity(const ecsact_entity_id& entity_id) {
		destroy_entities.push_back(entity_id);
	}

	inline void push_action(const ecsact_action& action) {
		actions.push_back(action);
	}

	inline void clear() {
		options = {};

		add_entities_container.clear();
		add_components_container.clear();

		update_entities_container.clear();
		update_components_container.clear();

		remove_entities_container.clear();
		remove_component_ids_container.clear();

		entities_components.clear();
		entities_component_lengths.clear();

		create_entities.clear();
		destroy_entities.clear();

		actions.clear();
	}

	inline ecsact_execution_options c() {
		options.add_components_length = add_components_container.size();
		options.add_components_entities = add_entities_container.data();
		options.add_components = add_components_container.data();

		options.update_components_length = update_components_container.size();
		options.update_components_entities = update_entities_container.data();
		options.update_components = update_components_container.data();

		options.remove_components_length = remove_component_ids_container.size();
		options.remove_components_entities = remove_entities_container.data();
		options.remove_components = remove_component_ids_container.data();

		for(auto& built_entity : create_entities) {
			entities_component_lengths.push_back(built_entity.components.size());
			entities_components.push_back(built_entity.components.data());
		}

		options.create_entities_components = entities_components.data();
		options.create_entities_length = create_entities.size();
		options.create_entities_components_length =
			entities_component_lengths.data();

		options.destroy_entities = destroy_entities.data();
		options.destroy_entities_length = destroy_entities.size();

		options.actions = actions.data();
		options.actions_length = actions.size();

		return options;
	}

private:
	std::vector<ecsact_entity_id> add_entities_container;
	std::vector<ecsact_component> add_components_container;

	std::vector<ecsact_entity_id> update_entities_container;
	std::vector<ecsact_component> update_components_container;

	std::vector<ecsact_entity_id>    remove_entities_container;
	std::vector<ecsact_component_id> remove_component_ids_container;

	std::vector<builder_entity>   create_entities;
	std::vector<ecsact_entity_id> destroy_entities;

	std::vector<int>               entities_component_lengths;
	std::vector<ecsact_component*> entities_components;

	std::vector<ecsact_action> actions;

	ecsact_execution_options options;
};

} // namespace ecsact::core
