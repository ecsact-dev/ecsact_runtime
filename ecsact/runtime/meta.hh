#pragma once

#include <vector>
#include <string>
#include <optional>
#include <filesystem>
#include <unordered_map>
#include "ecsact/runtime/common.h"
#include "ecsact/runtime/meta.h"

namespace ecsact::meta {

ECSACT_ALWAYS_INLINE std::vector<ecsact_package_id> get_package_ids() {
	std::vector<ecsact_package_id> package_ids;
	package_ids.resize(ecsact_meta_count_packages());
	ecsact_meta_get_package_ids(
		static_cast<int32_t>(package_ids.size()),
		package_ids.data(),
		nullptr
	);
	return package_ids;
}

ECSACT_ALWAYS_INLINE std::vector<ecsact_package_id> get_package_ids(
	int32_t max_size
) {
	std::vector<ecsact_package_id> package_ids;
	package_ids.resize(max_size);
	ecsact_meta_get_package_ids(max_size, package_ids.data(), &max_size);
	package_ids.resize(max_size);
	return package_ids;
}

ECSACT_ALWAYS_INLINE std::filesystem::path package_file_path(
	ecsact_package_id package_id
) {
	auto file_path = ecsact_meta_package_file_path(package_id);
	if(file_path == nullptr) {
		return {};
	}
	return std::filesystem::path(file_path);
}

ECSACT_ALWAYS_INLINE std::string package_name(ecsact_package_id package_id) {
	auto pkg_name = ecsact_meta_package_name(package_id);
	if(pkg_name == nullptr) {
		return {};
	}
	return pkg_name;
}

ECSACT_ALWAYS_INLINE std::vector<ecsact_package_id> get_dependencies(
	ecsact_package_id package_id
) {
	std::vector<ecsact_package_id> dep_ids;
	dep_ids.resize(ecsact_meta_count_dependencies(package_id));
	ecsact_meta_get_dependencies(
		package_id,
		static_cast<int32_t>(dep_ids.size()),
		dep_ids.data(),
		nullptr
	);
	return dep_ids;
}

ECSACT_ALWAYS_INLINE std::vector<ecsact_package_id> get_dependencies(
	ecsact_package_id package_id,
	int32_t           max_size
) {
	std::vector<ecsact_package_id> dep_ids;
	dep_ids.resize(max_size);
	ecsact_meta_get_dependencies(package_id, max_size, dep_ids.data(), &max_size);
	dep_ids.resize(max_size);
	return dep_ids;
}

ECSACT_ALWAYS_INLINE std::vector<ecsact_component_id> get_component_ids(
	ecsact_package_id package_id
) {
	std::vector<ecsact_component_id> component_ids;
	component_ids.resize(ecsact_meta_count_components(package_id));
	ecsact_meta_get_component_ids(
		package_id,
		static_cast<int32_t>(component_ids.size()),
		component_ids.data(),
		nullptr
	);
	return component_ids;
}

ECSACT_ALWAYS_INLINE std::vector<ecsact_component_id> get_component_ids(
	ecsact_package_id package_id,
	int32_t           max_size
) {
	std::vector<ecsact_component_id> component_ids;
	component_ids.resize(max_size);
	ecsact_meta_get_component_ids(
		package_id,
		max_size,
		component_ids.data(),
		&max_size
	);
	component_ids.resize(max_size);
	return component_ids;
}

ECSACT_ALWAYS_INLINE std::string component_name(ecsact_component_id component_id
) {
	auto comp_name = ecsact_meta_component_name(component_id);
	if(comp_name == nullptr) {
		return {};
	}
	return comp_name;
}

ECSACT_ALWAYS_INLINE std::vector<ecsact_transient_id> get_transient_ids(
	ecsact_package_id package_id
) {
	std::vector<ecsact_transient_id> transient_ids;
	transient_ids.resize(ecsact_meta_count_transients(package_id));
	ecsact_meta_get_transient_ids(
		package_id,
		static_cast<int32_t>(transient_ids.size()),
		transient_ids.data(),
		nullptr
	);
	return transient_ids;
}

ECSACT_ALWAYS_INLINE std::vector<ecsact_transient_id> get_transient_ids(
	ecsact_package_id package_id,
	int32_t           max_size
) {
	std::vector<ecsact_transient_id> transient_ids;
	transient_ids.resize(max_size);
	ecsact_meta_get_transient_ids(
		package_id,
		max_size,
		transient_ids.data(),
		&max_size
	);
	transient_ids.resize(max_size);
	return transient_ids;
}

ECSACT_ALWAYS_INLINE std::string transient_name(ecsact_transient_id transient_id
) {
	auto comp_name = ecsact_meta_transient_name(transient_id);
	if(comp_name == nullptr) {
		return {};
	}
	return comp_name;
}

template<typename CompositeID>
ECSACT_ALWAYS_INLINE std::vector<ecsact_field_id> get_field_ids(CompositeID id
) {
	std::vector<ecsact_field_id> field_ids;
	auto compo_id = ecsact_id_cast<ecsact_composite_id>(id);
	field_ids.resize(ecsact_meta_count_fields(compo_id));
	ecsact_meta_get_field_ids(
		compo_id,
		static_cast<int32_t>(field_ids.size()),
		field_ids.data(),
		nullptr
	);
	return field_ids;
}

template<typename CompositeID>
ECSACT_ALWAYS_INLINE std::vector<ecsact_field_id> get_field_ids(
	CompositeID id,
	int32_t     max_size
) {
	std::vector<ecsact_field_id> field_ids;
	auto compo_id = ecsact_id_cast<ecsact_composite_id>(id);
	field_ids.resize(max_size);
	ecsact_meta_get_field_ids(compo_id, max_size, field_ids.data(), &max_size);
	field_ids.resize(max_size);
	return field_ids;
}

template<typename CompositeID>
ECSACT_ALWAYS_INLINE auto get_field_type(
	CompositeID     id,
	ecsact_field_id field_id
) -> ecsact_field_type {
	return ecsact_meta_field_type(
		ecsact_id_cast<ecsact_composite_id>(id),
		field_id
	);
}

ECSACT_ALWAYS_INLINE std::vector<ecsact_system_id> get_system_ids(
	ecsact_package_id package_id
) {
	std::vector<ecsact_system_id> system_ids;
	system_ids.resize(ecsact_meta_count_systems(package_id));
	ecsact_meta_get_system_ids(
		package_id,
		static_cast<int32_t>(system_ids.size()),
		system_ids.data(),
		nullptr
	);
	return system_ids;
}

ECSACT_ALWAYS_INLINE std::vector<ecsact_system_id> get_system_ids(
	ecsact_package_id package_id,
	int32_t           max_size
) {
	std::vector<ecsact_system_id> system_ids;
	system_ids.resize(max_size);
	ecsact_meta_get_system_ids(
		package_id,
		max_size,
		system_ids.data(),
		&max_size
	);
	system_ids.resize(max_size);
	return system_ids;
}

ECSACT_ALWAYS_INLINE std::string system_name(ecsact_system_id system_id) {
	auto name = ecsact_meta_system_name(system_id);
	if(name == nullptr) {
		return {};
	}
	return name;
}

ECSACT_ALWAYS_INLINE std::vector<ecsact_action_id> get_action_ids(
	ecsact_package_id package_id
) {
	std::vector<ecsact_action_id> action_ids;
	action_ids.resize(ecsact_meta_count_actions(package_id));
	ecsact_meta_get_action_ids(
		package_id,
		static_cast<int32_t>(action_ids.size()),
		action_ids.data(),
		nullptr
	);
	return action_ids;
}

ECSACT_ALWAYS_INLINE std::vector<ecsact_action_id> get_action_ids(
	ecsact_package_id package_id,
	int32_t           max_size
) {
	std::vector<ecsact_action_id> action_ids;
	action_ids.resize(max_size);
	ecsact_meta_get_action_ids(
		package_id,
		max_size,
		action_ids.data(),
		&max_size
	);
	action_ids.resize(max_size);
	return action_ids;
}

ECSACT_ALWAYS_INLINE std::string action_name(ecsact_action_id action_id) {
	auto name = ecsact_meta_action_name(action_id);
	if(name == nullptr) {
		return {};
	}
	return name;
}

ECSACT_ALWAYS_INLINE std::vector<ecsact_enum_id> get_enum_ids(
	ecsact_package_id package_id
) {
	std::vector<ecsact_enum_id> enum_ids;
	enum_ids.resize(ecsact_meta_count_enums(package_id));
	ecsact_meta_get_enum_ids(
		package_id,
		static_cast<int32_t>(enum_ids.size()),
		enum_ids.data(),
		nullptr
	);
	return enum_ids;
}

ECSACT_ALWAYS_INLINE std::vector<ecsact_enum_id> get_enum_ids(
	ecsact_package_id package_id,
	int32_t           max_size
) {
	std::vector<ecsact_enum_id> enum_ids;
	enum_ids.resize(max_size);
	ecsact_meta_get_enum_ids(package_id, max_size, enum_ids.data(), &max_size);
	enum_ids.resize(max_size);
	return enum_ids;
}

ECSACT_ALWAYS_INLINE std::string enum_name(ecsact_enum_id enum_id) {
	auto name = ecsact_meta_enum_name(enum_id);
	if(name == nullptr) {
		return {};
	}
	return name;
}

ECSACT_ALWAYS_INLINE std::vector<ecsact_enum_value_id> get_enum_value_ids(
	ecsact_enum_id enum_id,
	int32_t        max_size
) {
	std::vector<ecsact_enum_value_id> enum_value_ids;
	enum_value_ids.resize(max_size);
	ecsact_meta_get_enum_value_ids(
		enum_id,
		max_size,
		enum_value_ids.data(),
		&max_size
	);
	enum_value_ids.resize(max_size);
	return enum_value_ids;
}

ECSACT_ALWAYS_INLINE std::vector<ecsact_enum_value_id> get_enum_value_ids(
	ecsact_enum_id enum_id
) {
	return get_enum_value_ids(enum_id, ecsact_meta_count_enum_values(enum_id));
}

struct enum_value {
	ecsact_enum_value_id id;
	std::string          name;
	std::int32_t         value;
};

ECSACT_ALWAYS_INLINE std::vector<enum_value> get_enum_values(
	ecsact_enum_id enum_id
) {
	std::vector<enum_value> enum_values;
	auto                    size = ecsact_meta_count_enum_values(enum_id);
	enum_values.reserve(size);

	for(auto enum_value_id : get_enum_value_ids(enum_id, size)) {
		enum_values.push_back({
			.id = enum_value_id,
			.name = ecsact_meta_enum_value_name(enum_id, enum_value_id),
			.value = ecsact_meta_enum_value(enum_id, enum_value_id),
		});
	}

	return enum_values;
}

template<typename DeclarationID>
ECSACT_ALWAYS_INLINE std::string decl_full_name(DeclarationID id) {
	auto decl_id = ecsact_id_cast<ecsact_decl_id>(id);
	auto full_name = ecsact_meta_decl_full_name(decl_id);
	if(full_name == nullptr) {
		return {};
	}
	return full_name;
}

template<typename SystemLikeID>
ECSACT_ALWAYS_INLINE std::vector<ecsact_system_id> get_child_system_ids(
	SystemLikeID id
) {
	auto system_like_id = ecsact_id_cast<ecsact_system_like_id>(id);
	std::vector<ecsact_system_id> child_system_ids;
	child_system_ids.resize(ecsact_meta_count_child_systems(system_like_id));
	ecsact_meta_get_child_system_ids(
		system_like_id,
		static_cast<int32_t>(child_system_ids.size()),
		child_system_ids.data(),
		nullptr
	);
	return child_system_ids;
}

ECSACT_ALWAYS_INLINE std::optional<ecsact_system_like_id> get_parent_system_id(
	ecsact_system_id child_system_id
) {
	auto parent_sys_id = ecsact_meta_get_parent_system_id(child_system_id);
	if((int32_t)parent_sys_id == -1) {
		return {};
	}
	return parent_sys_id;
}

ECSACT_ALWAYS_INLINE std::vector<ecsact_system_like_id> get_top_level_systems(
	ecsact_package_id package_id
) {
	std::vector<ecsact_system_like_id> top_sys_like_ids;
	top_sys_like_ids.resize(ecsact_meta_count_top_level_systems(package_id));
	ecsact_meta_get_top_level_systems(
		package_id,
		static_cast<int32_t>(top_sys_like_ids.size()),
		top_sys_like_ids.data(),
		nullptr
	);
	return top_sys_like_ids;
}

ECSACT_ALWAYS_INLINE auto get_all_system_like_ids(ecsact_package_id package_id
) {
	std::vector<ecsact_system_like_id> result;
	auto                               sys_ids = get_system_ids(package_id);
	auto                               act_ids = get_action_ids(package_id);
	result.reserve(sys_ids.size() + act_ids.size());

	result.insert(
		result.end(),
		reinterpret_cast<ecsact_system_like_id*>(sys_ids.data()),
		reinterpret_cast<ecsact_system_like_id*>(sys_ids.data() + sys_ids.size())
	);
	result.insert(
		result.end(),
		reinterpret_cast<ecsact_system_like_id*>(act_ids.data()),
		reinterpret_cast<ecsact_system_like_id*>(act_ids.data() + act_ids.size())
	);

	return result;
}

template<typename SystemLikeID>
ECSACT_ALWAYS_INLINE auto system_capabilities(SystemLikeID id) {
	using result_t =
		std::unordered_map<ecsact_component_like_id, ecsact_system_capability>;

	const auto sys_like_id = ecsact_id_cast<ecsact_system_like_id>(id);
	auto       count = ecsact_meta_system_capabilities_count(sys_like_id);
	std::vector<ecsact_component_like_id> components;
	std::vector<ecsact_system_capability> capabilities;
	components.resize(count);
	capabilities.resize(count);

	ecsact_meta_system_capabilities(
		sys_like_id,
		count,
		components.data(),
		capabilities.data(),
		nullptr
	);

	result_t result;
	result.reserve(count);

	for(decltype(count) i = 0; count > i; ++i) {
		result[components[i]] = capabilities[i];
	}

	return result;
}

template<typename SystemLikeID>
ECSACT_ALWAYS_INLINE auto get_system_generates_ids(SystemLikeID id) {
	auto sys_like_id = ecsact_id_cast<ecsact_system_like_id>(id);
	std::vector<ecsact_system_generates_id> result;
	result.resize(ecsact_meta_count_system_generates_ids(sys_like_id));
	ecsact_meta_system_generates_ids(
		sys_like_id,
		static_cast<int32_t>(result.size()),
		result.data(),
		nullptr
	);
	return result;
}

template<typename SystemLikeID>
ECSACT_ALWAYS_INLINE auto get_system_generates_components(
	SystemLikeID               id,
	ecsact_system_generates_id generates_id
) {
	auto sys_like_id = ecsact_id_cast<ecsact_system_like_id>(id);

	std::vector<ecsact_component_id>    component_ids;
	std::vector<ecsact_system_generate> generate_flags;

	auto count =
		ecsact_meta_count_system_generates_components(sys_like_id, generates_id);

	component_ids.resize(count);
	generate_flags.resize(count);

	ecsact_meta_system_generates_components(
		sys_like_id,
		generates_id,
		count,
		component_ids.data(),
		generate_flags.data(),
		nullptr
	);

	std::unordered_map<ecsact_component_id, ecsact_system_generate> result;
	result.reserve(count);

	for(int i = 0; count > i; ++i) {
		result[component_ids[i]] = generate_flags[i];
	}

	return result;
}

template<typename SystemLikeID>
ECSACT_ALWAYS_INLINE auto system_assoc_ids( //
	SystemLikeID system_id
) -> std::vector<ecsact_system_assoc_id> {
	auto result = std::vector<ecsact_system_assoc_id>{};
	result.resize(32);
	auto assoc_count = int32_t{};

	ecsact_meta_system_assoc_ids(
		ecsact_id_cast<ecsact_system_like_id>(system_id),
		static_cast<int32_t>(result.size()),
		result.data(),
		&assoc_count
	);

	result.resize(assoc_count);

	return result;
}

template<typename SystemLikeID>
ECSACT_ALWAYS_INLINE auto system_assoc_component_id( //
	SystemLikeID           system_id,
	ecsact_system_assoc_id assoc_id
) -> ecsact_component_like_id {
	return ecsact_meta_system_assoc_component_id(
		ecsact_id_cast<ecsact_system_like_id>(system_id),
		assoc_id
	);
}

template<typename SystemLikeID>
ECSACT_ALWAYS_INLINE auto system_assoc_fields(
	SystemLikeID           system_id,
	ecsact_system_assoc_id assoc_id
) -> std::vector<ecsact_field_id> {
	auto result = std::vector<ecsact_field_id>{};
	result.resize(32);
	auto fields_count = int32_t{};
	ecsact_meta_system_assoc_fields(
		system_id,
		assoc_id,
		result.size(),
		result.data(),
		&fields_count
	);
	result.resize(fields_count);
	return result;
}

template<typename SystemLikeID>
ECSACT_ALWAYS_INLINE auto system_assoc_capabilities(
	SystemLikeID           id,
	ecsact_system_assoc_id assoc_id
) {
	using result_t =
		std::vector<std::pair<ecsact_component_like_id, ecsact_system_capability>>;

	const auto sys_like_id = ecsact_id_cast<ecsact_system_like_id>(id);
	auto       count = ecsact_meta_system_assoc_capabilities_count(sys_like_id);
	std::vector<ecsact_component_like_id> components;
	std::vector<ecsact_system_capability> capabilities;
	components.resize(count);
	capabilities.resize(count);

	ecsact_meta_system_assoc_capabilities(
		sys_like_id,
		assoc_id,
		count,
		components.data(),
		capabilities.data(),
		nullptr
	);

	result_t result;
	result.resize(count);

	for(decltype(count) i = 0; count > i; ++i) {
		result[i] = {components[i], capabilities[i]};
	}

	return result;
}

template<typename SystemLikeId>
auto system_notify_settings_count( //
	SystemLikeId sys_like_id
) -> int32_t {
	return ecsact_meta_system_notify_settings_count(
		ecsact_id_cast<ecsact_system_like_id>(sys_like_id)
	);
}

template<typename SystemLikeId>
auto system_notify_settings( //
	SystemLikeId id
) {
	const auto sys_like_id = ecsact_id_cast<ecsact_system_like_id>(id);

	using result_t =
		std::unordered_map<ecsact_component_like_id, ecsact_system_notify_setting>;

	auto count = ecsact_meta_system_notify_settings_count(sys_like_id);
	auto components = std::vector<ecsact_component_like_id>{};
	auto notify_settings = std::vector<ecsact_system_notify_setting>{};
	components.resize(count);
	notify_settings.resize(count);

	ecsact_meta_system_notify_settings(
		sys_like_id,
		count,
		components.data(),
		notify_settings.data(),
		nullptr
	);

	result_t result;
	result.reserve(count);

	for(decltype(count) i = 0; count > i; ++i) {
		result[components[i]] = notify_settings[i];
	}

	return result;
}

template<typename CompositeID>
ECSACT_ALWAYS_INLINE auto field_name( //
	CompositeID     id,
	ecsact_field_id field_id
) -> std::string {
	return ecsact_meta_field_name(
		ecsact_id_cast<ecsact_composite_id>(id),
		field_id
	);
}

ECSACT_ALWAYS_INLINE auto main_package() -> std::optional<ecsact_package_id> {
	auto main_pkg_id = ecsact_meta_main_package();
	if(main_pkg_id == static_cast<ecsact_package_id>(-1)) {
		return std::nullopt;
	}
	return main_pkg_id;
}

template<typename SystemLikeID>
ECSACT_ALWAYS_INLINE auto get_system_parallel_execution( //
	SystemLikeID id
) -> ecsact_parallel_execution {
	return ecsact_meta_get_system_parallel_execution(
		ecsact_id_cast<ecsact_system_like_id>(id)
	);
}

} // namespace ecsact::meta
