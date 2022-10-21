#pragma once

#include <type_traits>
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
	inline auto
	add_component(ecsact_entity_id entity_id, const Component& component) {
		if constexpr(std::is_empty_v<Component>) {
			return ecsact_add_component(_id, entity_id, Component::id, nullptr);
		} else {
			return ecsact_add_component(_id, entity_id, Component::id, &component);
		}
	}

	template<typename Component>
	inline auto
	update_component(ecsact_entity_id entity_id, const Component& component) {
		return ecsact_update_component(_id, entity_id, Component::id, &component);
	}
};

} // namespace ecsact::core
