#ifndef ECSACT_RUNTIME_CORE_H
#define ECSACT_RUNTIME_CORE_H

#include <stdint.h>
#include <stdbool.h>
#include "common.h"
#include "ecsact/runtime/common.h"

#ifdef ECSACT_CORE_API_VISIBILITY
#	error "ECSACT_CORE_API_VISIBILITY define is deprecated"
#endif

#ifndef ECSACT_CORE_API_FN
#	if defined(ECSACT_CORE_API)
#		define ECSACT_CORE_API_FN(ret, name) ECSACT_CORE_API ret name
#	elif defined(ECSACT_CORE_API_LOAD_AT_RUNTIME)
#		if defined(ECSACT_CORE_API_EXPORT)
#			define ECSACT_CORE_API_FN(ret, name) \
				ECSACT_EXTERN ECSACT_EXPORT(#name) ret(*name)
#		else
#			define ECSACT_CORE_API_FN(ret, name) \
				ECSACT_EXTERN ECSACT_IMPORT("env", #name) ret(*name)
#		endif
#	elif defined(ECSACT_CORE_API_EXPORT)
#		define ECSACT_CORE_API_FN(ret, name) \
			ECSACT_EXTERN ECSACT_EXPORT(#name) ret name
#	else
#		define ECSACT_CORE_API_FN(ret, name) \
			ECSACT_EXTERN ECSACT_IMPORT("env", #name) ret name
#	endif
#endif // ECSACT_CORE_API_FN

/**
 * Create a new registry.
 * @param registry_name (Optional) Display name for the registry. Only used for
 * debugging.
 * @return The newly created registry ID.
 */
ECSACT_CORE_API_FN(ecsact_registry_id, ecsact_create_registry)
( //
	const char* registry_name
);

/**
 * Effectively calls `ecsact_destroy_entity` on each entity in the registry. The
 * registry ID is invalid after this call.
 */
ECSACT_CORE_API_FN(void, ecsact_destroy_registry)
( //
	ecsact_registry_id registry
);

/**
 * Creates a new registry from an existing one with all its entities and
 * components intact.
 *
 * If `ecsact_hash_registry` is defined then the cloned registry hash must
 * match the original registry.
 */
ECSACT_CORE_API_FN(ecsact_registry_id, ecsact_clone_registry)
( //
	ecsact_registry_id registry,
	const char*        registry_name
);

/**
 * Creates a hash of current state of the registry. The algorithm is
 * implementation defined, but must represent both user state and internal
 * state.
 */
ECSACT_CORE_API_FN(uint64_t, ecsact_hash_registry)
( //
	ecsact_registry_id registry
);

/**
 * Destroy all entities
 */
ECSACT_CORE_API_FN(void, ecsact_clear_registry)
( //
	ecsact_registry_id registry
);

/**
 * Create an entity and return the ID
 */
ECSACT_CORE_API_FN(ecsact_entity_id, ecsact_create_entity)
( //
	ecsact_registry_id registry
);

/**
 * Ensure an entity with the provided ID exists on the registry. If the entity
 * does not exist it will be created.
 *
 * NOTE: Avoid this method if possible.
 */
ECSACT_CORE_API_FN(void, ecsact_ensure_entity)
( //
	ecsact_registry_id registry,
	ecsact_entity_id   entity
);

/**
 * Check if entity exists.
 *
 * NOTE: Avoid this method if possible.
 */
ECSACT_CORE_API_FN(bool, ecsact_entity_exists)
( //
	ecsact_registry_id,
	ecsact_entity_id
);

/**
 * Destroys an entity. Effectively removes each component on the specified
 * entity. The entity ID is invalid after this call, but may be re-used later.
 */
ECSACT_CORE_API_FN(void, ecsact_destroy_entity)
( //
	ecsact_registry_id registry_id,
	ecsact_entity_id   entity_id
);

/**
 * Count number of entities in registry
 */
ECSACT_CORE_API_FN(int, ecsact_count_entities)
( //
	ecsact_registry_id registry
);

/**
 * Get list of entities in registry
 */
ECSACT_CORE_API_FN(void, ecsact_get_entities)
( //
	ecsact_registry_id registry,
	int                max_entities_count,
	ecsact_entity_id*  out_entities,
	int*               out_entities_count
);

/**
 * Adds a component to the specified entity.
 *
 * NOTE: This method should be avoided if possible. Adding a component in a
 *       system or system execution options is preferred.
 *       SEE: `ecsact_execute_systems`
 */
ECSACT_CORE_API_FN(ecsact_add_error, ecsact_add_component)
( //
	ecsact_registry_id  registry_id,
	ecsact_entity_id    entity_id,
	ecsact_component_id component_id,
	const void*         component_data
);

/**
 * Checks if a given entity has component with id @p component_id
 *
 * @param indexed_field_values if the component has indexed fields then those
 * fields must be supplied as a sequential array in declaration order,
 * otherwise may be NULL.
 */
ECSACT_CORE_API_FN(bool, ecsact_has_component)
( //
	ecsact_registry_id  registry_id,
	ecsact_entity_id    entity_id,
	ecsact_component_id component_id,
	const void*         indexed_field_values
);

/**
 * @param indexed_field_values if the component has indexed fields then those
 * fields must be supplied as a sequential array in declaration order,
 * otherwise may be NULL.
 *
 * @returns non-owning pointer of the component data
 *
 * NOTE: This method should be avoided if possible.
 */
ECSACT_CORE_API_FN(const void*, ecsact_get_component)
( //
	ecsact_registry_id  registry_id,
	ecsact_entity_id    entity_id,
	ecsact_component_id component_id,
	const void*         indexed_field_values
);

/**
 * @returns the number of components added to an entity
 */
ECSACT_CORE_API_FN(int, ecsact_count_components)
( //
	ecsact_registry_id registry_id,
	ecsact_entity_id   entity_id
);

ECSACT_CORE_API_FN(void, ecsact_get_components)
( //
	ecsact_registry_id   registry_id,
	ecsact_entity_id     entity_id,
	int                  max_components_count,
	ecsact_component_id* out_component_ids,
	const void**         out_components_data,
	int*                 out_components_count
);

typedef void (*ecsact_each_component_callback)( //
	ecsact_component_id component_id,
	const void*         component_data,
	void*               user_data
);

/**
 * Invoke `callback` for every component an entity has
 */
ECSACT_CORE_API_FN(void, ecsact_each_component)
( //
	ecsact_registry_id             registry_id,
	ecsact_entity_id               entity_id,
	ecsact_each_component_callback callback,
	void*                          callback_user_data
);

/**
 * Update a component for the specified entity.
 *
 * @param indexed_field_values if the component has indexed fields then those
 * fields must be supplied as a sequential array in declaration order,
 * otherwise may be NULL.
 *
 * NOTE: This method should be avoided if possible. Updating a component in a
 *       system or system execution options is preferred.
 *       SEE: `ecsact_execute_systems`
 */
ECSACT_CORE_API_FN(ecsact_update_error, ecsact_update_component)
( //
	ecsact_registry_id  registry_id,
	ecsact_entity_id    entity_id,
	ecsact_component_id component_id,
	const void*         component_data,
	const void*         indexed_field_values
);

/**
 * Removes a component from the specified entity.
 *
 * @param indexed_field_values if the component has indexed fields then those
 * fields must be supplied as a sequential array in declaration order,
 * otherwise may be NULL.
 *
 * NOTE: This method should be avoided if possible. Removing a component in a
 *       system or system execution options is preferred.
 *       SEE: `ecsact_execute_systems`
 */
ECSACT_CORE_API_FN(void, ecsact_remove_component)
( //
	ecsact_registry_id  registry_id,
	ecsact_entity_id    entity_id,
	ecsact_component_id component_id,
	const void*         indexed_field_values
);

/**
 * Execute system implementations for all registered systems and pushed actions
 * against all registered components. System implementations may run in parallel
 * on multiple threads.
 * @param execution_count how many times the systems list should execute
 * @param execution_options_list (optional) Sequential list of execution
 * options. If set (not NULL), list length is determined by `execution_count`.
 * @param events_collector (optional) Pointer to events collector. If set,
 *        events will be recorded and the callbacks on the collector will be
 *        invoked. Invocations occur on the calling thread.
 */
ECSACT_CORE_API_FN(ecsact_execute_systems_error, ecsact_execute_systems)
( //
	ecsact_registry_id                       registry_id,
	int                                      execution_count,
	const ecsact_execution_options*          execution_options_list,
	const ecsact_execution_events_collector* events_collector
);

/**
 * Gets the current execution status of an entity.
 *
 * @see ecsact_ees
 */
ECSACT_CORE_API_FN(ecsact_ees, ecsact_get_entity_execution_status)
( //
	ecsact_registry_id    registry_id,
	ecsact_entity_id      entity_id,
	ecsact_system_like_id system_like_id
);

/**
 * Sends Ecsact stream data to the specified registry. Stream data will be
 * applied on the next ecsact_execute_systems call. The last set of stream data
 * is always used.
 *
 * @param indexed_field_values if the component has indexed fields then those
 * fields must be supplied as a sequential array in declaration order,
 * otherwise may be NULL.
 */
ECSACT_CORE_API_FN(ecsact_stream_error, ecsact_stream)
( //
	ecsact_registry_id  registry_id,
	ecsact_entity_id    entity,
	ecsact_component_id component_id,
	const void*         component_data,
	const void*         indexed_field_values
);

// # BEGIN FOR_EACH_ECSACT_CORE_API_FN
#ifdef ECSACT_MSVC_TRADITIONAL
#	define FOR_EACH_ECSACT_CORE_API_FN(fn, ...) ECSACT_MSVC_TRADITIONAL_ERROR()
#else
#	define FOR_EACH_ECSACT_CORE_API_FN(fn, ...)           \
		fn(ecsact_create_registry, __VA_ARGS__);             \
		fn(ecsact_destroy_registry, __VA_ARGS__);            \
		fn(ecsact_clone_registry, __VA_ARGS__);              \
		fn(ecsact_hash_registry, __VA_ARGS__);               \
		fn(ecsact_clear_registry, __VA_ARGS__);              \
		fn(ecsact_create_entity, __VA_ARGS__);               \
		fn(ecsact_ensure_entity, __VA_ARGS__);               \
		fn(ecsact_entity_exists, __VA_ARGS__);               \
		fn(ecsact_destroy_entity, __VA_ARGS__);              \
		fn(ecsact_count_entities, __VA_ARGS__);              \
		fn(ecsact_get_entities, __VA_ARGS__);                \
		fn(ecsact_add_component, __VA_ARGS__);               \
		fn(ecsact_has_component, __VA_ARGS__);               \
		fn(ecsact_get_component, __VA_ARGS__);               \
		fn(ecsact_count_components, __VA_ARGS__);            \
		fn(ecsact_get_components, __VA_ARGS__);              \
		fn(ecsact_each_component, __VA_ARGS__);              \
		fn(ecsact_update_component, __VA_ARGS__);            \
		fn(ecsact_remove_component, __VA_ARGS__);            \
		fn(ecsact_execute_systems, __VA_ARGS__);             \
		fn(ecsact_get_entity_execution_status, __VA_ARGS__); \
		fn(ecsact_stream, __VA_ARGS__)
#endif

#endif // ECSACT_RUNTIME_CORE_H
