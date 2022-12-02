#ifndef ECSACT_RUNTIME_ASYNC_H
#define ECSACT_RUNTIME_ASYNC_H

#include <stdint.h>
#include "ecsact/runtime/common.h"
#include "ecsact/runtime/core.h"

#ifndef ECSACT_ASYNC_API_VISIBILITY
#	ifdef ECSACT_ASYNC_API_LOAD_AT_RUNTIME
#		define ECSACT_ASYNC_API_VISIBILITY
#	else
#		ifdef ECSACT_ASYNC_API_EXPORT
#			ifdef _WIN32
#				define ECSACT_ASYNC_API_VISIBILITY __declspec(dllexport)
#			else
#				define ECSACT_ASYNC_API_VISIBILITY \
					__attribute__((visibility("default")))
#			endif
#		else
#			ifdef _WIN32
#				define ECSACT_ASYNC_API_VISIBILITY __declspec(dllimport)
#			else
#				define ECSACT_ASYNC_API_VISIBILITY
#			endif
#		endif
#	endif
#endif // ECSACT_ASYNC_API_VISIBILITY

#ifndef ECSACT_ASYNC_API
#	ifdef __cplusplus
#		define ECSACT_ASYNC_API extern "C" ECSACT_ASYNC_API_VISIBILITY
#	else
#		define ECSACT_ASYNC_API extern ECSACT_ASYNC_API_VISIBILITY
#	endif
#endif // ECSACT_ASYNC_API

#ifndef ECSACT_ASYNC_API_FN
#	ifdef ECSACT_ASYNC_API_LOAD_AT_RUNTIME
#		define ECSACT_ASYNC_API_FN(ret, name) ECSACT_ASYNC_API ret(*name)
#	else
#		define ECSACT_ASYNC_API_FN(ret, name) ECSACT_ASYNC_API ret name
#	endif
#endif // ECSACT_ASYNC_API_FN

typedef enum {
	/**
	 * No error
	 */
	ECSACT_ASYNC_OK = 0,

	/**
	 * Client has invalid permissions
	 */
	ECSACT_ASYNC_ERR_PERMISSION_DENIED,

	/**
	 * Connection to the client is closed
	 */
	ECSACT_ASYNC_ERR_CONNECTION_CLOSED,

	/**
	 * ExecutionOptions failed to merge, and upon failure the connection is closed
	 */
	ECSACT_ASYNC_ERR_EXECUTION_MERGE_FAILURE,
} ecsact_async_error;

/**
 * When an error occurs due to an async request this callback is invoked.
 *
 * @param async_err when there is no async error this will be @ref
 * ECSACT_ASYNC_OK otherwise @see ecsact_async_error

 * @param request_id the request ID returned by an async request function that
 * was responsible for this error
 * @param callback_user_data the @ref
 * ecsact_async_events_collector::error_callback_user_data
 */
typedef void (*ecsact_async_error_callback)(
	//
	ecsact_async_error      async_err,
	ecsact_async_request_id request_id,
	void*                   callback_user_data
);

/**
 * When an occurs from the system execution this callback is invoked.
 *
 * @param execute_err when there is no system execution error, this will be
 * @ref ECSACT_EXEC_SYS_OK other @see ecsact_execute_systems_error
 */
typedef void (*ecsact_execute_sys_error_callback)(
	//
	ecsact_execute_systems_error execute_err,
	void*                        callback_user_data
);

/**
 * When an entity is sucessfully created this callback is
 * invoked.
 *
 * @param entity_id the entity id of the created entity
 * @param request_id the request ID returned by the create entity callback
 * @param callback_user_data the @ref
 * ecsact_async_events_collector::error_callback_user_data
 */
typedef void (*ecsact_async_create_entity_callback)(
	//
	ecsact_entity_id        entity_id,
	ecsact_async_request_id request_id,
	void*                   callback_user_data
);

typedef struct ecsact_async_events_collector {
	/**
	 * invoked when an async request failed.
	 * @see ecsact_async_error_callback
	 * @see ecsact_async_error
	 */
	ecsact_async_error_callback async_error_callback;

	/**
	 * `callback_user_data` passed to `error_callback`
	 */
	void* async_error_callback_user_data;

	/**
	 * Invoked when a create entity request succeeds.
	 * @see ecsact_async_create_entity_callback
	 * @see ecsact_entity_id
	 * @see ecsact_async_error
	 */
	ecsact_async_create_entity_callback async_entity_callback;

	/**
	 * `callback_user_data` passed to `error_callback`
	 */
	void* async_entity_error_callback_user_data;

	/**
	 * invoked when a system execution error occurred.
	 * @see ecsact_execute_sys_error_callback
	 * @see ecsact_execute_systems_error
	 */
	ecsact_execute_sys_error_callback system_error_callback;

	/**
	 * `callback_user_data` passed to `error_callback`
	 */
	void* system_error_callback_user_data;
} ecsact_async_events_collector;

/**
 * Enqueues system execution options that will be used during system execution
 * as soon as possible.
 * @param options - the options passed to `ecsact_execute_systems` in the Async
 * module
 * @returns a request ID representing this async request. Later used in @ref
 * ecsact_async_error_callback if an error occurs
 */
ECSACT_ASYNC_API_FN(
	ecsact_async_request_id,
	ecsact_async_enqueue_execution_options
)
( //
	const ecsact_execution_options options
);

/**
 * Invokes the various callbacks in `execution_events` and `async_events` that
 * have been pending. If either a system or async error occurs it's treated
 * as a call to ecscact_async_disconnect
 */
ECSACT_ASYNC_API_FN(void, ecsact_async_flush_events)
( //
	const ecsact_execution_events_collector* execution_events,
	const ecsact_async_events_collector*     async_events
);

/**
 * @param connection_string - null-terminated string used to connect to the
 *        underlying async runtime. This may be a hostname/ip address + port or
 *        some other string deinfed by the implementation. Please review
 *        documentation for your ecsact async api provider. May be NULL to
 *        indiciate wanting to connect to the 'default' if available.
 * @returns a request ID representing this async request. Later used in @ref
 * ecsact_async_error_callback if an error occurs
 */
ECSACT_ASYNC_API_FN(ecsact_async_request_id, ecsact_async_connect)
( //
	const char* connection_string
);

/**
 * Starts a disconnect. May happen in background, but is guaranteed to
 * disconnect before any new @ref ecsact_async_connect resolves.
 */
ECSACT_ASYNC_API_FN(void, ecsact_async_disconnect)(void);

/**
 * Returns an entity
 */
ECSACT_ASYNC_API_FN(ecsact_async_request_id, ecsact_async_request_entity)(void);

#define FOR_EACH_ECSACT_ASYNC_API_FN(fn, ...)              \
	fn(ecsact_async_enqueue_execution_options, __VA_ARGS__); \
	fn(ecsact_async_flush_events, __VA_ARGS__);              \
	fn(ecsact_async_connect, __VA_ARGS__);                   \
	fn(ecsact_async_disconnect, __VA_ARGS__);                \
	fn(ecsact_async_request_entity, __VA_ARGS__);

#undef ECSACT_ASYNC_API
#undef ECSACT_ASYNC_API_FN
#endif // ECSACT_RUNTIME_ASYNC_H
