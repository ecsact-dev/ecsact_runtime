#ifndef ECSACT_RUNTIME_META_H
#define ECSACT_RUNTIME_META_H

#include <stdlib.h>

#include "ecsact/runtime/common.h"
#include "ecsact/runtime/definitions.h"

#ifndef ECSACT_META_API_VISIBILITY
#	ifdef ECSACT_META_API_LOAD_AT_RUNTIME
#		define ECSACT_META_API_VISIBILITY
#	else
#		ifdef ECSACT_META_API_EXPORT
#			ifdef _WIN32
#				define ECSACT_META_API_VISIBILITY __declspec(dllexport)
#			else
#				define ECSACT_META_API_VISIBILITY __attribute__((visibility("default")))
#			endif
#		else
#			ifdef _WIN32
#				define ECSACT_META_API_VISIBILITY __declspec(dllimport)
#			else
#				define ECSACT_META_API_VISIBILITY
#			endif
#		endif
#	endif
#endif // ECSACT_META_API_VISIBILITY

#ifndef ECSACT_META_API
#	ifdef __cplusplus
#		define ECSACT_META_API extern "C" ECSACT_META_API_VISIBILITY
#	else
#		define ECSACT_META_API extern ECSACT_META_API_VISIBILITY
# endif
#endif // ECSACT_META_API

#ifndef ECSACT_META_API_FN
#	ifdef ECSACT_META_API_LOAD_AT_RUNTIME
#		define ECSACT_META_API_FN(ret, name) ECSACT_META_API ret (*name)
#	else
#		define ECSACT_META_API_FN(ret, name) ECSACT_META_API ret name
#	endif
#endif // ECSACT_META_API_FN

/**
 * @returns number of packages
 */
ECSACT_META_API_FN(int32_t, ecsact_meta_count_packages)();

ECSACT_META_API_FN(void, ecsact_meta_get_package_ids)
	( int32_t             max_package_count
	, ecsact_package_id*  out_package_ids
	, int32_t*            out_package_count
	);

ECSACT_META_API_FN(const char*, ecsact_meta_package_name)
	( ecsact_package_id package_id
	);

/**
 * @param package_id the ID given to the package when created
 * @returns the ecsact file path for the package
 */
ECSACT_META_API_FN(const char*, ecsact_meta_package_file_path)
	( ecsact_package_id package_id
	);

ECSACT_META_API_FN(int32_t, ecsact_meta_count_dependencies)
	( ecsact_package_id package_id
	);

ECSACT_META_API_FN(void, ecsact_meta_get_dependencies)
	( ecsact_package_id   package_id
	, int32_t             max_dependency_count
	, ecsact_package_id*  out_package_ids
	, int32_t*            out_dependencies_count
	);

/**
 * Get number of components in package
 */
ECSACT_META_API_FN(int32_t, ecsact_meta_count_components)
	( ecsact_package_id package_id
	);

ECSACT_META_API_FN(void, ecsact_meta_get_component_ids)
	( ecsact_package_id     package_id
	, int32_t               max_component_count
	, ecsact_component_id*  out_component_ids
	, int32_t*              out_component_count
	);

/**
 * @param declaration_id component or action ID
 * @returns number of fields declaration has
 */
ECSACT_META_API_FN(int32_t, ecsact_meta_count_fields)
	( ecsact_composite_id composite_id
	);

ECSACT_META_API_FN(void, ecsact_meta_get_field_ids)
	( ecsact_composite_id  composite_id
	, int32_t              max_field_count
	, ecsact_field_id*     out_field_ids
	, int32_t*             out_field_ids_count
	);

ECSACT_META_API_FN(const char*, ecsact_meta_field_name)
	( ecsact_composite_id  composite_id
	, ecsact_field_id      field_id
	);

ECSACT_META_API_FN(ecsact_field_type, ecsact_meta_field_type)
	( ecsact_composite_id  composite_id
	, ecsact_field_id      field_id
	);

ECSACT_META_API_FN(int32_t, ecsact_meta_count_systems)
	( ecsact_package_id package_id
	);

ECSACT_META_API_FN(void, ecsact_meta_get_system_ids)
	( ecsact_package_id  package_id
	, int32_t            max_system_count
	, ecsact_system_id*  out_system_ids
	, int32_t*           out_system_count
	);

ECSACT_META_API_FN(int32_t, ecsact_meta_count_actions)
	( ecsact_package_id package_id
	);

ECSACT_META_API_FN(void, ecsact_meta_get_action_ids)
	( ecsact_package_id  package_id
	, int32_t            max_action_count
	, ecsact_action_id*  out_action_ids
	, int32_t*           out_action_count
	);

ECSACT_META_API_FN(int32_t, ecsact_meta_count_enums)
	( ecsact_package_id package_id
	);

ECSACT_META_API_FN(void, ecsact_meta_get_enum_ids)
	( ecsact_package_id  package_id
	, int32_t            max_enum_count
	, ecsact_enum_id*    out_enum_ids
	, int32_t*           out_enum_count
	);

/**
 * The storage type of the enum can be an unsigned or signed integer based on
 * the enum values. The storage type will be the smallest integer that can hold
 * all possible valid enum values.
 * @returns storage type that can be used to represent the enum
 */
ECSACT_META_API_FN(ecsact_builtin_type, ecsact_meta_enum_storage_type)
	( ecsact_enum_id enum_id
	);

ECSACT_META_API_FN(int32_t, ecsact_meta_count_enum_values)
	( ecsact_enum_id enum_id
	);

ECSACT_META_API_FN(void, ecsact_meta_get_enum_value_ids)
	( ecsact_enum_id         enum_id
	, int32_t                max_enum_value_ids
	, ecsact_enum_value_id*  out_enum_value_ids
	, int32_t*               out_enum_values_count
	);

ECSACT_META_API_FN(const char*, ecsact_meta_enum_name)
	( ecsact_enum_id enum_id
	);

ECSACT_META_API_FN(const char*, ecsact_meta_enum_value_name)
	( ecsact_enum_id        enum_id
	, ecsact_enum_value_id  value_id
	);

ECSACT_META_API_FN(int32_t, ecsact_meta_enum_value)
	( ecsact_enum_id        enum_id
	, ecsact_enum_value_id  value_id
	);

/**
 * Get a registry name. May or may not return originally set name when creating
 * the registry. Typically not returned in release/optimized build.
 */
ECSACT_META_API_FN(const char*, ecsact_meta_registry_name)
	( ecsact_registry_id
	);

/**
 * Get the component data size
 */
ECSACT_META_API_FN(size_t, ecsact_meta_component_size)
	( ecsact_component_id
	);

/**
 * Get the component name. May or may not return originally set name when
 * creating a component. Typically not returned in release/optimized build.
 */
ECSACT_META_API_FN(const char*, ecsact_meta_component_name)
	( ecsact_component_id
	);

/**
 * Get the action data size
 */
ECSACT_META_API_FN(size_t, ecsact_meta_action_size)
	( ecsact_system_id
	);

/**
 * Get the action name. May or may not return originally set name when
 * creating a action. Typically not returned in release/optimized build.
 */
ECSACT_META_API_FN(const char*, ecsact_meta_action_name)
	( ecsact_action_id
	);

/**
 * Get the system name. May or may not return originally set name when
 * creating a system. Typically not returned in release/optimized build.
 */
ECSACT_META_API_FN(const char*, ecsact_meta_system_name)
	( ecsact_system_id
	);

ECSACT_META_API_FN(const char*, ecsact_meta_decl_full_name)
	( ecsact_decl_id
	);

/**
 * Get the count of capabilities for a particular system/action.
 */
ECSACT_META_API_FN(int32_t, ecsact_meta_system_capabilities_count)
	( ecsact_system_like_id system_id
	);

/**
 * Get all the system/action capabilities
 * @param out_capability_component_ids - Output for component ids. Should be 
 * allocated to the size returned from `ecsact_meta_system_capabilities_count`
 * @param out_capabilities - Output for capabilities. Should be allocated to the
 * size returned from `ecsact_meta_system_capabilities_count`
 */
ECSACT_META_API_FN(void, ecsact_meta_system_capabilities)
	( ecsact_system_like_id      system_id
	, int32_t                    max_capabilities_count
	, ecsact_component_id*       out_capability_component_ids
	, ecsact_system_capability*  out_capabilities
	, int32_t*                   out_capabilities_count
	);

#define FOR_EACH_ECSACT_META_API_FN(fn, ...)\
	fn(ecsact_meta_get_package_ids, __VA_ARGS__);\
	fn(ecsact_meta_package_name, __VA_ARGS__);\
	fn(ecsact_meta_package_file_path, __VA_ARGS__);\
	fn(ecsact_meta_count_dependencies, __VA_ARGS__);\
	fn(ecsact_meta_get_dependencies, __VA_ARGS__);\
	fn(ecsact_meta_count_components, __VA_ARGS__);\
	fn(ecsact_meta_get_component_ids, __VA_ARGS__);\
	fn(ecsact_meta_count_fields, __VA_ARGS__);\
	fn(ecsact_meta_get_field_ids, __VA_ARGS__);\
	fn(ecsact_meta_field_name, __VA_ARGS__);\
	fn(ecsact_meta_field_type, __VA_ARGS__);\
	fn(ecsact_meta_count_systems, __VA_ARGS__);\
	fn(ecsact_meta_get_system_ids, __VA_ARGS__);\
	fn(ecsact_meta_count_actions, __VA_ARGS__);\
	fn(ecsact_meta_get_action_ids, __VA_ARGS__);\
	fn(ecsact_meta_count_enums, __VA_ARGS__);\
	fn(ecsact_meta_get_enum_ids, __VA_ARGS__);\
	fn(ecsact_meta_enum_storage_type, __VA_ARGS__);\
	fn(ecsact_meta_count_enum_values, __VA_ARGS__);\
	fn(ecsact_meta_get_enum_value_ids, __VA_ARGS__);\
	fn(ecsact_meta_enum_name, __VA_ARGS__);\
	fn(ecsact_meta_enum_value_name, __VA_ARGS__);\
	fn(ecsact_meta_enum_value, __VA_ARGS__);\
	fn(ecsact_meta_registry_name, __VA_ARGS__);\
	fn(ecsact_meta_component_size, __VA_ARGS__);\
	fn(ecsact_meta_component_name, __VA_ARGS__);\
	fn(ecsact_meta_action_size, __VA_ARGS__);\
	fn(ecsact_meta_action_name, __VA_ARGS__);\
	fn(ecsact_meta_system_name, __VA_ARGS__);\
	fn(ecsact_meta_decl_full_name, __VA_ARGS__);\
	fn(ecsact_meta_system_capabilities_count, __VA_ARGS__);\
	fn(ecsact_meta_system_capabilities, __VA_ARGS__)

#endif // ECSACT_RUNTIME_META_H
