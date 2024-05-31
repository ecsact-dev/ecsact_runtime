#ifndef ECSACT_RUNTIME_META_H
#define ECSACT_RUNTIME_META_H

#include <stdlib.h>

#include "ecsact/runtime/common.h"
#include "ecsact/runtime/definitions.h"

#ifdef ECSACT_META_API_VISIBILITY
#	error "ECSACT_META_API_VISIBILITY define is deprecated"
#endif

#ifndef ECSACT_META_API_FN
#	if defined(ECSACT_META_API)
#		define ECSACT_META_API_FN(ret, name) ECSACT_META_API ret name
#	elif defined(ECSACT_META_API_LOAD_AT_RUNTIME)
#		define ECSACT_META_API_FN(ret, name) ECSACT_EXTERN ret(*name)
#	elif defined(ECSACT_META_API_EXPORT)
#		define ECSACT_META_API_FN(ret, name) \
			ECSACT_EXTERN ECSACT_EXPORT(#name) ret name
#	else
#		define ECSACT_META_API_FN(ret, name) \
			ECSACT_EXTERN ECSACT_IMPORT("env", #name) ret name
#	endif
#endif // ECSACT_META_API_FN

/**
 * @returns number of packages
 */
ECSACT_META_API_FN(int32_t, ecsact_meta_count_packages)();

ECSACT_META_API_FN(void, ecsact_meta_get_package_ids)
( //
	int32_t            max_package_count,
	ecsact_package_id* out_package_ids,
	int32_t*           out_package_count
);

ECSACT_META_API_FN(const char*, ecsact_meta_package_name)
( //
	ecsact_package_id package_id
);

/**
 * @returns the main package ID. -1 if there is no main package
 */
ECSACT_META_API_FN(ecsact_package_id, ecsact_meta_main_package)();

/**
 * @param package_id the ID given to the package when created
 * @returns the ecsact file path for the package
 */
ECSACT_META_API_FN(const char*, ecsact_meta_package_file_path)
( //
	ecsact_package_id package_id
);

ECSACT_META_API_FN(int32_t, ecsact_meta_count_dependencies)
( //
	ecsact_package_id package_id
);

ECSACT_META_API_FN(void, ecsact_meta_get_dependencies)
( //
	ecsact_package_id  package_id,
	int32_t            max_dependency_count,
	ecsact_package_id* out_package_ids,
	int32_t*           out_dependencies_count
);

/**
 * Get number of components in package
 */
ECSACT_META_API_FN(int32_t, ecsact_meta_count_components)
( //
	ecsact_package_id package_id
);

ECSACT_META_API_FN(void, ecsact_meta_get_component_ids)
( //
	ecsact_package_id    package_id,
	int32_t              max_component_count,
	ecsact_component_id* out_component_ids,
	int32_t*             out_component_count
);

/**
 * Get number of transients in package
 */
ECSACT_META_API_FN(int32_t, ecsact_meta_count_transients)
( //
	ecsact_package_id package_id
);

ECSACT_META_API_FN(void, ecsact_meta_get_transient_ids)
( //
	ecsact_package_id    package_id,
	int32_t              max_transient_count,
	ecsact_transient_id* out_transient_ids,
	int32_t*             out_transient_count
);

/**
 * @param declaration_id component or action ID
 * @returns number of fields declaration has
 */
ECSACT_META_API_FN(int32_t, ecsact_meta_count_fields)
( //
	ecsact_composite_id composite_id
);

ECSACT_META_API_FN(void, ecsact_meta_get_field_ids)
( //
	ecsact_composite_id composite_id,
	int32_t             max_field_count,
	ecsact_field_id*    out_field_ids,
	int32_t*            out_field_ids_count
);

ECSACT_META_API_FN(const char*, ecsact_meta_field_name)
( //
	ecsact_composite_id composite_id,
	ecsact_field_id     field_id
);

ECSACT_META_API_FN(ecsact_field_type, ecsact_meta_field_type)
( //
	ecsact_composite_id composite_id,
	ecsact_field_id     field_id
);

ECSACT_META_API_FN(int32_t, ecsact_meta_field_offset)
( //
	ecsact_composite_id composite_id,
	ecsact_field_id     field_id
);

ECSACT_META_API_FN(int32_t, ecsact_meta_count_systems)
( //
	ecsact_package_id package_id
);

ECSACT_META_API_FN(void, ecsact_meta_get_system_ids)
( //
	ecsact_package_id package_id,
	int32_t           max_system_count,
	ecsact_system_id* out_system_ids,
	int32_t*          out_system_count
);

ECSACT_META_API_FN(int32_t, ecsact_meta_count_actions)
( //
	ecsact_package_id package_id
);

ECSACT_META_API_FN(void, ecsact_meta_get_action_ids)
( //
	ecsact_package_id package_id,
	int32_t           max_action_count,
	ecsact_action_id* out_action_ids,
	int32_t*          out_action_count
);

ECSACT_META_API_FN(int32_t, ecsact_meta_count_enums)
( //
	ecsact_package_id package_id
);

ECSACT_META_API_FN(void, ecsact_meta_get_enum_ids)
( //
	ecsact_package_id package_id,
	int32_t           max_enum_count,
	ecsact_enum_id*   out_enum_ids,
	int32_t*          out_enum_count
);

/**
 * The storage type of the enum can be an unsigned or signed integer based on
 * the enum values. The storage type will be the smallest integer that can hold
 * all possible valid enum values.
 * @returns storage type that can be used to represent the enum
 */
ECSACT_META_API_FN(ecsact_builtin_type, ecsact_meta_enum_storage_type)
( //
	ecsact_enum_id enum_id
);

ECSACT_META_API_FN(int32_t, ecsact_meta_count_enum_values)
( //
	ecsact_enum_id enum_id
);

ECSACT_META_API_FN(void, ecsact_meta_get_enum_value_ids)
( //
	ecsact_enum_id        enum_id,
	int32_t               max_enum_value_ids,
	ecsact_enum_value_id* out_enum_value_ids,
	int32_t*              out_enum_values_count
);

ECSACT_META_API_FN(const char*, ecsact_meta_enum_name)
( //
	ecsact_enum_id enum_id
);

ECSACT_META_API_FN(const char*, ecsact_meta_enum_value_name)
( //
	ecsact_enum_id       enum_id,
	ecsact_enum_value_id value_id
);

ECSACT_META_API_FN(int32_t, ecsact_meta_enum_value)
( //
	ecsact_enum_id       enum_id,
	ecsact_enum_value_id value_id
);

/**
 * Get a registry name. May or may not return originally set name when creating
 * the registry. Typically not returned in release/optimized build.
 */
ECSACT_META_API_FN(const char*, ecsact_meta_registry_name)
( //
	ecsact_registry_id
);

/**
 * Get the component name. May or may not return originally set name when
 * creating a component. Typically not returned in release/optimized build.
 */
ECSACT_META_API_FN(const char*, ecsact_meta_component_name)
( //
	ecsact_component_id
);

/**
 * Get the transient name. May or may not return originally set name when
 * creating a transient. Typically not returned in release/optimized build.
 */
ECSACT_META_API_FN(const char*, ecsact_meta_transient_name)
( //
	ecsact_transient_id
);

/**
 * Get the action name. May or may not return originally set name when
 * creating a action. Typically not returned in release/optimized build.
 */
ECSACT_META_API_FN(const char*, ecsact_meta_action_name)
( //
	ecsact_action_id
);

/**
 * Get the system name. May or may not return originally set name when
 * creating a system. Typically not returned in release/optimized build.
 */
ECSACT_META_API_FN(const char*, ecsact_meta_system_name)
( //
	ecsact_system_id
);

/**
 * Get the count of capabilities for a particular system/action.
 */
ECSACT_META_API_FN(int32_t, ecsact_meta_system_capabilities_count)
( //
	ecsact_system_like_id system_id
);

/**
 * Get all the system/action capabilities
 * @param out_capability_component_ids - Output for component ids. Should be
 * allocated to the size returned from `ecsact_meta_system_capabilities_count`
 * @param out_capabilities - Output for capabilities. Should be allocated to the
 * size returned from `ecsact_meta_system_capabilities_count`
 */
ECSACT_META_API_FN(void, ecsact_meta_system_capabilities)
( //
	ecsact_system_like_id     system_id,
	int32_t                   max_capabilities_count,
	ecsact_component_like_id* out_capability_component_ids,
	ecsact_system_capability* out_capabilities,
	int32_t*                  out_capabilities_count
);

ECSACT_META_API_FN(int32_t, ecsact_meta_system_assoc_count)
( //
	ecsact_system_like_id system_id
);

ECSACT_META_API_FN(void, ecsact_meta_system_assoc_ids)
( //
	ecsact_system_like_id   system_id,
	int32_t                 max_assoc_count,
	ecsact_system_assoc_id* out_assoc_ids,
	int32_t*                out_assoc_count
);

ECSACT_META_API_FN(
	ecsact_component_like_id,
	ecsact_meta_system_assoc_component_id
)
( //
	ecsact_system_like_id  system_id,
	ecsact_system_assoc_id assoc_id
);

ECSACT_META_API_FN(int32_t, ecsact_meta_system_assoc_fields_count)
( //
	ecsact_system_like_id  system_id,
	ecsact_system_assoc_id assoc_id
);

ECSACT_META_API_FN(void, ecsact_meta_system_assoc_fields)
( //
	ecsact_system_like_id  system_id,
	ecsact_system_assoc_id assoc_id,
	int32_t                max_fields_count,
	ecsact_field_id*       out_fields,
	int32_t*               out_fields_count
);

ECSACT_META_API_FN(int32_t, ecsact_meta_system_assoc_capabilities_count)
( //
	ecsact_system_like_id  system_id,
	ecsact_system_assoc_id assoc_id
);

ECSACT_META_API_FN(void, ecsact_meta_system_assoc_capabilities)
( //
	ecsact_system_like_id     system_id,
	ecsact_system_assoc_id    assoc_id,
	int32_t                   max_capabilities_count,
	ecsact_component_like_id* out_capability_component_ids,
	ecsact_system_capability* out_capabilities,
	int32_t*                  out_capabilities_count
);

/**
 * @deprecated use ecsact_meta_system_assoc_* fns instead
 */
ECSACT_META_API_FN(int32_t, ecsact_meta_system_association_fields_count)
( //
	ecsact_system_like_id    system_id,
	ecsact_component_like_id component_id
);

/**
 * @deprecated use ecsact_meta_system_assoc_* fns instead
 */
ECSACT_META_API_FN(void, ecsact_meta_system_association_fields)
( //
	ecsact_system_like_id    system_id,
	ecsact_component_like_id component_id,
	int32_t                  max_fields_count,
	ecsact_field_id*         out_fields,
	int32_t*                 out_fields_count
);

/**
 * @deprecated use ecsact_meta_system_assoc_* fns instead
 */
ECSACT_META_API_FN(int32_t, ecsact_meta_system_association_capabilities_count)
( //
	ecsact_system_like_id    system_id,
	ecsact_component_like_id component_id,
	ecsact_field_id          field_id
);

/**
 * @deprecated use ecsact_meta_system_assoc_* fns instead
 */
ECSACT_META_API_FN(void, ecsact_meta_system_association_capabilities)
( //
	ecsact_system_like_id     system_id,
	ecsact_component_like_id  component_id,
	ecsact_field_id           field_id,
	int32_t                   max_capabilities_count,
	ecsact_component_like_id* out_capability_component_ids,
	ecsact_system_capability* out_capabilities,
	int32_t*                  out_capabilities_count
);

ECSACT_META_API_FN(int32_t, ecsact_meta_count_system_generates_ids)
( //
	ecsact_system_like_id system_id
);

ECSACT_META_API_FN(void, ecsact_meta_system_generates_ids)
( //
	ecsact_system_like_id       system_id,
	int32_t                     max_generates_ids_count,
	ecsact_system_generates_id* out_generates_ids,
	int32_t*                    out_generates_ids_count
);

ECSACT_META_API_FN(int32_t, ecsact_meta_count_system_generates_components)
( //
	ecsact_system_like_id      system_id,
	ecsact_system_generates_id generates_id
);

ECSACT_META_API_FN(void, ecsact_meta_system_generates_components)
( //
	ecsact_system_like_id      system_id,
	ecsact_system_generates_id generates_id,
	int32_t                    max_components_count,
	ecsact_component_id*       component_ids,
	ecsact_system_generate*    component_generate_flags,
	int32_t*                   out_components_count
);

/**
 * @returns a declarations full name including package and any parent
 *          declarations.
 */
ECSACT_META_API_FN(const char*, ecsact_meta_decl_full_name)
( //
	ecsact_decl_id id
);

ECSACT_META_API_FN(int32_t, ecsact_meta_count_child_systems)
( //
	ecsact_system_like_id system_id
);

/**
 * Get a list of system IDs in declaration order (which also means execution
 * order.)
 */
ECSACT_META_API_FN(void, ecsact_meta_get_child_system_ids)
( //
	ecsact_system_like_id system_id,
	int32_t               max_child_system_ids_count,
	ecsact_system_id*     out_child_system_ids,
	int32_t*              out_child_system_count
);

/**
 * @returns the parent system-like ID. Returns -1 if child has no parent
 */
ECSACT_META_API_FN(ecsact_system_like_id, ecsact_meta_get_parent_system_id)
( //
	ecsact_system_id child_system_id
);

/**
 * Count the number of top level systems and actions.
 */
ECSACT_META_API_FN(int32_t, ecsact_meta_count_top_level_systems)
( //
	ecsact_package_id package_id
);

/**
 * Get a list of system-like ids (system and actions) that are at the top level
 * in declaration order (which also means execution order.)
 */
ECSACT_META_API_FN(void, ecsact_meta_get_top_level_systems)
( //
	ecsact_package_id      package_id,
	int32_t                max_systems_count,
	ecsact_system_like_id* out_systems,
	int32_t*               out_systems_count
);

/**
 * Get the lazy iteration rate of a system. Returns `0` if system is not lazy.
 */
ECSACT_META_API_FN(int32_t, ecsact_meta_get_lazy_iteration_rate)
( //
	ecsact_system_id system_id
);

/**
 * Check if a system/action can run on multiple entities in parallel. This is
 * only a _hint_. The runtime implementation may choose to not run in parallel.
 */
ECSACT_META_API_FN(
	ecsact_parallel_execution,
	ecsact_meta_get_system_parallel_execution
)
( //
	ecsact_system_like_id system_like_id
);

/**
 * Count the number of notify settings a system has.
 */
ECSACT_META_API_FN(int32_t, ecsact_meta_system_notify_settings_count)
( //
	ecsact_system_like_id system_like_id
);

/**
 *
 */
ECSACT_META_API_FN(void, ecsact_meta_system_notify_settings)
( //
	ecsact_system_like_id         system_like_id,
	int32_t                       max_notifies_count,
	ecsact_component_like_id*     out_notify_component_ids,
	ecsact_system_notify_setting* out_notify_settings,
	int32_t*                      out_notifies_count
);

// # BEGIN FOR_EACH_ECSACT_META_API_FN
#ifdef ECSACT_MSVC_TRADITIONAL
#	define FOR_EACH_ECSACT_META_API_FN(fn, ...) ECSACT_MSVC_TRADITIONAL_ERROR()
#else
#	define FOR_EACH_ECSACT_META_API_FN(fn, ...)                          \
		fn(ecsact_meta_count_packages, __VA_ARGS__);                        \
		fn(ecsact_meta_get_package_ids, __VA_ARGS__);                       \
		fn(ecsact_meta_package_name, __VA_ARGS__);                          \
		fn(ecsact_meta_main_package, __VA_ARGS__);                          \
		fn(ecsact_meta_package_file_path, __VA_ARGS__);                     \
		fn(ecsact_meta_count_dependencies, __VA_ARGS__);                    \
		fn(ecsact_meta_get_dependencies, __VA_ARGS__);                      \
		fn(ecsact_meta_count_components, __VA_ARGS__);                      \
		fn(ecsact_meta_get_component_ids, __VA_ARGS__);                     \
		fn(ecsact_meta_count_transients, __VA_ARGS__);                      \
		fn(ecsact_meta_get_transient_ids, __VA_ARGS__);                     \
		fn(ecsact_meta_count_fields, __VA_ARGS__);                          \
		fn(ecsact_meta_get_field_ids, __VA_ARGS__);                         \
		fn(ecsact_meta_field_name, __VA_ARGS__);                            \
		fn(ecsact_meta_field_type, __VA_ARGS__);                            \
		fn(ecsact_meta_field_offset, __VA_ARGS__);                          \
		fn(ecsact_meta_count_systems, __VA_ARGS__);                         \
		fn(ecsact_meta_get_system_ids, __VA_ARGS__);                        \
		fn(ecsact_meta_count_actions, __VA_ARGS__);                         \
		fn(ecsact_meta_get_action_ids, __VA_ARGS__);                        \
		fn(ecsact_meta_count_enums, __VA_ARGS__);                           \
		fn(ecsact_meta_get_enum_ids, __VA_ARGS__);                          \
		fn(ecsact_meta_enum_storage_type, __VA_ARGS__);                     \
		fn(ecsact_meta_count_enum_values, __VA_ARGS__);                     \
		fn(ecsact_meta_get_enum_value_ids, __VA_ARGS__);                    \
		fn(ecsact_meta_enum_name, __VA_ARGS__);                             \
		fn(ecsact_meta_enum_value_name, __VA_ARGS__);                       \
		fn(ecsact_meta_enum_value, __VA_ARGS__);                            \
		fn(ecsact_meta_registry_name, __VA_ARGS__);                         \
		fn(ecsact_meta_component_name, __VA_ARGS__);                        \
		fn(ecsact_meta_transient_name, __VA_ARGS__);                        \
		fn(ecsact_meta_action_name, __VA_ARGS__);                           \
		fn(ecsact_meta_system_name, __VA_ARGS__);                           \
		fn(ecsact_meta_system_capabilities_count, __VA_ARGS__);             \
		fn(ecsact_meta_system_capabilities, __VA_ARGS__);                   \
		fn(ecsact_meta_system_assoc_count, __VA_ARGS__);                    \
		fn(ecsact_meta_system_assoc_ids, __VA_ARGS__);                      \
		fn(ecsact_meta_system_assoc_component_id, __VA_ARGS__);             \
		fn(ecsact_meta_system_assoc_fields_count, __VA_ARGS__);             \
		fn(ecsact_meta_system_assoc_fields, __VA_ARGS__);                   \
		fn(ecsact_meta_system_assoc_capabilities_count, __VA_ARGS__);       \
		fn(ecsact_meta_system_assoc_capabilities, __VA_ARGS__);             \
		fn(ecsact_meta_system_association_fields_count, __VA_ARGS__);       \
		fn(ecsact_meta_system_association_fields, __VA_ARGS__);             \
		fn(ecsact_meta_system_association_capabilities_count, __VA_ARGS__); \
		fn(ecsact_meta_system_association_capabilities, __VA_ARGS__);       \
		fn(ecsact_meta_count_system_generates_ids, __VA_ARGS__);            \
		fn(ecsact_meta_system_generates_ids, __VA_ARGS__);                  \
		fn(ecsact_meta_count_system_generates_components, __VA_ARGS__);     \
		fn(ecsact_meta_system_generates_components, __VA_ARGS__);           \
		fn(ecsact_meta_decl_full_name, __VA_ARGS__);                        \
		fn(ecsact_meta_count_child_systems, __VA_ARGS__);                   \
		fn(ecsact_meta_get_child_system_ids, __VA_ARGS__);                  \
		fn(ecsact_meta_get_parent_system_id, __VA_ARGS__);                  \
		fn(ecsact_meta_count_top_level_systems, __VA_ARGS__);               \
		fn(ecsact_meta_get_top_level_systems, __VA_ARGS__);                 \
		fn(ecsact_meta_get_lazy_iteration_rate, __VA_ARGS__);               \
		fn(ecsact_meta_get_system_parallel_execution, __VA_ARGS__);         \
		fn(ecsact_meta_system_notify_settings_count, __VA_ARGS__);          \
		fn(ecsact_meta_system_notify_settings, __VA_ARGS__)
#endif

#endif // ECSACT_RUNTIME_META_H
