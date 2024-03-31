#ifndef ECSACT_RUNTIME_COMMON_H
#define ECSACT_RUNTIME_COMMON_H

#include <stdint.h>

#ifdef __cplusplus
#	define ECSACT_TYPED_ID(name) enum class name : int32_t
#else
#	define ECSACT_TYPED_ID(name) typedef int32_t name
#endif

#ifdef __cplusplus
#	define ECSACT_EXTERN extern "C"
#else
#	define ECSACT_EXTERN extern
#endif

#if defined(__wasm__)
#	define ECSACT_EXPORT(ExportName) [[clang::export_name(ExportName)]]
#	define ECSACT_IMPORT(ImportModule, ImportName) \
		[[clang::import_module(ImportModule)]] [[clang::import_name(ImportName)]]
#elif defined(_WIN32)
#	define ECSACT_EXPORT(ExportName) __declspec(dllexport)
#	define ECSACT_IMPORT(ImportModule, ImportName) __declspec(dllimport)
#else
#	define ECSACT_EXPORT(ExportName) __attribute__((visibility("default")))
#	define ECSACT_IMPORT(ImportModule, ImportName)
#endif

ECSACT_TYPED_ID(ecsact_package_id);
ECSACT_TYPED_ID(ecsact_system_id);
ECSACT_TYPED_ID(ecsact_action_id);
ECSACT_TYPED_ID(ecsact_component_id);
ECSACT_TYPED_ID(ecsact_transient_id);
ECSACT_TYPED_ID(ecsact_enum_id);
ECSACT_TYPED_ID(ecsact_enum_value_id);
ECSACT_TYPED_ID(ecsact_field_id);
ECSACT_TYPED_ID(ecsact_variant_id);
ECSACT_TYPED_ID(ecsact_registry_id);
ECSACT_TYPED_ID(ecsact_entity_id);
ECSACT_TYPED_ID(ecsact_placeholder_entity_id);
ECSACT_TYPED_ID(ecsact_system_generates_id);
ECSACT_TYPED_ID(ecsact_async_request_id);

ECSACT_TYPED_ID(ecsact_decl_id);
ECSACT_TYPED_ID(ecsact_composite_id);
ECSACT_TYPED_ID(ecsact_system_like_id);
ECSACT_TYPED_ID(ecsact_component_like_id);

#ifdef __cplusplus
template<typename To, typename From>
To ecsact_id_cast(From);
#	define ECSACT_CAST_ID_FN(From, To)             \
		template<>                                    \
		inline To ecsact_id_cast<To, From>(From id) { \
			return (To)id;                              \
		}
#else
inline int32_t ecsact_id_cast(int32_t id) {
	return id;
}

#	define ECSACT_CAST_ID_FN(From, To)
#endif

ECSACT_CAST_ID_FN(ecsact_system_id, ecsact_system_like_id)
ECSACT_CAST_ID_FN(ecsact_action_id, ecsact_system_like_id)
ECSACT_CAST_ID_FN(ecsact_action_id, ecsact_composite_id)
ECSACT_CAST_ID_FN(ecsact_component_id, ecsact_composite_id)
ECSACT_CAST_ID_FN(ecsact_transient_id, ecsact_composite_id)
ECSACT_CAST_ID_FN(ecsact_component_like_id, ecsact_composite_id)

ECSACT_CAST_ID_FN(ecsact_component_id, ecsact_decl_id)
ECSACT_CAST_ID_FN(ecsact_transient_id, ecsact_decl_id)
ECSACT_CAST_ID_FN(ecsact_system_id, ecsact_decl_id)
ECSACT_CAST_ID_FN(ecsact_action_id, ecsact_decl_id)
ECSACT_CAST_ID_FN(ecsact_variant_id, ecsact_decl_id)
ECSACT_CAST_ID_FN(ecsact_system_like_id, ecsact_decl_id)
ECSACT_CAST_ID_FN(ecsact_composite_id, ecsact_decl_id)
ECSACT_CAST_ID_FN(ecsact_component_like_id, ecsact_decl_id)

ECSACT_CAST_ID_FN(ecsact_component_id, ecsact_component_like_id)
ECSACT_CAST_ID_FN(ecsact_transient_id, ecsact_component_like_id)

ECSACT_CAST_ID_FN(ecsact_package_id, ecsact_package_id)
ECSACT_CAST_ID_FN(ecsact_system_id, ecsact_system_id)
ECSACT_CAST_ID_FN(ecsact_action_id, ecsact_action_id)
ECSACT_CAST_ID_FN(ecsact_component_id, ecsact_component_id)
ECSACT_CAST_ID_FN(ecsact_enum_id, ecsact_enum_id)
ECSACT_CAST_ID_FN(ecsact_enum_value_id, ecsact_enum_value_id)
ECSACT_CAST_ID_FN(ecsact_field_id, ecsact_field_id)
ECSACT_CAST_ID_FN(ecsact_variant_id, ecsact_variant_id)
ECSACT_CAST_ID_FN(ecsact_registry_id, ecsact_registry_id)
ECSACT_CAST_ID_FN(ecsact_entity_id, ecsact_entity_id)
ECSACT_CAST_ID_FN(ecsact_decl_id, ecsact_decl_id)
ECSACT_CAST_ID_FN(ecsact_composite_id, ecsact_composite_id)
ECSACT_CAST_ID_FN(ecsact_system_like_id, ecsact_system_like_id)
ECSACT_CAST_ID_FN(ecsact_transient_id, ecsact_transient_id)
ECSACT_CAST_ID_FN(ecsact_component_like_id, ecsact_component_like_id)

#undef ECSACT_TYPED_ID
#undef ECSACT_CAST_ID_FN

#if defined(_WIN32) && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)
#	define ECSACT_MSVC_TRADITIONAL
#endif

#ifdef ECSACT_MSVC_TRADITIONAL
#	define ECSACT_MSVC_TRADITIONAL_ERROR_MESSAGE               \
		"\nTraditional MSVC preprocessor not supported.\n\tSee: " \
		"https://docs.microsoft.com/en-us/cpp/preprocessor/"      \
		"preprocessor-experimental-overview?view=msvc-160\n"
#	ifdef __cplusplus
#		define ECSACT_MSVC_TRADITIONAL_ERROR() \
			static_assert(false, ECSACT_MSVC_TRADITIONAL_ERROR_MESSAGE);
#	else
#		define ECSACT_MSVC_TRADITIONAL_ERROR() \
			_STATIC_ASSERT(false && ECSACT_MSVC_TRADITIONAL_ERROR_MESSAGE);
#	endif
#endif // ECSACT_MSVC_TRADITIONAL

/**
 * Context for system execution. This contains (or points to) state required for
 * executing a systems implementation. The structure for this type is runtime
 * implementation defined.
 */
struct ecsact_system_execution_context;

typedef void (*ecsact_system_execution_impl)(//
	struct ecsact_system_execution_context*
);

static const ecsact_system_id ecsact_invalid_system_id = (ecsact_system_id)-1;

static const ecsact_registry_id ecsact_invalid_registry_id =
	(ecsact_registry_id)-1;

static const ecsact_component_id ecsact_invalid_component_id =
	(ecsact_component_id)-1;

static const ecsact_entity_id ecsact_invalid_entity_id = (ecsact_entity_id)-1;

/**
 * Entity is from a generator system.
 */
static const ecsact_placeholder_entity_id ecsact_generated_entity =
	(ecsact_placeholder_entity_id)-1;

/**
 * Entity is from an external source such as over a network.
 */
static const ecsact_placeholder_entity_id ecsact_external_entity =
	(ecsact_placeholder_entity_id)-2;

typedef enum {
	/**
	 * Add component was successful
	 */
	ECSACT_ADD_OK = 0,

	/**
	 * An invalid or non-existent entity ID was found in one or more of the
	 * component fields.
	 */
	ECSACT_ADD_ERR_ENTITY_INVALID = 1,

	/**
	 * One or more of the component entity fields constraints were not satisfied.
	 */
	ECSACT_ADD_ERR_ENTITY_CONSTRAINT_BROKEN = 2,
} ecsact_add_error;

typedef enum {
	/**
	 * Update component was successful
	 */
	ECSACT_UPDATE_OK = 0,

	/**
	 * An invalid or non-existent entity ID was found in one or more of the
	 * component fields.
	 */
	ECSACT_UPDATE_ERR_ENTITY_INVALID = 1,

	/**
	 * One or more of the component entity fields constraints were not satisfied.
	 */
	ECSACT_UPDATE_ERR_ENTITY_CONSTRAINT_BROKEN = 2,
} ecsact_update_error;

typedef enum {
	/**
	 * System execution completed successfully
	 */
	ECSACT_EXEC_SYS_OK = 0,

	/**
	 * An invalid or non-existent entity ID was found in one or more of the
	 * action fields.
	 */
	ECSACT_EXEC_SYS_ERR_ACTION_ENTITY_INVALID = 1,

	/**
	 * One or more of the action entity fields constraints were not satisfied.
	 */
	ECSACT_EXEC_SYS_ERR_ACTION_ENTITY_CONSTRAINT_BROKEN = 2,
} ecsact_execute_systems_error;

typedef enum {
	/**
	 * System may read component
	 */
	ECSACT_SYS_CAP_READONLY = 1,

	/**
	 * System may only write to component.
	 * NOTE: This flag is only valid if accompanied by `ECSACT_SYS_CAP_READONLY`.
	 */
	ECSACT_SYS_CAP_WRITEONLY = 2,

	/**
	 * System may read and write to component.
	 */
	ECSACT_SYS_CAP_READWRITE = 3,

	/**
	 * System component is not required.
	 */
	ECSACT_SYS_CAP_OPTIONAL = 4,

	/**
	 * System may read component, but component may not exist.
	 */
	ECSACT_SYS_CAP_OPTIONAL_READONLY = ECSACT_SYS_CAP_OPTIONAL |
		ECSACT_SYS_CAP_READONLY,

	/**
	 * System may write to component, but component may not exist.
	 * NOTE: This flag is not allowed to be used standalone.
	 */
	ECSACT_SYS_CAP_OPTIONAL_WRITEONLY = ECSACT_SYS_CAP_OPTIONAL |
		ECSACT_SYS_CAP_WRITEONLY,

	/**
	 * System may read and write to component, but component may not exist.
	 */
	ECSACT_SYS_CAP_OPTIONAL_READWRITE = ECSACT_SYS_CAP_OPTIONAL |
		ECSACT_SYS_CAP_READWRITE,

	/**
	 * System may only execute on entities where this component is present, but
	 * the system may not read or write to the component.
	 */
	ECSACT_SYS_CAP_INCLUDE = 8,

	/**
	 * System may only execute on entities where this component does not exist.
	 */
	ECSACT_SYS_CAP_EXCLUDE = 16,

	/**
	 * System may add this component to entities. Implies
	 * `ECSACT_SYS_CAP_EXCLUDE`
	 */
	ECSACT_SYS_CAP_ADDS = 32 | ECSACT_SYS_CAP_EXCLUDE,

	/**
	 * System may remove this component from entities. Implies
	 * `ECSACT_SYS_CAP_INCLUDE`
	 */
	ECSACT_SYS_CAP_REMOVES = 64 | ECSACT_SYS_CAP_INCLUDE,
} ecsact_system_capability;

/**
 * System notify settings control when a system is executed. These settings are
 * on a per-component basis.
 */
typedef enum {
	/**
	 * Not a real setting. Used to unset a system notify setting.
	 */
	ECSACT_SYS_NOTIFY_NONE,

	/**
	 * Default behaviour. System always executes. If any component has this
	 * setting then all other settings are overwritten.
	 */
	ECSACT_SYS_NOTIFY_ALWAYS,

	/**
	 * System executes when the component is initialized.
	 */
	ECSACT_SYS_NOTIFY_ONINIT,

	/**
	 * System executes when the component is updated with a system update call.
	 */
	ECSACT_SYS_NOTIFY_ONUPDATE,

	/**
	 * System executes when the components fields have changed.
	 */
	ECSACT_SYS_NOTIFY_ONCHANGE,

	/**
	 * System executes when the component has been removed.
	 */
	ECSACT_SYS_NOTIFY_ONREMOVE,
} ecsact_system_notify_setting;

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
 * ees - Entity Execution Status
 *
 * An entities execution status gives details about how an entity relates to a
 * system or action's execution. By default an entities execution status is
 * `ECSACT_ESS_IDLE`.
 *
 * @note This detail is exposed mostly for serialization. It is important to
 * maintain this status to properly replicate simulations over something like a
 * network.
 */
typedef enum {
	/**
	 * Entity has no execution status
	 */
	ECSACT_EES_IDLE,

	/**
	 * Entity is waiting to be processed by the specified lazy system
	 */
	ECSACT_EES_PENDING_LAZY,
} ecsact_ees;

/**
 * Comparison function between 2 components of the same type
 */
typedef int (*ecsact_component_compare_fn_t)(const void* a, const void* b);

/**
 * Comparison function between 2 actions of the same type
 */
typedef int (*ecsact_action_compare_fn_t)(const void* a, const void* b);

/**
 * A convenient function for generating placeholder entity IDs. This is not a
 * necessary function. Any 32 bit integer >=0 is a valid placeholder entity ID.
 * Integers <0 are reserved as special placeholder IDs.
 */
inline ecsact_placeholder_entity_id ecsact_util_make_placeholder_entity_id(
	int32_t id
) {
	return (ecsact_placeholder_entity_id)id;
}

/**
 * Struct representing a single action
 */
typedef struct ecsact_action {
	/**
	 * ID of action originally given by `ecsact_register_action` or the statically
	 * known ID in the case of a compile time action.
	 */
	ecsact_action_id action_id;

	/**
	 * Pointer to action data. Size is determined by the registerd action
	 * associated with the `action_id`.
	 */
	const void* action_data;
} ecsact_action;

/**
 * Struct representing a single component
 */
typedef struct ecsact_component {
	/**
	 * ID of component originally given by `ecsact_register_component` or the
	 * statically known ID in the case of a compile time component.
	 */
	ecsact_component_id component_id;

	/**
	 * Pointer to component data. Size is determined by the registerd component
	 * associated with the `component_id`.
	 */
	const void* component_data;
} ecsact_component;

/**
 * Options related to system execution to be passed to `ecsact_execute_systems`
 */
typedef struct ecsact_execution_options {
	/**
	 * Length of `add_components_entities` and `add_components` sequential lists.
	 */
	int add_components_length;

	/**
	 * Sequential list of entities that will have components added determined by
	 * `add_components` before any system or action execution occurs. Length is
	 * determined by `add_components_length`.
	 */
	ecsact_entity_id* add_components_entities;

	/**
	 * Sequential list of components that will be added to the entities listed in
	 * `add_components_entities`. Length is determined by `add_components_length`.
	 */
	ecsact_component* add_components;

	/**
	 * Length of `update_components_entities` and `update_components` sequential
	 * lists.
	 */
	int update_components_length;

	/**
	 * Sequential list of entities that will have components updated determined
	 * by `update_components` before any system or action execution occurs. Length
	 * is determined by `update_components_length`.
	 */
	ecsact_entity_id* update_components_entities;

	/**
	 * Sequential list of components that will be updated on the entities listed
	 * in `update_components_entities`. Length is determined by
	 * `update_components_length`.
	 */
	ecsact_component* update_components;

	/**
	 * Length of `remove_components_entities` and `remove_components` sequential
	 * lists.
	 */
	int remove_components_length;

	/**
	 * Sequential list of entities that will have components removed determined
	 * by `remove_components` before any system or action execution occurs.
	 * Length is determined by `remove_components_length`.
	 */
	ecsact_entity_id* remove_components_entities;

	/**
	 * Sequential list of component IDs that will be removed on the entities
	 * listed in `remove_components_entities`. Length is determined by
	 * `remove_components_length`.
	 */
	ecsact_component_id* remove_components;

	/**
	 * Length of `actions` sequential list.
	 */
	int actions_length;

	/**
	 * Sequential list of actions to be executed.
	 */
	ecsact_action* actions;

	/**
	 * Length of entities to be created.
	 */
	int create_entities_length;

	/**
	 * Placeholder IDs that represent an entity.
	 */
	ecsact_placeholder_entity_id* create_entities;

	/**
	 * Sequential list of component lengths for each entity in
	 * `create_entities_components`.
	 */
	int* create_entities_components_length;

	/**
	 * A sequential 2D list that represents a set of entities with a list of
	 * components. Entity length is determined by `create_entities_length`. The
	 * length of components for each entity is determined by
	 * `create_entities_components_length`.
	 */
	ecsact_component** create_entities_components;

	/**
	 * Length of `destroy_entities` sequential list.
	 */
	int destroy_entities_length;

	/**
	 * A sequentual list of entity IDs that will be destroyed along with its
	 * components. Length is determined by `destroy_entities_length`.
	 */
	ecsact_entity_id* destroy_entities;

} ecsact_execution_options;

typedef enum {
	/**
	 * Initialized component - Newly added component during execution.
	 *
	 * `component_data` does not necessarily reflect the component_data when the
	 * component was original added due to the component potentially being
	 * modified during system execution or (in the case of multi-execution calls)
	 * modification across executions.
	 */
	ECSACT_EVENT_INIT_COMPONENT = 0,

	/**
	 * Update component - Component has been modified during execution.
	 *
	 * This event never occurs for components without any fields (tag components).
	 *
	 * If a component is modified during execution
	 */
	ECSACT_EVENT_UPDATE_COMPONENT = 1,

	/**
	 * Remove component - Component has been removed during execution.
	 *
	 * If a component is removed and then re-added during single or
	 * multi-execution calls this event will not occur. The event only occurs
	 * when at the end of the execution call the component is removed.
	 */
	ECSACT_EVENT_REMOVE_COMPONENT = 2,

	/**
	 * Create entity - Entity has been created during execution.
	 *
	 * Happens on entity creation and before any `INIT_COMPONENT`
	 * events. It will also be triggered by any generated entities. Returns the
	 * entity ID associated with the created entity
	 */
	ECSACT_EVENT_CREATE_ENTITY = 3,

	/**
	 * Destroy entity - Entity has been destroyed during execution.
	 *
	 * Invoked when an entity is destroyed. Any components associated with the
	 * entity will also be destroyed. Returns the entity ID associated with the
	 * destroyed entity
	 */
	ECSACT_EVENT_DESTROY_ENTITY = 4,
} ecsact_event;

/**
 * Component event callback
 */
typedef void (*ecsact_component_event_callback)( //
	ecsact_event        event,
	ecsact_entity_id    entity_id,
	ecsact_component_id component_id,
	const void*         component_data,
	void*               callback_user_data
);

/**
 * Entity event callback
 * @param event always ECSACT_EVENT_CREATE_ENTITY or ECSACT_EVENT_DESTROY_ENTITY
 * @param entity_id the entity that was created or destroyed
 * @param placeholder_entity_id the placeholder entity ID originally given in
 *        execution options or one of the constant placeholder entity IDs.
 * @param  * @param callback_user_data void pointer originally given at
 * execution / flush
 */
typedef void (*ecsact_entity_event_callback)( //
	ecsact_event                 event,
	ecsact_entity_id             entity_id,
	ecsact_placeholder_entity_id placeholder_entity_id,
	void*                        callback_user_data
);

/**
 * Holds event handler callbacks and their user data
 */
typedef struct ecsact_execution_events_collector {
	/**
	 * Invoked after system executions are finished for every component that is
	 * new. The component_data is the last value given for the component, not the
	 * first. Invocation happens in the calling thread. `event` will always be
	 * `ECSACT_EVENT_INIT_COMPONENT`
	 */
	ecsact_component_event_callback init_callback;

	/**
	 * `callback_user_data` passed to `init_callback`
	 */
	void* init_callback_user_data;

	/**
	 * Invoked after system executions are finished for every changed component.
	 * Invocation happens in the calling thread. `event` will always be
	 * `ECSACT_EVENT_UPDATE_COMPONENT`
	 */
	ecsact_component_event_callback update_callback;

	/**
	 * `callback_user_data` passed to `update_callback`
	 */
	void* update_callback_user_data;

	/**
	 * Invoked after system executions are finished for every removed component.
	 * Invocation happens in the calling thread. `event` will always be
	 * `ECSACT_EVENT_REMOVE_COMPONENT`.
	 */
	ecsact_component_event_callback remove_callback;

	/**
	 * `callback_user_data` passed to `remove_callback`
	 */
	void* remove_callback_user_data;

	/**
	 * Invoked after system executions are finished for every created entity.
	 * Invocation happens in the calling thread. `event` will will always be
	 * `ECSACT_EVENT_CREATE_ENTITY`.
	 */
	ecsact_entity_event_callback entity_created_callback;

	/**
	 * `callback_user_data` passed to `entity_created_callback`
	 */
	void* entity_created_callback_user_data;

	/**
	 * Invoked after system executions are finished for every removed component.
	 * Invocation happens in the calling thread. `event` will will always be
	 * `ECSACT_EVENT_DESTROY_COMPONENT`.
	 */
	ecsact_entity_event_callback entity_destroyed_callback;

	/**
	 * `callback_user_data` passed to `entity_destroyed_callback`
	 */
	void* entity_destroyed_callback_user_data;

} ecsact_execution_events_collector;

#endif // ECSACT_RUNTIME_COMMON_H
