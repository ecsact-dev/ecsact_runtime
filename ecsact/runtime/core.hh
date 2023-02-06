#pragma once

#include <type_traits>
#include <span>
#include <vector>
#include <utility>
#if __has_include(<ranges>)
#	include <ranges>
#endif
#include "ecsact/runtime/core.h"

namespace ecsact::core {

/**
 * Lightweight view of a `ecsact_execution_options` struct.
 */
class execution_options_view {
	const ecsact_execution_options& _c_exec_opts;

public:
	inline execution_options_view(const ecsact_execution_options& options)
		: _c_exec_opts(options) {
	}

	inline auto add_components() const -> std::span<ecsact_component> {
		return std::span(
			_c_exec_opts.add_components,
			static_cast<size_t>(_c_exec_opts.add_components_length)
		);
	}

	inline auto add_components_entities() const -> std::span<ecsact_entity_id> {
		return std::span(
			_c_exec_opts.add_components_entities,
			static_cast<size_t>(_c_exec_opts.add_components_length)
		);
	}

	inline auto update_components() const -> std::span<ecsact_component> {
		return std::span(
			_c_exec_opts.update_components,
			static_cast<size_t>(_c_exec_opts.update_components_length)
		);
	}

	inline auto update_components_entities() const
		-> std::span<ecsact_entity_id> {
		return std::span(
			_c_exec_opts.update_components_entities,
			static_cast<size_t>(_c_exec_opts.update_components_length)
		);
	}

	inline auto remove_components() const -> std::span<ecsact_component_id> {
		return std::span(
			_c_exec_opts.remove_components,
			static_cast<size_t>(_c_exec_opts.remove_components_length)
		);
	}

	inline auto remove_components_entities() const
		-> std::span<ecsact_entity_id> {
		return std::span(
			_c_exec_opts.remove_components_entities,
			static_cast<size_t>(_c_exec_opts.remove_components_length)
		);
	}

	inline auto actions() const -> std::span<ecsact_action> {
		return std::span(
			_c_exec_opts.actions,
			static_cast<size_t>(_c_exec_opts.actions_length)
		);
	}

	inline auto add_components_pairs() const
		-> std::vector<std::pair<ecsact_entity_id, ecsact_component>> {
		auto result = std::vector<std::pair<ecsact_entity_id, ecsact_component>>{};
		result.reserve(_c_exec_opts.add_components_length);
		for(int i = 0; _c_exec_opts.add_components_length > i; ++i) {
			auto entity = _c_exec_opts.add_components_entities[i];
			auto comp = _c_exec_opts.add_components[i];
			result.push_back({entity, comp});
		}

		return result;
	}

	inline auto update_components_pairs() const
		-> std::vector<std::pair<ecsact_entity_id, ecsact_component>> {
		auto result = std::vector<std::pair<ecsact_entity_id, ecsact_component>>{};
		result.reserve(_c_exec_opts.update_components_length);
		for(int i = 0; _c_exec_opts.update_components_length > i; ++i) {
			auto entity = _c_exec_opts.update_components_entities[i];
			auto comp = _c_exec_opts.update_components[i];
			result.push_back({entity, comp});
		}

		return result;
	}

	inline auto remove_components_pairs() const
		-> std::vector<std::pair<ecsact_entity_id, ecsact_component_id>> {
		auto result =
			std::vector<std::pair<ecsact_entity_id, ecsact_component_id>>{};
		result.reserve(_c_exec_opts.remove_components_length);
		for(int i = 0; _c_exec_opts.remove_components_length > i; ++i) {
			auto entity = _c_exec_opts.remove_components_entities[i];
			auto comp = _c_exec_opts.remove_components[i];
			result.push_back({entity, comp});
		}

		return result;
	}

	/**
	 * @returns `true` when all lengths are `0`
	 */
	inline auto empty() const noexcept -> bool {
		return //
			_c_exec_opts.actions_length == 0 &&
			_c_exec_opts.add_components_length == 0 &&
			_c_exec_opts.update_components_length == 0 &&
			_c_exec_opts.remove_components_length == 0;
	}

#ifdef __cpp_lib_ranges_zip
	inline auto add_components_zip() {
		return std::views::zip(add_components_entities(), add_components());
	}

	inline auto update_components_zip() {
		return std::views::zip(update_components_entities(), update_components());
	}

	inline auto remove_components_zip() {
		return std::views::zip(remove_components_entities(), remove_components());
	}
#endif
};

template<std::size_t Extent = std::dynamic_extent>
class execution_options_view_span {
	std::span<ecsact_execution_options, Extent> _span;

public:
	using size_type = std::size_t;

	constexpr execution_options_view_span() noexcept {
	}

	template<class It>
	constexpr execution_options_view_span(It first, size_type count)
		: _span(std::forward<It>(first), count) {
	}

	template<class It, class End>
	constexpr execution_options_view_span(It first, End last)
		: _span(std::forward<It>(first), std::forward<End>(last)) {
	}
};

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

} // namespace ecsact::core
