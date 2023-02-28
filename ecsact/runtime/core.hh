#pragma once

#include <type_traits>
#include <vector>
#include <map>
#include <functional>
#include <optional>
#include "common.h"
#include "ecsact/runtime/core.h"

namespace ecsact::core {

class builder_entity {
	friend class execution_options;

public:
	template<typename C>
	inline builder_entity& add_component(C* component) {
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

	inline builder_entity& create_entity(
		ecsact_placeholder_entity_id placeholder_entity_id = {}
	) {
		auto& builder = create_entities.emplace_back();
		builder.placeholder_entity_id = placeholder_entity_id;
		return builder;
	}

	inline void destroy_entity(const ecsact_entity_id& entity_id) {
		destroy_entities.push_back(entity_id);
	}

	/**
	 * The lifetime of @p `action` must be maintained until the
	 * `ecsact::core::execution_options` destructor occurs or `clear()` occurs.
	 */
	template<typename Action>
	inline void push_action(const Action* action) {
		actions.push_back(ecsact_action{Action::id, action});
	}

	inline void clear() {
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

template<
	template<class R, class... Args> typename CallbackContainer = std::function>
class execution_events_collector {
public:
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
	auto set_init_callback(init_component_callback_t<C> callback)
		-> execution_events_collector& {
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
	auto set_update_callback(update_component_callback_t<C> callback)
		-> execution_events_collector& {
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
	auto set_remove_callback(remove_component_callback_t<C> callback)
		-> execution_events_collector& {
		_remove_cb[C::id] = //
			[callback = std::move(callback)](
				ecsact_entity_id    entity,
				ecsact_component_id component_id,
				const void*         component_data
			) { callback(entity, *static_cast<const C*>(component_data)); };
		return *this;
	}

	auto set_entity_created_callback(entity_created_callback_t callback)
		-> execution_events_collector& {
		_entity_created_cb = callback;
		return *this;
	}

	auto set_entity_destroyed_callback(entity_destroyed_callback_t callback)
		-> execution_events_collector& {
		_entity_destroyed_cb = callback;
		return *this;
	}

	auto c() const -> const ecsact_execution_events_collector {
		auto evc = ecsact_execution_events_collector{};
		auto user_data =
			static_cast<void*>(const_cast<execution_events_collector*>(this));

		if(!_init_cb.empty()) {
			evc.init_callback = &execution_events_collector::init_callback;
			evc.init_callback_user_data = user_data;
		}
		if(!_update_cb.empty()) {
			evc.update_callback = &execution_events_collector::update_callback;
			evc.update_callback_user_data = user_data;
		}
		if(!_remove_cb.empty()) {
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
		_init_cb.clear();
		_update_cb.clear();
		_remove_cb.clear();
		_entity_created_cb = std::nullopt;
		_entity_destroyed_cb = std::nullopt;
	}

	auto empty() const noexcept -> bool {
		return _init_cb.empty() && _update_cb.empty() && _remove_cb.empty() &&
			!_entity_created_cb.has_value() && !_entity_destroyed_cb.has_value();
	}

private:
	// std::function is used here explicitly for type erasure
	using _component_cb_t =
		std::function<void(ecsact_entity_id, ecsact_component_id, const void*)>;
	using _component_cb_map_t = std::map<ecsact_component_id, _component_cb_t>;
	_component_cb_map_t _init_cb;
	_component_cb_map_t _update_cb;
	_component_cb_map_t _remove_cb;

	std::optional<entity_created_callback_t>   _entity_created_cb;
	std::optional<entity_destroyed_callback_t> _entity_destroyed_cb;

	static void init_callback(
		ecsact_event        event,
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data,
		void*               callback_user_data
	) {
		auto self = static_cast<execution_events_collector*>(callback_user_data);

		auto itr = self->_init_cb.find(component_id);
		if(itr != self->_init_cb.end()) {
			itr->second(entity_id, component_id, component_data);
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

		auto itr = self->_update_cb.find(component_id);
		if(itr != self->_update_cb.end()) {
			itr->second(entity_id, component_id, component_data);
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

		auto itr = self->_remove_cb.find(component_id);
		if(itr != self->_remove_cb.end()) {
			itr->second(entity_id, component_id, component_data);
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

	auto count_entities() -> int32_t {
		return ecsact_count_entities(_id);
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
	[[nodiscard]] auto execute_systems(ExecutionOptionsRange&& execution_options)
		-> ecsact_execute_systems_error {
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
	auto execute_systems(
		int32_t                    execution_count,
		ExecutionEventsCollector&& evc
	) {
		const ecsact_execution_events_collector evc_c = evc.c();
		execute_systems(execution_count, evc_c);
	}

	template<typename ExecutionOptionsRange, typename ExecutionEventsCollector>
	[[nodiscard]] auto execute_systems(
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
