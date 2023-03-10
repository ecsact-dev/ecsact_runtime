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
	 * the systme may not read or write to the component.
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

#endif // ECSACT_RUNTIME_COMMON_H
