#ifndef ECSACT_RUNTIME_COMMON_H
#define ECSACT_RUNTIME_COMMON_H

#include <stdint.h>

#ifdef __cplusplus
#	define ECSACT_TYPED_ID(name) enum class name : int32_t
#else
#	define ECSACT_TYPED_ID(name) typedef int32_t name
#endif

ECSACT_TYPED_ID(ecsact_system_id);
ECSACT_TYPED_ID(ecsact_component_id);
ECSACT_TYPED_ID(ecsact_variant_id);
ECSACT_TYPED_ID(ecsact_registry_id);
ECSACT_TYPED_ID(ecsact_entity_id);

#undef ECSACT_TYPED_ID

/**
 * Context for system execution. This contains (or points to) state required for
 * executing a systems implementation. The structure for this type is runtime
 * implementation defined.
 */
struct ecsact_system_execution_context;

typedef void(*ecsact_system_execution_impl)
	( ecsact_system_execution_context*
	);

static const ecsact_system_id
ecsact_invalid_system_id = (ecsact_system_id)-1;

static const ecsact_registry_id
ecsact_invalid_registry_id = (ecsact_registry_id)-1;

static const ecsact_component_id
ecsact_invalid_component_id = (ecsact_component_id)-1;

static const ecsact_entity_id
ecsact_invalid_entity_id = (ecsact_entity_id)-1;

typedef enum {
	/**
	 * Add component was successful
	 */
	ECSACT_ADD_OK = 0,

	/**
	 * An invalid or non-existant entity ID was found in one or more of the
	 * component fields.
	 */
	ECSACT_ADD_ERR_ENTITY_INVALID = 1,

	/**
	 * One or more of the component entity fields constraints were not satisifed.
	 */
	ECSACT_ADD_ERR_ENTITY_CONSTRAINT_BROKEN = 2,
} ecsact_add_error;

typedef enum {
	/**
	 * Update component was successful
	 */
	ECSACT_UPDATE_OK = 0,

	/**
	 * An invalid or non-existant entity ID was found in one or more of the
	 * component fields.
	 */
	ECSACT_UPDATE_ERR_ENTITY_INVALID = 1,

	/**
	 * One or more of the component entity fields constraints were not satisifed.
	 */
	ECSACT_UPDATE_ERR_ENTITY_CONSTRAINT_BROKEN = 2,
} ecsact_update_error;

typedef enum {
	/**
	 * System execution completed successfully
	 */
	ECSACT_EXEC_SYS_OK = 0,

	/**
	 * An invalid or non-existant entity ID was found in one or more of the
	 * action fields.
	 */
	ECSACT_EXEC_SYS_ERR_ACTION_ENTITY_INVALID = 1,

	/**
	 * One or more of the action entity fields constraints were not satisifed.
	 */
	ECSACT_EXEC_SYS_ERR_ACTION_ENTITY_CONSTRAINT_BROKEN = 2,
} ecsact_execute_systems_error;

typedef enum {
	/**
	 * System may read component
	 */
	ECSACT_SYS_CAP_READONLY             = 1,

	/**
	 * System may only write to component.
	 * NOTE: This flag is only valid if accompanied by `ECSACT_SYS_CAP_READONLY`.
	 */
	ECSACT_SYS_CAP_WRITEONLY            = 2,

	/**
	 * System may read and write to component.
	 */
	ECSACT_SYS_CAP_READWRITE            = 3,

	/**
	 * System component is not required.
	 */
	ECSACT_SYS_CAP_OPTIONAL             = 4,

	/**
	 * System may read component, but component may not exist.
	 */
	ECSACT_SYS_CAP_OPTIONAL_READONLY    = ECSACT_SYS_CAP_OPTIONAL | ECSACT_SYS_CAP_READONLY,

	/**
	 * System may write to component, but component may not exist.
	 * NOTE: This flag is not allowed to be used standalone.
	 */
	ECSACT_SYS_CAP_OPTIONAL_WRITEONLY   = ECSACT_SYS_CAP_OPTIONAL | ECSACT_SYS_CAP_WRITEONLY,

	/**
	 * System may read and write to component, but component may not exist.
	 */
	ECSACT_SYS_CAP_OPTIONAL_READWRITE   = ECSACT_SYS_CAP_OPTIONAL | ECSACT_SYS_CAP_READWRITE,

	/**
	 * System may only execute on entities where this component is present, but
	 * the systme may not read or write to the component.
	 */
	ECSACT_SYS_CAP_INCLUDE              = 8,

	/**
	 * System may only execute on entities where this component does not exist.
	 */
	ECSACT_SYS_CAP_EXCLUDE              = 16,

	/**
	 * System may add this component to entities. Implies
	 * `ECSACT_SYS_CAP_EXCLUDE`
	 */
	ECSACT_SYS_CAP_ADDS                 = 32 | ECSACT_SYS_CAP_EXCLUDE,

	/**
	 * System may remove this component from entities. Implies 
	 * `ECSACT_SYS_CAP_INCLUDE`
	 */
	ECSACT_SYS_CAP_REMOVES              = 64 | ECSACT_SYS_CAP_INCLUDE,
} ecsact_system_capability;


/**
 * Flags for generates component set
 */
typedef enum {
	/**
	 * When generating the associated component must be present
	 */
	ECSACT_SYS_GEN_REQUIRED = 1,

	/**
	 * When generating the associated component may or may not be present
	 */
	ECSACT_SYS_GEN_OPTIONAL = 2,
} ecsact_system_generate;

/**
 * Comparison function between 2 components of the same type
 */
typedef int(*ecsact_component_compare_fn_t)(const void* a, const void* b);

/**
 * Comparison function between 2 actions of the same type
 */
typedef int(*ecsact_action_compare_fn_t)(const void* a, const void* b);

#endif // ECSACT_RUNTIME_COMMON_H
