#ifndef ECSACT_RUNTIME_ASYNC_H
#define ECSACT_RUNTIME_ASYNC_H

#include <stdint.h>
#include "ecsact/runtime/common.h"
#include "ecsact/runtime/core.h"

#ifdef ECSACT_ASYNC_API_VISIBILITY
#	error "ECSACT_ASYNC_API_VISIBILITY define is deprecated"
#endif

#ifndef ECSACT_ASYNC_API_FN
#	if defined(ECSACT_ASYNC_API)
#		define ECSACT_ASYNC_API_FN(ret, name) ECSACT_ASYNC_API ret name
#	elif defined(ECSACT_ASYNC_API_LOAD_AT_RUNTIME)
#		if defined(ECSACT_ASYNC_API_EXPORT)
#			define ECSACT_ASYNC_API_FN(ret, name) \
				ECSACT_EXTERN ECSACT_EXPORT(#name) ret(*name)
#		else
#			define ECSACT_ASYNC_API_FN(ret, name) \
				ECSACT_EXTERN ECSACT_IMPORT("env", #name) ret(*name)
#		endif
#	elif defined(ECSACT_ASYNC_API_EXPORT)
#		define ECSACT_ASYNC_API_FN(ret, name) \
			ECSACT_EXTERN ECSACT_EXPORT(#name) ret name
#	else
#		define ECSACT_ASYNC_API_FN(ret, name) \
			ECSACT_EXTERN ECSACT_IMPORT("env", #name) ret name
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
	 * Client sent invalid connection options
	 */
	ECSACT_ASYNC_INVALID_CONNECTION_STRING,

	/**
	 * Connection to the client is closed
	 */
	ECSACT_ASYNC_ERR_CONNECTION_CLOSED,

	/**
	 * ExecutionOptions failed to merge, and upon failure the connection is closed
	 */
	ECSACT_ASYNC_ERR_EXECUTION_MERGE_FAILURE,

	/**
	 * A request was made before connection was established
	 */
	ECSACT_ASYNC_ERR_NOT_CONNECTED,

	/**
	 * Internal error has occurred
	 */
	ECSACT_ASYNC_ERR_INTERNAL = 99,

	/**
	 * Start of custom implementation specific error codes
	 */
	ECSACT_ASYNC_ERR_CUSTOM_BEGIN = 100,

	/**
	 * End of custom implementation specific error codes
	 */
	ECSACT_ASYNC_ERR_CUSTOM_END = 200,
} ecsact_async_error;

typedef enum {
	/**
	 * The session has started
	 */
	ECSACT_ASYNC_SESSION_STARTED = 0,

	/**
	 * The session has stopped
	 */
	ECSACT_ASYNC_SESSION_STOPPED = 1,
} ecsact_async_session_event;

/**
 * When an error occurs due to an async request this callback is invoked.
 *
 * @param session_id the session that received the error
 *
 * @param async_err when there is no async error this will be @ref
 * ECSACT_ASYNC_OK otherwise @see ecsact_async_error

 * @param request_ids A list of request IDs returned by an async request
 function that
 * was responsible for this error
 * @param callback_user_data the @ref
 * ecsact_async_events_collector::error_callback_user_data
 */
typedef void (*ecsact_async_error_callback)( //
	ecsact_async_session_id  session_id,
	ecsact_async_error       async_err,
	int                      request_ids_length,
	ecsact_async_request_id* request_ids,
	void*                    callback_user_data
);

/**
 * When an occurs from the system execution this callback is invoked.
 *
 * @param session_id the session that received the error
 * @param execute_err when there is no system execution error, this will be
 * @ref ECSACT_EXEC_SYS_OK other @see ecsact_execute_systems_error
 */
typedef void (*ecsact_execute_sys_error_callback)( //
	ecsact_async_session_id      session_id,
	ecsact_execute_systems_error execute_err,
	void*                        callback_user_data
);

/**
 * Handler for when a request is done (error or success)
 */
typedef void (*ecsact_async_request_done_callback)( //
	ecsact_async_session_id  session_id,
	int                      request_ids_length,
	ecsact_async_request_id* request_ids,
	void*                    callback_user_data
);

/**
 * Handler async session events
 */
typedef void (*ecsact_async_session_event_callback)( //
	ecsact_async_session_id    session_id,
	ecsact_async_session_event event,
	void*                      callback_user_data
);

typedef struct ecsact_async_events_collector {
	/**
	 * invoked when an async request failed.
	 * @see ecsact_async_error_callback
	 * @see ecsact_async_error
	 */
	ecsact_async_error_callback async_error_callback;

	/**
	 * `callback_user_data` passed to `async_error_callback`
	 */
	void* async_error_callback_user_data;

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

	/**
	 * invoked when requests are done (error or success). The request IDs passed
	 * to the callback are no longer valid, but may be reused for future
	 * requests.
	 */
	ecsact_async_request_done_callback async_request_done_callback;

	/**
	 * `callback_user_data` passed to `async_request_done_callback`
	 */
	void* async_request_done_callback_user_data;

	/**
	 * invoked when a session event has occured. @see ecsact_async_session_event
	 */
	ecsact_async_session_event_callback async_session_event_callback;

	/**
	 * `callback_user_data` passed to `async_session_event_callback`
	 */
	void* async_session_event_callback_user_data;
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
	ecsact_async_session_id        session_id,
	const ecsact_execution_options options
);

/**
 * Invokes the various callbacks in `execution_events` and `async_events` that
 * have been pending. If either a system or async error occurs it's treated
 * as a call to ecscact_async_disconnect
 */
ECSACT_ASYNC_API_FN(void, ecsact_async_flush_events)
( //
	ecsact_async_session_id                  session_id,
	const ecsact_execution_events_collector* execution_events,
	const ecsact_async_events_collector*     async_events
);

/**
 * @param option_data implementation defined options used to start the async
 * session. This usually contains information such as the host/ip, port, and
 * authentication for a network connection or various settings that affect the
 * simulation. It is recommended that all implementations handle `NULL` as the
 * 'default' options. You should refer to the implementations documentation for
 * what should be passed here.
 *
 * @param option_data_size length (in bytes) of @p option_data
 *
 * @returns an ID that represents the session that all other async functions
 * take in
 */
ECSACT_ASYNC_API_FN(ecsact_async_session_id, ecsact_async_start)
( //
	const void* option_data,
	int32_t     option_data_size
);

/**
 * Begins stopping the session. May happen in background.
 * @param session_id the session that should stop
 */
ECSACT_ASYNC_API_FN(void, ecsact_async_stop)
( //
	ecsact_async_session_id session_id
);

/**
 * Gets the current tick
 */
ECSACT_ASYNC_API_FN(int32_t, ecsact_async_get_current_tick)
( //
	ecsact_async_session_id session_id
);

/**
 * Sends Ecsact stream data to the specified registry. Stream data will be
 * applied on the next ecsact_execute_systems call.
 *
 * @param indexed_field_values if the component has indexed fields then those
 * fields must be supplied as a sequential array in declaration order,
 * otherwise may be NULL.
 */
ECSACT_ASYNC_API_FN(void, ecsact_async_stream)
( //
	ecsact_async_session_id session_id,
	ecsact_entity_id        entity,
	ecsact_component_id     component_id,
	const void*             component_data,
	const void*             indexed_field_values
);

#ifdef ECSACT_MSVC_TRADITIONAL
#	define FOR_EACH_ECSACT_ASYNC_API_FN(fn, ...) ECSACT_MSVC_TRADITIONAL_ERROR()
#else
#	define FOR_EACH_ECSACT_ASYNC_API_FN(fn, ...)              \
		fn(ecsact_async_enqueue_execution_options, __VA_ARGS__); \
		fn(ecsact_async_flush_events, __VA_ARGS__);              \
		fn(ecsact_async_start, __VA_ARGS__);                     \
		fn(ecsact_async_stop, __VA_ARGS__);                      \
		fn(ecsact_async_get_current_tick, __VA_ARGS__);          \
		fn(ecsact_async_stream, __VA_ARGS__)
#endif

#endif // ECSACT_RUNTIME_ASYNC_H
