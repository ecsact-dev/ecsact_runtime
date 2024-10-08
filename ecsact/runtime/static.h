#ifndef ECSACT_RUNTIME_STATIC_H
#define ECSACT_RUNTIME_STATIC_H

#include <stdlib.h>

#include "ecsact/runtime/common.h"

#ifdef ECSACT_STATIC_API_VISIBILITY
#	error "ECSACT_STATIC_API_VISIBILITY define is deprecated"
#endif

#ifndef ECSACT_STATIC_API_FN
#	if defined(ECSACT_STATIC_API)
#		define ECSACT_STATIC_API_FN(ret, name) ECSACT_STATIC_API ret name
#	elif defined(ECSACT_STATIC_API_LOAD_AT_RUNTIME)
#		if defined(ECSACT_STATIC_API_EXPORT)
#			define ECSACT_STATIC_API_FN(ret, name) \
				ECSACT_EXTERN ECSACT_EXPORT(#name) ret(*name)
#		else
#			define ECSACT_STATIC_API_FN(ret, name) \
				ECSACT_EXTERN ECSACT_IMPORT("env", #name) ret(*name)
#		endif
#	elif defined(ECSACT_STATIC_API_EXPORT)
#		define ECSACT_STATIC_API_FN(ret, name) \
			ECSACT_EXTERN ECSACT_EXPORT(#name) ret name
#	else
#		define ECSACT_STATIC_API_FN(ret, name) \
			ECSACT_EXTERN ECSACT_IMPORT("env", #name) ret name
#	endif
#endif // ECSACT_STATIC_API_FN

typedef struct {
	ecsact_component_id           component_id;
	const char*                   component_name;
	size_t                        component_size;
	ecsact_component_compare_fn_t component_compare_fn;
	bool                          transient;
} ecsact_static_component_info;

/**
 * Get list of static fixed components info that were available at compile time.
 */
ECSACT_STATIC_API_FN(void, ecsact_static_components)
( //
	const ecsact_static_component_info** out_components,
	size_t*                              out_components_count
);

typedef struct {
	ecsact_variant_id    variant_id;
	const char*          variant_name;
	size_t               alternatives_count;
	ecsact_component_id* alternatives;
} ecsact_static_variant_info;

/**
 * Get list of static fixed variants info that were available at compile time.
 */
ECSACT_STATIC_API_FN(void, ecsact_static_variants)
( //
	const ecsact_static_variant_info** out_variants,
	size_t*                            out_variants_count
);

typedef struct {
	ecsact_system_id             system_id;
	int32_t                      order;
	const char*                  system_name;
	ecsact_system_id             parent_system_id;
	size_t                       child_systems_count;
	ecsact_system_id*            child_system_ids;
	size_t                       capabilities_count;
	ecsact_component_id*         capability_components;
	ecsact_system_capability*    capabilities;
	ecsact_system_execution_impl execution_impl;
} ecsact_static_system_info;

/**
 * Get list of static fixed systems info that were available at compile time.
 */
ECSACT_STATIC_API_FN(void, ecsact_static_systems)
( //
	const ecsact_static_system_info** out_systems,
	size_t*                           out_systems_count
);

typedef struct {
	ecsact_system_id             action_id;
	int32_t                      order;
	const char*                  action_name;
	size_t                       action_size;
	ecsact_action_compare_fn_t   action_compare_fn;
	size_t                       child_systems_count;
	ecsact_system_id*            child_system_ids;
	size_t                       capabilities_count;
	ecsact_component_id*         capability_components;
	ecsact_system_capability*    capabilities;
	ecsact_system_execution_impl execution_impl;
} ecsact_static_action_info;

/**
 * Get list of static fixed actions info that were available at compile time.
 */
ECSACT_STATIC_API_FN(void, ecsact_static_actions)
( //
	const ecsact_static_action_info** out_actions,
	size_t*                           out_actions_count
);

typedef void (*ecsact_static_reload_callback)(void* user_data);
ECSACT_STATIC_API_FN(void, ecsact_static_on_reload)
( //
	ecsact_static_reload_callback callback,
	void*                         callback_user_data
);
ECSACT_STATIC_API_FN(void, ecsact_static_off_reload)
( //
	ecsact_static_reload_callback callback
);

// # BEGIN FOR_EACH_ECSACT_STATIC_API_FN
#ifdef ECSACT_MSVC_TRADITIONAL
#	define FOR_EACH_ECSACT_STATIC_API_FN(fn, ...) ECSACT_MSVC_TRADITIONAL_ERROR()
#else
#	define FOR_EACH_ECSACT_STATIC_API_FN(fn, ...) \
		fn(ecsact_static_components, __VA_ARGS__);   \
		fn(ecsact_static_variants, __VA_ARGS__);     \
		fn(ecsact_static_systems, __VA_ARGS__);      \
		fn(ecsact_static_actions, __VA_ARGS__);      \
		fn(ecsact_static_on_reload, __VA_ARGS__);    \
		fn(ecsact_static_off_reload, __VA_ARGS__)
#endif

#endif // ECSACT_RUNTIME_STATIC_H
