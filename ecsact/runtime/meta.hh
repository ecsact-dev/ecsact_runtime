#pragma once

#include <vector>
#include <string>
#include <optional>
#include <filesystem>
#include <unordered_map>
#include "ecsact/runtime/meta.h"

namespace ecsact::meta {

	inline std::vector<ecsact_package_id> get_package_ids() {
		std::vector<ecsact_package_id> package_ids;
		package_ids.resize(ecsact_meta_count_packages());
		ecsact_meta_get_package_ids(
			static_cast<int32_t>(package_ids.size()),
			package_ids.data(),
			nullptr
		);
		return package_ids;
	}

	inline std::vector<ecsact_package_id> get_package_ids
		( int32_t max_size
		)
	{
		std::vector<ecsact_package_id> package_ids;
		package_ids.resize(max_size);
		ecsact_meta_get_package_ids(
			max_size,
			package_ids.data(),
			&max_size
		);
		package_ids.resize(max_size);
		return package_ids;
	}

	inline std::filesystem::path package_file_path
		( ecsact_package_id package_id
		)
	{
		auto file_path = ecsact_meta_package_file_path(package_id);
		if(file_path == nullptr) {
			return {};
		}
		return std::filesystem::path(file_path);
	}

	inline std::string package_name
		( ecsact_package_id package_id
		)
	{
		auto pkg_name = ecsact_meta_package_name(package_id);
		if(pkg_name == nullptr) {
			return {};
		}
		return pkg_name;
	}

	inline std::vector<ecsact_package_id> get_dependencies
		( ecsact_package_id package_id
		)
	{
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

	inline std::vector<ecsact_package_id> get_dependencies
		( ecsact_package_id  package_id
		, int32_t            max_size
		)
	{
		std::vector<ecsact_package_id> dep_ids;
		dep_ids.resize(max_size);
		ecsact_meta_get_dependencies(
			package_id,
			max_size,
			dep_ids.data(),
			&max_size
		);
		dep_ids.resize(max_size);
		return dep_ids;
	}

	inline std::vector<ecsact_component_id> get_component_ids
		( ecsact_package_id package_id
		)
	{
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

	inline std::vector<ecsact_component_id> get_component_ids
		( ecsact_package_id  package_id
		, int32_t            max_size
		)
	{
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

	inline std::string component_name
		( ecsact_component_id component_id
		)
	{
		auto comp_name = ecsact_meta_component_name(component_id);
		if(comp_name == nullptr) {
			return {};
		}
		return comp_name;
	}

	inline std::vector<ecsact_transient_id> get_transient_ids
		( ecsact_package_id package_id
		)
	{
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

	inline std::vector<ecsact_transient_id> get_transient_ids
		( ecsact_package_id  package_id
		, int32_t            max_size
		)
	{
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

	inline std::string transient_name
		( ecsact_transient_id transient_id
		)
	{
		auto comp_name = ecsact_meta_transient_name(transient_id);
		if(comp_name == nullptr) {
			return {};
		}
		return comp_name;
	}

	template<typename CompositeID>
	inline std::vector<ecsact_field_id> get_field_ids
		( CompositeID id
		)
	{
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
	inline std::vector<ecsact_field_id> get_field_ids
		( CompositeID  id
		, int32_t      max_size
		)
	{
		std::vector<ecsact_field_id> field_ids;
		auto compo_id = ecsact_id_cast<ecsact_composite_id>(id);
		field_ids.resize(max_size);
		ecsact_meta_get_field_ids(
			compo_id,
			max_size,
			field_ids.data(),
			&max_size
		);
		field_ids.resize(max_size);
		return field_ids;
	}

	inline std::vector<ecsact_system_id> get_system_ids
		( ecsact_package_id package_id
		)
	{
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

	inline std::vector<ecsact_system_id> get_system_ids
		( ecsact_package_id  package_id
		, int32_t            max_size
		)
	{
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

	inline std::string system_name
		( ecsact_system_id system_id
		)
	{
		auto name = ecsact_meta_system_name(system_id);
		if(name == nullptr) {
			return {};
		}
		return name;
	}

	inline std::vector<ecsact_action_id> get_action_ids
		( ecsact_package_id package_id
		)
	{
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

	inline std::vector<ecsact_action_id> get_action_ids
		( ecsact_package_id  package_id
		, int32_t            max_size
		)
	{
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

	inline std::string action_name
		( ecsact_action_id action_id
		)
	{
		auto name = ecsact_meta_action_name(action_id);
		if(name == nullptr) {
			return {};
		}
		return name;
	}

	inline std::vector<ecsact_enum_id> get_enum_ids
		( ecsact_package_id package_id
		)
	{
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

	inline std::vector<ecsact_enum_id> get_enum_ids
		( ecsact_package_id  package_id
		, int32_t            max_size
		)
	{
		std::vector<ecsact_enum_id> enum_ids;
		enum_ids.resize(max_size);
		ecsact_meta_get_enum_ids(
			package_id,
			max_size,
			enum_ids.data(),
			&max_size
		);
		enum_ids.resize(max_size);
		return enum_ids;
	}

	template<typename DeclarationID>
	inline std::string decl_full_name
		( DeclarationID id
		)
	{
		auto decl_id = ecsact_id_cast<ecsact_decl_id>(id);
		auto full_name = ecsact_meta_decl_full_name(decl_id);
		if(full_name == nullptr) {
			return {};
		}
		return full_name;
	}

	template<typename SystemLikeID>
	inline std::vector<ecsact_system_id> get_child_system_ids
		( SystemLikeID id
		)
	{
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

	inline std::optional<ecsact_system_like_id> get_parent_system_id
		( ecsact_system_id child_system_id
		)
	{
		auto parent_sys_id = ecsact_meta_get_parent_system_id(child_system_id);
		if((int32_t)parent_sys_id == -1) {
			return {};
		}
		return parent_sys_id;
	}

	inline std::vector<ecsact_system_like_id> get_top_level_systems
		( ecsact_package_id package_id
		)
	{
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

	template<typename SystemLikeID>
	inline auto system_capabilities
		( SystemLikeID id
		)
	{
		using result_t = std::unordered_map
			< ecsact_component_like_id
			, ecsact_system_capability
			>;

		const auto sys_like_id = ecsact_id_cast<ecsact_system_like_id>(id);
		auto count = ecsact_meta_system_capabilities_count(sys_like_id);
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

		for(decltype(count) i=0; count > i; ++i) {
			result[components[i]] = capabilities[i];
		}

		return result;
	}

}
