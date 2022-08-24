#ifndef ECSACT_RUNTIME_DYNAMIC_H
#define ECSACT_RUNTIME_DYNAMIC_H

#include <stdlib.h>

#include "ecsact/runtime/common.h"
#include "ecsact/runtime/definitions.h"

#ifndef ECSACT_DYNAMIC_API_VISIBILITY
#	ifdef ECSACT_DYNAMIC_API_LOAD_AT_RUNTIME
#		define ECSACT_DYNAMIC_API_VISIBILITY
#	else
#		ifdef ECSACT_DYNAMIC_API_EXPORT
#			ifdef _WIN32
#				define ECSACT_DYNAMIC_API_VISIBILITY __declspec(dllexport)
#			else
#				define ECSACT_DYNAMIC_API_VISIBILITY __attribute__((visibility("default")))
#			endif
#		else
#			ifdef _WIN32
#				define ECSACT_DYNAMIC_API_VISIBILITY __declspec(dllimport)
#			else
#				define ECSACT_DYNAMIC_API_VISIBILITY
#			endif
#		endif
#	endif
#endif // ECSACT_DYNAMIC_API_VISIBILITY

#ifndef ECSACT_DYNAMIC_API
#	ifdef __cplusplus
#		define ECSACT_DYNAMIC_API extern "C" ECSACT_DYNAMIC_API_VISIBILITY
#	else
#		define ECSACT_DYNAMIC_API extern ECSACT_DYNAMIC_API_VISIBILITY
# endif
#endif // ECSACT_DYNAMIC_API

#ifndef ECSACT_DYNAMIC_API_FN
#	ifdef ECSACT_DYNAMIC_API_LOAD_AT_RUNTIME
#		define ECSACT_DYNAMIC_API_FN(ret, name) ECSACT_DYNAMIC_API ret (*name)
#	else
#		define ECSACT_DYNAMIC_API_FN(ret, name) ECSACT_DYNAMIC_API ret name
#	endif
#endif // ECSACT_DYNAMIC_API_FN

/**
 * Get the action data. The caller must allocate the memory for the action data.
 * 
 * NOTE: It is considered an error to call this method on a non-action execution
 * context.
 */
ECSACT_DYNAMIC_API_FN(void, ecsact_system_execution_context_action)
	( ecsact_system_execution_context*  context
	, void*                             out_action_data
	);

/**
 * Add new component to the entity currently being processed by the system.
 * 
 * Only available if has one of these capabilities: 
 *  - `ECSACT_SYS_CAP_ADDS`
 */
ECSACT_DYNAMIC_API_FN(void, ecsact_system_execution_context_add)
	( ecsact_system_execution_context*  context
	, ecsact_component_id               component_id
	, const void*                       component_data
	);

/**
 * Remove existing component from the entity currently being processed by the
 * system.
 * 
 * Only available if has one of these capabilities:
 *  - `ECSACT_SYS_CAP_REMOVES`
 */
ECSACT_DYNAMIC_API_FN(void, ecsact_system_execution_context_remove)
	( ecsact_system_execution_context*  context
	, ecsact_component_id               component_id
	);

/**
 * Get data for component with ID `component_id`. caller must allocate the
 * memory required for the component data.
 *
 * NOTE: It is considered an error if `get` is called without first checking 
 * `has` when the system only has 'optional' capabilities.
 *
 * Only available if has one of these capabilities:
 *  - `ECSACT_SYS_CAP_READONLY`
 *  - `ECSACT_SYS_CAP_READWRITE`
 *  - `ECSACT_SYS_CAP_OPTIONAL_READONLY`
 *  - `ECSACT_SYS_CAP_OPTIONAL_READWRITE`
 */
ECSACT_DYNAMIC_API_FN(void, ecsact_system_execution_context_get)
	( ecsact_system_execution_context*  context
	, ecsact_component_id               component_id
	, void*                             out_component_data
	);

/**
 * Only available if has one of these capabilities:
 *  - `ECSACT_SYS_CAP_WRITEONLY`
 *  - `ECSACT_SYS_CAP_READWRITE`
 *  - `ECSACT_SYS_CAP_OPTIONAL_WRITEONLY`
 *  - `ECSACT_SYS_CAP_OPTIONAL_READWRITE`
 */
ECSACT_DYNAMIC_API_FN(void, ecsact_system_execution_context_update)
	( ecsact_system_execution_context*  context
	, ecsact_component_id               component_id
	, const void*                       component_data
	);

/**
 * Check if the component with ID `component_id` exists on the entity 
 * currently being processed  by the system.
 * 
 * Only available if has one of these capabilities:
 *  - `ECSACT_SYS_CAP_OPTIONAL_READONLY` 
 *  - `ECSACT_SYS_CAP_OPTIONAL_WRITEONLY`
 *  - `ECSACT_SYS_CAP_OPTIONAL_READWRITE`
 */
ECSACT_DYNAMIC_API_FN(bool, ecsact_system_execution_context_has)
	( ecsact_system_execution_context*  context
	, ecsact_component_id               component_id
	);

/**
 * Generate a new entity with specified components.
 * 
 * @param component_count length of `component_ids` and `components_data`
 * @param component_ids list of component ids associatd with `components_data`.
 * @param components_data list of component data associated with 
 *        `component_ids`.
 * 
 * @note Only available if the system is a generator. @see 
 * `ecsact_add_system_generate_component_set`
 */
ECSACT_DYNAMIC_API_FN(void, ecsact_system_execution_context_generate)
	( ecsact_system_execution_context*  context
	, int                               component_count
	, ecsact_component_id*              component_ids
	, const void**                      components_data
	);

/**
 * Get the parent system exeuction context.
 * 
 * Only available if the currently executing system is a nested system.
 */
ECSACT_DYNAMIC_API_FN(const ecsact_system_execution_context*, ecsact_system_execution_context_parent)
	( ecsact_system_execution_context*  context
	);

/**
 * Check if two execution contexts refer to the same entity. This is useful when
 * comparing against parent execution context to skip
 * 
 * NOTE: This is will eventually be deprecated in favour of a language feature
 *       to skip matching parent system entities.
 */
ECSACT_DYNAMIC_API_FN(bool, ecsact_system_execution_context_same)
	( const ecsact_system_execution_context*
	, const ecsact_system_execution_context*
	);

/**
 * Get execution context for a different entity
 * 
 * Only available if has one of these capabilities:
 *  - `ECSACT_SYS_CAP_OPTIONAL_READONLY` 
 *  - `ECSACT_SYS_CAP_OPTIONAL_WRITEONLY`
 *  - `ECSACT_SYS_CAP_OPTIONAL_READWRITE`
 */
ECSACT_DYNAMIC_API_FN(ecsact_system_execution_context*, ecsact_system_execution_context_other)
	( ecsact_system_execution_context*  context
	, ecsact_entity_id                  entity_id
	);

/**
 * Get the current system/action ID
 */
ECSACT_DYNAMIC_API_FN(ecsact_system_id, ecsact_system_execution_context_id)
	( ecsact_system_execution_context* context
	);

ECSACT_DYNAMIC_API_FN(ecsact_package_id, ecsact_create_package)
	( bool         main_package
	, const char*  package_name
	, int32_t      package_name_len
	);

ECSACT_DYNAMIC_API_FN(void, ecsact_set_package_source_file_path)
	( ecsact_package_id  package_id
	, const char*        source_file_path
	, int32_t            source_file_path_len
	);

ECSACT_DYNAMIC_API_FN(void, ecsact_add_dependency)
	( ecsact_package_id target
	, ecsact_package_id dependency
	);

ECSACT_DYNAMIC_API_FN(void, ecsact_remove_dependency)
	( ecsact_package_id target
	, ecsact_package_id dependency
	);

ECSACT_DYNAMIC_API_FN(void, ecsact_destroy_package)
	( ecsact_package_id package_id
	);

ECSACT_DYNAMIC_API_FN(ecsact_system_id, ecsact_create_system)
	( ecsact_package_id  owner
	, const char*        system_name
	, int32_t            system_name_len
	);

ECSACT_DYNAMIC_API_FN(void, ecsact_add_child_system)
	( ecsact_system_like_id  parent
	, ecsact_system_id       child
	);

ECSACT_DYNAMIC_API_FN(void, ecsact_remove_child_system)
	( ecsact_system_like_id  parent
	, ecsact_system_id       child
	);

/**
 * Systems execute in the order they are registered in. If the order needs to be
 * adjusted this method can be used to move systems before or after other
 * systems.
 */
ECSACT_DYNAMIC_API_FN(bool, ecsact_reorder_system)
	( ecsact_system_like_id  target_system_id
	, ecsact_system_like_id  relative_system_id
	, bool                   target_before_relative
	);

/**
 * Sets the system execution implementation function. If one is already set it
 * gets overwritten.
 * 
 * NOTE: ONLY `ecsact_system_execution_context_*` functions are allowed to be
 *       called while a system is executing.
 */
ECSACT_DYNAMIC_API_FN(bool, ecsact_set_system_execution_impl)
	( ecsact_system_like_id         system_id
	, ecsact_system_execution_impl  system_exec_impl
	);

ECSACT_DYNAMIC_API_FN(ecsact_action_id, ecsact_create_action)
	( ecsact_package_id  owner
	, const char*        action_name
	, int32_t            action_name_len
	);

/**
 * Create new component
 * @returns unique component ID for newly created component
 */
ECSACT_DYNAMIC_API_FN(ecsact_component_id, ecsact_create_component)
	( ecsact_package_id  owner
	, const char*        component_name
	, int32_t            component_name_len
	);

/**
 * @param declaration_id - Component or action ID
 * @returns field index for declaration
 */
ECSACT_DYNAMIC_API_FN(ecsact_field_id, ecsact_add_field)
	( ecsact_composite_id  composite_id
	, ecsact_field_type    field_type
	, const char*          field_name
	, int32_t              field_name_len
	);

/**
 * @param declaration_id - Component or action ID
 */
ECSACT_DYNAMIC_API_FN(void, ecsact_remove_field)
	( ecsact_composite_id  composite_id
	, ecsact_field_id      field_id
	);

ECSACT_DYNAMIC_API_FN(void, ecsact_destroy_component)
	( ecsact_component_id component_id
	);

ECSACT_DYNAMIC_API_FN(ecsact_enum_id, ecsact_create_enum)
	( ecsact_package_id  owner
	, const char*        enum_name
	, int32_t            enum_name_len
	);

ECSACT_DYNAMIC_API_FN(void, ecsact_destroy_enum)
	( ecsact_enum_id
	);

ECSACT_DYNAMIC_API_FN(ecsact_enum_value_id, ecsact_add_enum_value)
	( ecsact_enum_id  enum_id
	, int32_t         value
	, const char*     value_name
	, int32_t         value_name_len
	);

ECSACT_DYNAMIC_API_FN(void, ecsact_remove_enum_value)
	( ecsact_enum_id        enum_id
	, ecsact_enum_value_id  value_iud
	);

ECSACT_DYNAMIC_API_FN(void, ecsact_set_system_capability)
	( ecsact_system_like_id
	, ecsact_component_id
	, ecsact_system_capability
	);

ECSACT_DYNAMIC_API_FN(void, ecsact_unset_system_capability)
	( ecsact_system_like_id
	, ecsact_component_id
	);

ECSACT_DYNAMIC_API_FN(void, ecsact_set_system_association_capability)
	( ecsact_system_like_id
	, ecsact_component_id  
	, ecsact_field_id
	, ecsact_component_id
	, ecsact_system_capability
	);

ECSACT_DYNAMIC_API_FN(void, ecsact_unset_system_association_capability)
	( ecsact_system_like_id
	, ecsact_component_id  
	, ecsact_field_id
	, ecsact_component_id
	);

/**
 * Adds a set of component ids that this system may use to generate new
 * entities.
 * 
 * @note there is no way to remove a system generate component set. If it is
 *       a requirement, however, you may destroy the system and re-create it
 *       without the generate component set(s).
 * 
 * @return returns -1 if unsuccessful otherwise returns unspecified value
 */
ECSACT_DYNAMIC_API_FN(int, ecsact_add_system_generate_component_set)
	( ecsact_system_like_id    system_id
	, int                      components_count
	, ecsact_component_id*     component_ids
	, ecsact_system_generate*  component_generate_flags
	);

ECSACT_DYNAMIC_API_FN(bool, ecsact_register_component)
	( ecsact_registry_id
	, ecsact_component_id
	);

ECSACT_DYNAMIC_API_FN(bool, ecsact_register_system)
	( ecsact_registry_id
	, ecsact_system_id
	);

ECSACT_DYNAMIC_API_FN(bool, ecsact_register_action)
	( ecsact_registry_id
	, ecsact_action_id
	);

#define FOR_EACH_ECSACT_DYNAMIC_API_FN(fn, ...)\
	fn(ecsact_system_execution_context_action, __VA_ARGS__);\
	fn(ecsact_system_execution_context_add, __VA_ARGS__);\
	fn(ecsact_system_execution_context_remove, __VA_ARGS__);\
	fn(ecsact_system_execution_context_get, __VA_ARGS__);\
	fn(ecsact_system_execution_context_update, __VA_ARGS__);\
	fn(ecsact_system_execution_context_has, __VA_ARGS__);\
	fn(ecsact_system_execution_context_generate, __VA_ARGS__);\
	fn(ecsact_system_execution_context_parent, __VA_ARGS__);\
	fn(ecsact_system_execution_context_same, __VA_ARGS__);\
	fn(ecsact_system_execution_context_other, __VA_ARGS__);\
	fn(ecsact_system_execution_context_id, __VA_ARGS__);\
	fn(ecsact_create_package, __VA_ARGS__);\
	fn(ecsact_set_package_source_file_path, __VA_ARGS__);\
	fn(ecsact_add_dependency, __VA_ARGS__);\
	fn(ecsact_remove_dependency, __VA_ARGS__);\
	fn(ecsact_destroy_package, __VA_ARGS__);\
	fn(ecsact_create_system, __VA_ARGS__);\
	fn(ecsact_add_child_system, __VA_ARGS__);\
	fn(ecsact_remove_child_system, __VA_ARGS__);\
	fn(ecsact_reorder_system, __VA_ARGS__);\
	fn(ecsact_set_system_execution_impl, __VA_ARGS__);\
	fn(ecsact_create_action, __VA_ARGS__);\
	fn(ecsact_create_component, __VA_ARGS__);\
	fn(ecsact_add_field, __VA_ARGS__);\
	fn(ecsact_remove_field, __VA_ARGS__);\
	fn(ecsact_destroy_component, __VA_ARGS__);\
	fn(ecsact_create_enum, __VA_ARGS__);\
	fn(ecsact_destroy_enum, __VA_ARGS__);\
	fn(ecsact_add_enum_value, __VA_ARGS__);\
	fn(ecsact_remove_enum_value, __VA_ARGS__);\
	fn(ecsact_set_system_capability, __VA_ARGS__);\
	fn(ecsact_unset_system_capability, __VA_ARGS__);\
	fn(ecsact_set_system_association_capability, __VA_ARGS__);\
	fn(ecsact_unset_system_association_capability, __VA_ARGS__);\
	fn(ecsact_add_system_generate_component_set, __VA_ARGS__);\
	fn(ecsact_register_component, __VA_ARGS__);\
	fn(ecsact_register_system, __VA_ARGS__);\
	fn(ecsact_register_action, __VA_ARGS__)

#undef ECSACT_DYNAMIC_API
#undef ECSACT_DYNAMIC_API_FN
#endif // ECSACT_RUNTIME_DYNAMIC_H
