#ifndef ECSACT_RUNTIME_DYNAMIC_H
#define ECSACT_RUNTIME_DYNAMIC_H

#include <stdbool.h>
#include <stdint.h>

#include "ecsact/runtime/common.h"
#include "ecsact/runtime/definitions.h"

#ifdef ECSACT_DYNAMIC_API_VISIBILITY
#	error "ECSACT_DYNAMIC_API_VISIBILITY define is deprecated"
#endif

#ifndef ECSACT_DYNAMIC_API_FN
#	if defined(ECSACT_DYNAMIC_API)
#		define ECSACT_DYNAMIC_API_FN(ret, name) ECSACT_DYNAMIC_API ret name
#	elif defined(ECSACT_DYNAMIC_API_LOAD_AT_RUNTIME)
#		define ECSACT_DYNAMIC_API_FN(ret, name) ECSACT_EXTERN ret(*name)
#	elif defined(ECSACT_DYNAMIC_API_EXPORT)
#		define ECSACT_DYNAMIC_API_FN(ret, name) \
			ECSACT_EXTERN ECSACT_EXPORT(#name) ret name
#	else
#		define ECSACT_DYNAMIC_API_FN(ret, name) \
			ECSACT_EXTERN ECSACT_IMPORT("env", #name) ret name
#	endif
#endif // ECSACT_DYNAMIC_API_FN

/**
 * Get the action data. The caller must allocate the memory for the action data.
 *
 * NOTE: It is considered an error to call this method on a non-action execution
 * context.
 */
ECSACT_DYNAMIC_API_FN(void, ecsact_system_execution_context_action)
( //
	struct ecsact_system_execution_context* context,
	void*                                   out_action_data
);

/**
 * Add new component to the entity currently being processed by the system.
 *
 * Only available if has one of these capabilities:
 *  - `ECSACT_SYS_CAP_ADDS`
 */
ECSACT_DYNAMIC_API_FN(void, ecsact_system_execution_context_add)
( //
	struct ecsact_system_execution_context* context,
	ecsact_component_like_id                component_id,
	const void*                             component_data
);

/**
 * Remove existing component from the entity currently being processed by the
 * system.
 *
 * Only available if has one of these capabilities:
 *  - `ECSACT_SYS_CAP_REMOVES`
 */
ECSACT_DYNAMIC_API_FN(void, ecsact_system_execution_context_remove)
( //
	struct ecsact_system_execution_context* context,
	ecsact_component_like_id                component_id
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
( //
	struct ecsact_system_execution_context* context,
	ecsact_component_like_id                component_id,
	void*                                   out_component_data
);

/**
 * Only available if has one of these capabilities:
 *  - `ECSACT_SYS_CAP_WRITEONLY`
 *  - `ECSACT_SYS_CAP_READWRITE`
 *  - `ECSACT_SYS_CAP_OPTIONAL_WRITEONLY`
 *  - `ECSACT_SYS_CAP_OPTIONAL_READWRITE`
 */
ECSACT_DYNAMIC_API_FN(void, ecsact_system_execution_context_update)
( //
	struct ecsact_system_execution_context* context,
	ecsact_component_like_id                component_id,
	const void*                             component_data
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
( //
	struct ecsact_system_execution_context* context,
	ecsact_component_like_id                component_id
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
( //
	struct ecsact_system_execution_context* context,
	int                                     component_count,
	ecsact_component_id*                    component_ids,
	const void**                            components_data
);

/**
 * Get the parent system exeuction context.
 *
 * Only available if the currently executing system is a nested system.
 */
ECSACT_DYNAMIC_API_FN(
	const struct ecsact_system_execution_context*,
	ecsact_system_execution_context_parent
)
( //
	struct ecsact_system_execution_context* context
);

/**
 * Check if two execution contexts refer to the same entity. This is useful when
 * comparing against parent execution context to skip
 *
 * NOTE: This is will eventually be deprecated in favour of a language feature
 *       to skip matching parent system entities.
 */
ECSACT_DYNAMIC_API_FN(bool, ecsact_system_execution_context_same)
( //
	const struct ecsact_system_execution_context*,
	const struct ecsact_system_execution_context*
);

/**
 * Get execution context for a different entity
 *
 * Only available if has one of these capabilities:
 *  - `ECSACT_SYS_CAP_OPTIONAL_READONLY`
 *  - `ECSACT_SYS_CAP_OPTIONAL_WRITEONLY`
 *  - `ECSACT_SYS_CAP_OPTIONAL_READWRITE`
 */
ECSACT_DYNAMIC_API_FN(
	struct ecsact_system_execution_context*,
	ecsact_system_execution_context_other
)
( //
	struct ecsact_system_execution_context* context,
	ecsact_entity_id                        entity_id
);

/**
 * Get the entity for the execution context
 */
ECSACT_DYNAMIC_API_FN(ecsact_entity_id, ecsact_system_execution_context_entity)
( //
	const struct ecsact_system_execution_context* context
);

/**
 * Get the current system/action ID
 */
ECSACT_DYNAMIC_API_FN(ecsact_system_like_id, ecsact_system_execution_context_id)
( //
	struct ecsact_system_execution_context* context
);

ECSACT_DYNAMIC_API_FN(ecsact_package_id, ecsact_create_package)
( //
	bool        main_package,
	const char* package_name,
	int32_t     package_name_len
);

ECSACT_DYNAMIC_API_FN(void, ecsact_set_package_source_file_path)
( //
	ecsact_package_id package_id,
	const char*       source_file_path,
	int32_t           source_file_path_len
);

ECSACT_DYNAMIC_API_FN(void, ecsact_add_dependency)
( //
	ecsact_package_id target,
	ecsact_package_id dependency
);

ECSACT_DYNAMIC_API_FN(void, ecsact_remove_dependency)
( //
	ecsact_package_id target,
	ecsact_package_id dependency
);

ECSACT_DYNAMIC_API_FN(void, ecsact_destroy_package)
( //
	ecsact_package_id package_id
);

ECSACT_DYNAMIC_API_FN(ecsact_system_id, ecsact_create_system)
( //
	ecsact_package_id owner,
	const char*       system_name,
	int32_t           system_name_len
);

ECSACT_DYNAMIC_API_FN(void, ecsact_add_child_system)
( //
	ecsact_system_like_id parent,
	ecsact_system_id      child
);

ECSACT_DYNAMIC_API_FN(void, ecsact_remove_child_system)
( //
	ecsact_system_like_id parent,
	ecsact_system_id      child
);

/**
 * Systems execute in the order they are created in. If the order needs to be
 * adjusted this method can be used to move systems before or after other
 * systems.
 */
ECSACT_DYNAMIC_API_FN(bool, ecsact_reorder_system)
( //
	ecsact_system_like_id target_system_id,
	ecsact_system_like_id relative_system_id,
	bool                  target_before_relative
);

/**
 * Sets the system execution implementation function. If one is already set it
 * gets overwritten.
 *
 * NOTE: ONLY `ecsact_system_execution_context_*` functions are allowed to be
 *       called while a system is executing.
 */
ECSACT_DYNAMIC_API_FN(bool, ecsact_set_system_execution_impl)
( //
	ecsact_system_like_id        system_id,
	ecsact_system_execution_impl system_exec_impl
);

ECSACT_DYNAMIC_API_FN(ecsact_action_id, ecsact_create_action)
( //
	ecsact_package_id owner,
	const char*       action_name,
	int32_t           action_name_len
);

/**
 * Create new component
 * @returns unique component ID for newly created component
 */
ECSACT_DYNAMIC_API_FN(ecsact_component_id, ecsact_create_component)
( //
	ecsact_package_id owner,
	const char*       component_name,
	int32_t           component_name_len
);

/**
 * Create new transient
 * @returns unique transient ID for newly created transient
 */
ECSACT_DYNAMIC_API_FN(ecsact_transient_id, ecsact_create_transient)
( //
	ecsact_package_id owner,
	const char*       transient_name,
	int32_t           transient_name_len
);

/**
 * @param declaration_id - Component or action ID
 * @returns field index for declaration
 */
ECSACT_DYNAMIC_API_FN(ecsact_field_id, ecsact_add_field)
( //
	ecsact_composite_id composite_id,
	ecsact_field_type   field_type,
	const char*         field_name,
	int32_t             field_name_len
);

/**
 * @param declaration_id - Component or action ID
 */
ECSACT_DYNAMIC_API_FN(void, ecsact_remove_field)
( //
	ecsact_composite_id composite_id,
	ecsact_field_id     field_id
);

ECSACT_DYNAMIC_API_FN(void, ecsact_destroy_component)
( //
	ecsact_component_id component_id
);

ECSACT_DYNAMIC_API_FN(void, ecsact_destroy_transient)
( //
	ecsact_transient_id component_id
);

ECSACT_DYNAMIC_API_FN(ecsact_enum_id, ecsact_create_enum)
( //
	ecsact_package_id owner,
	const char*       enum_name,
	int32_t           enum_name_len
);

ECSACT_DYNAMIC_API_FN(void, ecsact_destroy_enum)
( //
	ecsact_enum_id
);

ECSACT_DYNAMIC_API_FN(ecsact_enum_value_id, ecsact_add_enum_value)
( //
	ecsact_enum_id enum_id,
	int32_t        value,
	const char*    value_name,
	int32_t        value_name_len
);

ECSACT_DYNAMIC_API_FN(void, ecsact_remove_enum_value)
( //
	ecsact_enum_id       enum_id,
	ecsact_enum_value_id value_id
);

ECSACT_DYNAMIC_API_FN(void, ecsact_set_system_capability)
( //
	ecsact_system_like_id,
	ecsact_component_like_id,
	ecsact_system_capability
);

ECSACT_DYNAMIC_API_FN(void, ecsact_unset_system_capability)
( //
	ecsact_system_like_id,
	ecsact_component_like_id
);

ECSACT_DYNAMIC_API_FN(void, ecsact_set_system_association_capability)
( //
	ecsact_system_like_id,
	ecsact_component_like_id,
	ecsact_field_id,
	ecsact_component_like_id,
	ecsact_system_capability
);

ECSACT_DYNAMIC_API_FN(void, ecsact_unset_system_association_capability)
( //
	ecsact_system_like_id,
	ecsact_component_like_id,
	ecsact_field_id,
	ecsact_component_like_id
);

ECSACT_DYNAMIC_API_FN(ecsact_system_generates_id, ecsact_add_system_generates)
( //
	ecsact_system_like_id system_id
);

ECSACT_DYNAMIC_API_FN(void, ecsact_remove_system_generates)
( //
	ecsact_system_like_id      system_id,
	ecsact_system_generates_id generates_id
);

ECSACT_DYNAMIC_API_FN(void, ecsact_system_generates_set_component)
( //
	ecsact_system_like_id      system_id,
	ecsact_system_generates_id generates_id,
	ecsact_component_id        component_id,
	ecsact_system_generate     generate_flag
);

ECSACT_DYNAMIC_API_FN(void, ecsact_system_generates_unset_component)
( //
	ecsact_system_like_id      system_id,
	ecsact_system_generates_id generates_id,
	ecsact_component_id        component_id
);

// # BEGIN FOR_EACH_ECSACT_DYNAMIC_API_FN
#ifdef ECSACT_MSVC_TRADITIONAL
#	define FOR_EACH_ECSACT_DYNAMIC_API_FN(fn, ...) \
		ECSACT_MSVC_TRADITIONAL_ERROR()
#else
#	define FOR_EACH_ECSACT_DYNAMIC_API_FN(fn, ...)                \
		fn(ecsact_system_execution_context_action, __VA_ARGS__);     \
		fn(ecsact_system_execution_context_add, __VA_ARGS__);        \
		fn(ecsact_system_execution_context_remove, __VA_ARGS__);     \
		fn(ecsact_system_execution_context_get, __VA_ARGS__);        \
		fn(ecsact_system_execution_context_update, __VA_ARGS__);     \
		fn(ecsact_system_execution_context_has, __VA_ARGS__);        \
		fn(ecsact_system_execution_context_generate, __VA_ARGS__);   \
		fn(ecsact_system_execution_context_parent, __VA_ARGS__);     \
		fn(ecsact_system_execution_context_same, __VA_ARGS__);       \
		fn(ecsact_system_execution_context_other, __VA_ARGS__);      \
		fn(ecsact_system_execution_context_entity, __VA_ARGS__);     \
		fn(ecsact_system_execution_context_id, __VA_ARGS__);         \
		fn(ecsact_create_package, __VA_ARGS__);                      \
		fn(ecsact_set_package_source_file_path, __VA_ARGS__);        \
		fn(ecsact_add_dependency, __VA_ARGS__);                      \
		fn(ecsact_remove_dependency, __VA_ARGS__);                   \
		fn(ecsact_destroy_package, __VA_ARGS__);                     \
		fn(ecsact_create_system, __VA_ARGS__);                       \
		fn(ecsact_add_child_system, __VA_ARGS__);                    \
		fn(ecsact_remove_child_system, __VA_ARGS__);                 \
		fn(ecsact_reorder_system, __VA_ARGS__);                      \
		fn(ecsact_set_system_execution_impl, __VA_ARGS__);           \
		fn(ecsact_create_action, __VA_ARGS__);                       \
		fn(ecsact_create_component, __VA_ARGS__);                    \
		fn(ecsact_create_transient, __VA_ARGS__);                    \
		fn(ecsact_add_field, __VA_ARGS__);                           \
		fn(ecsact_remove_field, __VA_ARGS__);                        \
		fn(ecsact_destroy_component, __VA_ARGS__);                   \
		fn(ecsact_destroy_transient, __VA_ARGS__);                   \
		fn(ecsact_create_enum, __VA_ARGS__);                         \
		fn(ecsact_destroy_enum, __VA_ARGS__);                        \
		fn(ecsact_add_enum_value, __VA_ARGS__);                      \
		fn(ecsact_remove_enum_value, __VA_ARGS__);                   \
		fn(ecsact_set_system_capability, __VA_ARGS__);               \
		fn(ecsact_unset_system_capability, __VA_ARGS__);             \
		fn(ecsact_set_system_association_capability, __VA_ARGS__);   \
		fn(ecsact_unset_system_association_capability, __VA_ARGS__); \
		fn(ecsact_add_system_generates, __VA_ARGS__);                \
		fn(ecsact_remove_system_generates, __VA_ARGS__);             \
		fn(ecsact_system_generates_set_component, __VA_ARGS__);      \
		fn(ecsact_system_generates_unset_component, __VA_ARGS__)
#endif

#endif // ECSACT_RUNTIME_DYNAMIC_H
