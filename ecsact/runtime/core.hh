#pragma once

#include <type_traits>
#include <vector>
#include <map>
#include <functional>
#include <optional>
#include <cassert>
#include "ecsact/runtime/core.h"

namespace ecsact::core {

class builder_entity {
	friend class execution_options;

public:
	template<typename C>
	ECSACT_ALWAYS_INLINE auto add_component(C* component) -> builder_entity& {
		components.push_back(ecsact_component{
			.component_id = C::id,
			.component_data = component,
		});
		return *this;
	}

private:
	ecsact_placeholder_entity_id  placeholder_entity_id;
	std::vector<ecsact_component> components;
};

class execution_options {
public:
	execution_options() = default;
	execution_options(const execution_options&) = default;
	execution_options(execution_options&&) = default;

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
	ECSACT_ALWAYS_INLINE void remove_component(ecsact_entity_id entity_id) {
		remove_component_ids_container.push_back(C::id);
		remove_entities_container.push_back(entity_id);
	}

	ECSACT_ALWAYS_INLINE void remove_component(
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id
	) {
		remove_component_ids_container.push_back(component_id);
		remove_entities_container.push_back(entity_id);
	}

	ECSACT_ALWAYS_INLINE auto create_entity(
		ecsact_placeholder_entity_id placeholder_entity_id = {}
	) -> builder_entity& {
		auto& builder = create_entities.emplace_back();
		builder.placeholder_entity_id = placeholder_entity_id;
		return builder;
	}

	ECSACT_ALWAYS_INLINE void destroy_entity(const ecsact_entity_id& entity_id) {
		destroy_entities.push_back(entity_id);
	}

	/**
	 * The lifetime of @p `action` must be maintained until the
	 * `ecsact::core::execution_options` destructor occurs or `clear()` occurs.
	 */
	template<typename Action>
	ECSACT_ALWAYS_INLINE void push_action(const Action* action) {
		actions.push_back(ecsact_action{Action::id, action});
	}

	ECSACT_ALWAYS_INLINE void clear() {
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

	ECSACT_ALWAYS_INLINE auto c() -> ecsact_execution_options {
		auto options = ecsact_execution_options{};

		options.add_components_length = add_components_container.size();
		options.add_components_entities = add_entities_container.data();
		options.add_components = add_components_container.data();

		options.update_components_length = update_components_container.size();
		options.update_components_entities = update_entities_container.data();
		options.update_components = update_components_container.data();

		options.remove_components_length = remove_component_ids_container.size();
		options.remove_components_entities = remove_entities_container.data();
		options.remove_components = remove_component_ids_container.data();

		entities_components.clear();
		entities_components.reserve(create_entities.size());
		entities_component_lengths.clear();
		entities_component_lengths.reserve(create_entities.size());
		create_placeholders.clear();
		create_placeholders.reserve(create_entities.size());
		for(auto& built_entity : create_entities) {
			create_placeholders.push_back(built_entity.placeholder_entity_id);
			entities_component_lengths.push_back(built_entity.components.size());
			entities_components.push_back(built_entity.components.data());
		}

		options.create_entities = create_placeholders.data();
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

	std::vector<ecsact_placeholder_entity_id> create_placeholders;
	std::vector<int32_t>                      entities_component_lengths;
	std::vector<ecsact_component*>            entities_components;

	std::vector<ecsact_action> actions;
};

class any_component_view {
	ecsact_component_id _component_id;
	const void*         _component_data;

public:
	inline any_component_view(ecsact_component_id id, const void* data)
		: _component_id(id), _component_data(data) {
	}

	inline any_component_view(const any_component_view&) = default;
	inline ~any_component_view() = default;

	template<typename C>
	auto is() const -> bool {
		return _component_id == C::id;
	}

	template<typename C>
	auto as() const -> const C& {
		assert(is<C>());
		return *static_cast<const C*>(_component_data);
	}
};

template<
	template<class R, class... Args> typename CallbackContainer = std::function>
class execution_events_collector {
public:
	using any_component_callback_t =
		CallbackContainer<void(ecsact_entity_id, any_component_view)>;

	template<typename C>
	using init_component_callback_t =
		CallbackContainer<void(ecsact_entity_id, const C&)>;

	template<typename C>
	using update_component_callback_t =
		CallbackContainer<void(ecsact_entity_id, const C&)>;

	template<typename C>
	using remove_component_callback_t =
		CallbackContainer<void(ecsact_entity_id, const C&)>;

	using entity_created_callback_t =
		CallbackContainer<void(ecsact_entity_id, ecsact_placeholder_entity_id)>;

	using entity_destroyed_callback_t = CallbackContainer<void(ecsact_entity_id)>;

	/**
	 * Set the init callback for component @tp C. Overwrite existing callback
	 * for @tp C if one exists.
	 */
	template<typename C>
	auto set_init_callback( //
		init_component_callback_t<C> callback
	) -> execution_events_collector& {
		_current_init_callback = &execution_events_collector::typed_init_callback;
		_init_cb[C::id] = //
			[callback = std::move(callback)](
				ecsact_entity_id    entity,
				ecsact_component_id component_id,
				const void*         component_data
			) { callback(entity, *static_cast<const C*>(component_data)); };
		return *this;
	}

	/**
	 * Set the update callback for component @tp C. Overwrite existing callback
	 * for @tp C if one exists.
	 */
	template<typename C>
	auto set_update_callback( //
		update_component_callback_t<C> callback
	) -> execution_events_collector& {
		_current_update_callback =
			&execution_events_collector::typed_update_callback;
		_update_cb[C::id] = //
			[callback = std::move(callback)](
				ecsact_entity_id    entity,
				ecsact_component_id component_id,
				const void*         component_data
			) { callback(entity, *static_cast<const C*>(component_data)); };
		return *this;
	}

	/**
	 * Set the remove callback for component @tp C. Overwrite existing callback
	 * for @tp C if one exists.
	 */
	template<typename C>
	auto set_remove_callback( //
		remove_component_callback_t<C> callback
	) -> execution_events_collector& {
		_current_remove_callback =
			&execution_events_collector::typed_remove_callback;
		_remove_cb[C::id] = //
			[callback = std::move(callback)](
				ecsact_entity_id    entity,
				ecsact_component_id component_id,
				const void*         component_data
			) { callback(entity, *static_cast<const C*>(component_data)); };
		return *this;
	}

	auto set_entity_created_callback( //
		entity_created_callback_t callback
	) -> execution_events_collector& {
		_entity_created_cb = callback;
		return *this;
	}

	auto set_entity_destroyed_callback( //
		entity_destroyed_callback_t callback
	) -> execution_events_collector& {
		_entity_destroyed_cb = callback;
		return *this;
	}

	/**
	 * Set the init callback for when _any_ component is initialized. Overwrites
	 * existing init callbacks, including the typed ones.
	 *
	 * NOTE: You should prefer @ref set_init_callback over this function.
	 */
	auto set_any_init_callback( //
		any_component_callback_t callback
	) -> execution_events_collector& {
		if constexpr(std::is_convertible_v<any_component_callback_t, bool>) {
			if(!callback) {
				return;
			}
		}
		_current_init_callback = &execution_events_collector::any_init_callback;
		_any_init_component_cb = callback;
		return *this;
	}

	/**
	 * Set the update callback for when _any_ component is updated. Overwrites
	 * existing update callbacks, including the typed ones.
	 *
	 * NOTE: You should prefer @ref set_update_callback over this function.
	 */
	auto set_any_update_callback( //
		any_component_callback_t callback
	) -> execution_events_collector& {
		if constexpr(std::is_convertible_v<any_component_callback_t, bool>) {
			if(!callback) {
				return;
			}
		}
		_current_update_callback = &execution_events_collector::any_update_callback;
		_any_update_component_cb = callback;
		return *this;
	}

	/**
	 * Set the remove callback for when _any_ component is removed. Overwrites
	 * existing remove callbacks, including the typed ones.
	 *
	 * NOTE: You should prefer @ref set_remove_callback over this function.
	 */
	auto set_any_remove_callback( //
		any_component_callback_t callback
	) -> execution_events_collector& {
		if constexpr(std::is_convertible_v<any_component_callback_t, bool>) {
			if(!callback) {
				return;
			}
		}
		_current_remove_callback = &execution_events_collector::any_remove_callback;
		_any_remove_component_cb = callback;
		return *this;
	}

	auto c() const -> const ecsact_execution_events_collector {
		auto evc = ecsact_execution_events_collector{};
		auto user_data =
			static_cast<void*>(const_cast<execution_events_collector*>(this));

		if(_current_init_callback != nullptr) {
			evc.init_callback = &execution_events_collector::init_callback;
			evc.init_callback_user_data = user_data;
		}
		if(_current_update_callback != nullptr) {
			evc.update_callback = &execution_events_collector::update_callback;
			evc.update_callback_user_data = user_data;
		}
		if(_current_remove_callback != nullptr) {
			evc.remove_callback = &execution_events_collector::remove_callback;
			evc.remove_callback_user_data = user_data;
		}
		if(_entity_created_cb) {
			evc.entity_created_callback =
				&execution_events_collector::entity_created_callback;
			evc.entity_created_callback_user_data = user_data;
		}
		if(_entity_destroyed_cb) {
			evc.entity_destroyed_callback =
				&execution_events_collector::entity_destroyed_callback;
			evc.entity_destroyed_callback_user_data = user_data;
		}

		return evc;
	}

	auto clear() -> void {
		_current_init_callback = nullptr;
		_current_remove_callback = nullptr;
		_current_update_callback = nullptr;
		_init_cb.clear();
		_update_cb.clear();
		_remove_cb.clear();
		_entity_created_cb = std::nullopt;
		_entity_destroyed_cb = std::nullopt;
	}

	auto empty() const noexcept -> bool {
		return _current_init_callback == nullptr &&
			_current_remove_callback == nullptr &&
			_current_update_callback == nullptr && !_entity_created_cb.has_value() &&
			!_entity_destroyed_cb.has_value();
	}

private:
	// std::function is used here explicitly for type erasure
	using _component_cb_t =
		std::function<void(ecsact_entity_id, ecsact_component_id, const void*)>;
	using _component_cb_map_t = std::map<ecsact_component_id, _component_cb_t>;

	void (execution_events_collector::*_current_init_callback)(
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data
	) = nullptr;

	void (execution_events_collector::*_current_update_callback)(
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data
	) = nullptr;

	void (execution_events_collector::*_current_remove_callback)(
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data
	) = nullptr;

	any_component_callback_t _any_init_component_cb;
	any_component_callback_t _any_update_component_cb;
	any_component_callback_t _any_remove_component_cb;

	_component_cb_map_t _init_cb;
	_component_cb_map_t _update_cb;
	_component_cb_map_t _remove_cb;

	std::optional<entity_created_callback_t>   _entity_created_cb;
	std::optional<entity_destroyed_callback_t> _entity_destroyed_cb;

	auto typed_init_callback(
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data
	) -> void {
		auto itr = _init_cb.find(component_id);
		if(itr != _init_cb.end()) {
			itr->second(entity_id, component_id, component_data);
		}
	}

	auto any_init_callback(
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data
	) -> void {
		_any_init_component_cb(
			entity_id,
			any_component_view{component_id, component_data}
		);
	}

	auto typed_update_callback(
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data
	) -> void {
		auto itr = _update_cb.find(component_id);
		if(itr != _update_cb.end()) {
			itr->second(entity_id, component_id, component_data);
		}
	}

	auto any_update_callback(
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data
	) -> void {
		_any_update_component_cb(
			entity_id,
			any_component_view{component_id, component_data}
		);
	}

	auto typed_remove_callback(
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data
	) -> void {
		auto itr = _remove_cb.find(component_id);
		if(itr != _remove_cb.end()) {
			itr->second(entity_id, component_id, component_data);
		}
	}

	auto any_remove_callback(
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data
	) -> void {
		_any_remove_component_cb(
			entity_id,
			any_component_view{component_id, component_data}
		);
	}

	static void init_callback(
		ecsact_event,
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data,
		void*               callback_user_data
	) {
		auto self = static_cast<execution_events_collector*>(callback_user_data);
		if(self->_current_init_callback) {
			std::invoke(
				self->_current_init_callback,
				*self,
				entity_id,
				component_id,
				component_data
			);
		}
	}

	static void update_callback(
		ecsact_event        event,
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data,
		void*               callback_user_data
	) {
		auto self = static_cast<execution_events_collector*>(callback_user_data);
		if(self->_current_update_callback) {
			std::invoke(
				self->_current_update_callback,
				*self,
				entity_id,
				component_id,
				component_data
			);
		}
	}

	static void remove_callback(
		ecsact_event        event,
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data,
		void*               callback_user_data
	) {
		auto self = static_cast<execution_events_collector*>(callback_user_data);
		if(self->_current_remove_callback) {
			std::invoke(
				self->_current_remove_callback,
				*self,
				entity_id,
				component_id,
				component_data
			);
		}
	}

	static void entity_created_callback(
		ecsact_event                 event,
		ecsact_entity_id             entity_id,
		ecsact_placeholder_entity_id placeholder_entity_id,
		void*                        callback_user_data
	) {
		auto self = static_cast<execution_events_collector*>(callback_user_data);
		if(self->_entity_created_cb.has_value()) {
			self->_entity_created_cb.value()(entity_id, placeholder_entity_id);
		}
	}

	static void entity_destroyed_callback(
		ecsact_event                 event,
		ecsact_entity_id             entity_id,
		ecsact_placeholder_entity_id placeholder_entity_id,
		void*                        callback_user_data
	) {
		auto self = static_cast<execution_events_collector*>(callback_user_data);
		if(self->_entity_destroyed_cb.has_value()) {
			self->_entity_destroyed_cb.value()(entity_id);
		}
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

	auto clear() -> void {
		ecsact_clear_registry(_id);
	}

	auto empty() const -> bool {
		return ecsact_count_entities(_id) == 0;
	}

	auto create_entity() {
		return ecsact_create_entity(_id);
	}

	template<typename Component, typename... AssocFields>
		requires(!std::is_empty_v<Component>)
	ECSACT_ALWAYS_INLINE auto get_component( //
		ecsact_entity_id entity_id,
		AssocFields&&... assoc_fields
	) -> const Component& {
		if constexpr(Component::has_assoc_fields) {
			static_assert(
				sizeof...(AssocFields) > 0,
				"must be called with assoc fields"
			);
		}

		if constexpr(sizeof...(AssocFields) > 0) {
			const void* assoc_field_values[sizeof...(AssocFields)] = {
				&assoc_fields...,
			};

			return *reinterpret_cast<const Component*>(
				ecsact_get_component(_id, entity_id, Component::id, assoc_field_values)
			);
		} else {
			return *reinterpret_cast<const Component*>(
				ecsact_get_component(_id, entity_id, Component::id, nullptr)
			);
		}
	}

	template<typename Component, typename... AssocFields>
	ECSACT_ALWAYS_INLINE bool has_component(
		ecsact_entity_id entity_id,
		AssocFields&&... assoc_fields
	) {
		if constexpr(Component::has_assoc_fields) {
			static_assert(
				sizeof...(AssocFields) > 0,
				"must be called with assoc fields"
			);
		}

		if constexpr(sizeof...(AssocFields) > 0) {
			const void* assoc_field_values[sizeof...(AssocFields)] = {
				&assoc_fields...,
			};
			return ecsact_has_component(
				_id,
				entity_id,
				Component::id,
				assoc_field_values
			);
		} else {
			return ecsact_has_component(_id, entity_id, Component::id, nullptr);
		}
	}

	template<typename Component>
		requires(std::is_empty_v<Component>)
	ECSACT_ALWAYS_INLINE auto add_component(ecsact_entity_id entity_id) {
		return ecsact_add_component(_id, entity_id, Component::id, nullptr);
	}

	template<typename Component>
	ECSACT_ALWAYS_INLINE auto add_component(
		ecsact_entity_id entity_id,
		const Component& component
	) {
		if constexpr(std::is_empty_v<Component>) {
			return ecsact_add_component(_id, entity_id, Component::id, nullptr);
		} else {
			return ecsact_add_component(_id, entity_id, Component::id, &component);
		}
	}

	template<typename Component, typename... AssocFields>
	ECSACT_ALWAYS_INLINE auto update_component(
		ecsact_entity_id entity_id,
		const Component& component,
		AssocFields&&... assoc_fields
	) {
		if constexpr(Component::has_assoc_fields) {
			static_assert(
				sizeof...(AssocFields) > 0,
				"must be called with assoc fields"
			);
		}

		if constexpr(sizeof...(AssocFields) > 0) {
			const void* assoc_field_values[sizeof...(AssocFields)] = {
				&assoc_fields...,
			};
			return ecsact_update_component(
				_id,
				entity_id,
				Component::id,
				&component,
				assoc_field_values
			);
		} else {
			return ecsact_update_component(
				_id,
				entity_id,
				Component::id,
				&component,
				nullptr
			);
		}
	}

	template<typename Component, typename... AssocFields>
	ECSACT_ALWAYS_INLINE auto remove_component(
		ecsact_entity_id entity_id,
		AssocFields&&... assoc_fields
	) -> void {
		if constexpr(Component::has_assoc_fields) {
			static_assert(
				sizeof...(AssocFields) > 0,
				"must be called with assoc fields"
			);
		}

		if constexpr(sizeof...(AssocFields) > 0) {
			const void* assoc_field_values[sizeof...(AssocFields)] = {
				&assoc_fields...,
			};

			return ecsact_remove_component(
				_id,
				entity_id,
				Component::id,
				assoc_field_values
			);
		} else {
			return ecsact_remove_component(_id, entity_id, Component::id, nullptr);
		}
	}

	ECSACT_ALWAYS_INLINE auto count_entities() const -> int32_t {
		return ecsact_count_entities(_id);
	}

	ECSACT_ALWAYS_INLINE auto get_entities() const
		-> std::vector<ecsact_entity_id> {
		const auto entities_count = count_entities();
		auto       entities = std::vector<ecsact_entity_id>{};
		entities.resize(entities_count);
		ecsact_get_entities(_id, entities_count, entities.data(), nullptr);
		return entities;
	}

	ECSACT_ALWAYS_INLINE auto count_components( //
		ecsact_entity_id entity
	) const -> int32_t {
		return ecsact_count_components(_id, entity);
	}

	/**
	 * Execute systems @p execution_count times.
	 * @param execution_count must be >= 1
	 */
	auto execute_systems(int32_t execution_count = 1) -> void {
		ecsact_execute_systems(_id, execution_count, nullptr, nullptr);
	}

	/**
	 * Execute systems with options. Executes systems once for each element
	 * in @p execution_options range.
	 */
	template<typename ExecutionOptionsRange>
	[[nodiscard]] auto execute_systems( //
		ExecutionOptionsRange&& execution_options
	) -> ecsact_execute_systems_error {
		auto        execution_count = std::size(execution_options);
		const auto* execution_options_list_data = std::data(execution_options);

		const ecsact_execution_options* c_execution_options_list = nullptr;
		if constexpr(std::is_same_v<
									 decltype(execution_options_list_data),
									 decltype(c_execution_options_list)>) {
			return ecsact_execute_systems(
				_id,
				static_cast<int32_t>(execution_count),
				execution_options_list_data,
				nullptr
			);
		} else {
			auto execution_options_vec = std::vector<ecsact_execution_options>{};
			execution_options_vec.reserve(execution_count);
			for(auto& el : execution_options) {
				execution_options_vec.push_back(el.c());
			}
			return ecsact_execute_systems(
				_id,
				static_cast<int32_t>(execution_count),
				execution_options_vec.data(),
				nullptr
			);
		}
	}

	auto execute_systems(
		int32_t                                  execution_count,
		const ecsact_execution_events_collector& evc
	) -> void {
		ecsact_execute_systems(_id, execution_count, nullptr, &evc);
	}

	template<typename ExecutionEventsCollector>
	ECSACT_ALWAYS_INLINE auto execute_systems( //
		int32_t                    execution_count,
		ExecutionEventsCollector&& evc
	) -> void {
		const ecsact_execution_events_collector evc_c = evc.c();
		execute_systems(execution_count, evc_c);
	}

	template<typename ExecutionOptionsRange, typename ExecutionEventsCollector>
	[[nodiscard]] ECSACT_ALWAYS_INLINE auto execute_systems(
		ExecutionOptionsRange&&    execution_options,
		ExecutionEventsCollector&& evc
	) -> ecsact_execute_systems_error {
		auto        execution_count = std::size(execution_options);
		const auto* execution_options_list_data = std::data(execution_options);

		const ecsact_execution_events_collector evc_c = evc.c();

		const ecsact_execution_options* c_execution_options_list = nullptr;
		if constexpr(std::is_same_v<
									 decltype(execution_options_list_data),
									 decltype(c_execution_options_list)>) {
			return ecsact_execute_systems(
				_id,
				static_cast<int32_t>(execution_count),
				execution_options_list_data,
				&evc_c
			);
		} else {
			auto execution_options_vec = std::vector<ecsact_execution_options>{};
			execution_options_vec.reserve(execution_count);
			for(auto& el : execution_options) {
				execution_options_vec.push_back(el.c());
			}
			return ecsact_execute_systems(
				_id,
				static_cast<int32_t>(execution_count),
				execution_options_vec.data(),
				&evc_c
			);
		}
	}
};

} // namespace ecsact::core
