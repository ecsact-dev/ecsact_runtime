#ifndef ECSACT_RUNTIME_SERIALIZE_H
#define ECSACT_RUNTIME_SERIALIZE_H

#include <stdint.h>
#include <stdbool.h>
#include "ecsact/runtime/common.h"

#ifdef ECSACT_SERIALIZE_API_VISIBILITY
#	error "ECSACT_SERIALIZE_API_VISIBILITY define is deprecated"
#endif

#ifndef ECSACT_SERIALIZE_API_FN
#	if defined(ECSACT_SERIALIZE_API)
#		define ECSACT_SERIALIZE_API_FN(ret, name) ECSACT_SERIALIZE_API ret name
#	elif defined(ECSACT_SERIALIZE_API_LOAD_AT_RUNTIME)
#		define ECSACT_SERIALIZE_API_FN(ret, name) ECSACT_EXTERN ret(*name)
#	elif defined(ECSACT_SERIALIZE_API_EXPORT)
#		define ECSACT_SERIALIZE_API_FN(ret, name) \
			ECSACT_EXTERN ECSACT_EXPORT(#name) ret name
#	else
#		define ECSACT_SERIALIZE_API_FN(ret, name) \
			ECSACT_EXTERN ECSACT_IMPORT("env", #name) ret name
#	endif
#endif // ECSACT_SERIALIZE_API_FN

/**
 * Get the amount of bytes an action with id `action_id` requies to serialize.
 */
ECSACT_SERIALIZE_API_FN(int, ecsact_serialize_action_size)
( //
	ecsact_action_id action_id
);

/**
 * Get the amount of bytes a component with id `component_id` requies to
 * serialize.
 */
ECSACT_SERIALIZE_API_FN(int, ecsact_serialize_component_size)
( //
	ecsact_component_id component_id
);

/**
 * Serialize action into implementation defined format suitable for sending over
 * a socket and/or written to a file. Guranteed to be deserializable across
 * platforms.
 *
 * @param action_id Valid action ID associated with `action_data`
 * @param in_action_data Valid action data associated with `action_id`
 * @param out_bytes Sequential byte array pointer that will be written to. The
 *        memory must be pre-allocated by the caller. The required size can be
 *        queried for with `ecsact_serialize_action_size()`. The action ID is
 *        not serialized.
 * @returns amount of bytes written to `out_bytes`.
 */
ECSACT_SERIALIZE_API_FN(int, ecsact_serialize_action)
( //
	ecsact_action_id action_id,
	const void*      in_action_data,
	uint8_t*         out_bytes
);

ECSACT_SERIALIZE_API_FN(int, ecsact_deserialize_action)
( //
	ecsact_action_id action_id,
	const uint8_t*   in_bytes,
	void*            out_action_data
);

ECSACT_SERIALIZE_API_FN(int, ecsact_serialize_component)
( //
	ecsact_component_id component_id,
	const void*         in_component_data,
	uint8_t*            out_bytes
);

ECSACT_SERIALIZE_API_FN(int, ecsact_deserialize_component)
( //
	ecsact_component_id component_id,
	const uint8_t*      in_bytes,
	void*               out_component_data
);

typedef void (*ecsact_dump_entities_callback)( //
	const void* data,
	int32_t     data_length,
	void*       callback_user_data
);

/**
 * Invokes @p callback an unspecified amount of times with chunks of data
 * representing all the entities in @p registry.
 */
ECSACT_SERIALIZE_API_FN(void, ecsact_dump_entities)
( //
	ecsact_registry_id            registry,
	ecsact_dump_entities_callback callback,
	void*                         callback_user_data
);

typedef int32_t (*ecsact_restore_entities_callback)( //
	void*   out_data,
	int32_t data_max_length,
	void*   callback_user_data
);

/**
 * Clears @p registry and invokes @p callback until it returns `0` creating
 * entities and adding components from data given from the @p callback.
 */
ECSACT_SERIALIZE_API_FN(void, ecsact_restore_entities)
( //
	ecsact_registry_id                       registry,
	ecsact_restore_entities_callback         callback,
	const ecsact_execution_events_collector* events_collector,
	void*                                    callback_user_data
);

// # BEGIN FOR_EACH_ECSACT_SERIALIZE_API_FN
#ifdef ECSACT_MSVC_TRADITIONAL
#	define FOR_EACH_ECSACT_SERIALIZE_API_FN(fn, ...) \
		ECSACT_MSVC_TRADITIONAL_ERROR()
#else
#	define FOR_EACH_ECSACT_SERIALIZE_API_FN(fn, ...)   \
		fn(ecsact_serialize_action_size, __VA_ARGS__);    \
		fn(ecsact_serialize_component_size, __VA_ARGS__); \
		fn(ecsact_serialize_action, __VA_ARGS__);         \
		fn(ecsact_deserialize_action, __VA_ARGS__);       \
		fn(ecsact_serialize_component, __VA_ARGS__);      \
		fn(ecsact_deserialize_component, __VA_ARGS__);    \
		fn(ecsact_dump_entities, __VA_ARGS__);            \
		fn(ecsact_restore_entities, __VA_ARGS__)

#endif

#endif // ECSACT_RUNTIME_SERIALIZE_H
