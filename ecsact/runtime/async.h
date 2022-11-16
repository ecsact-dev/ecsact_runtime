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
	 * ExecutionOptions failed to merge
	 */
	ECSACT_ASYNC_ERR_EXECUTION_MERGE_FAILURE,
} ecsact_async_error;

/**
 * When an error occurs due to an async request this callback is invoked, only
 * either @p async_err or @p execute_err will have a non-ok value.
 *
 * @param async_err when there is no async error this will be @ref
 * ECSACT_ASYNC_OK otherwise @see ecsact_async_error
 * @param execute_err when there is no system execution error, this will be @ref
 * ECSACT_EXEC_SYS_OK other @see ecsact_execute_systems_error
 * @param request_id the request ID returned by an async request function that
 * was responsible for this error
 * @param callback_user_data the @ref
 * ecsact_async_events_collector::error_callback_user_data
 */
typedef void (*ecsact_async_error_callback)(
	//
	ecsact_async_error           async_err,
	ecsact_execute_systems_error execute_err,
	ecsact_async_request_id      request_id,
	void*                        callback_user_data
);

typedef struct ecsact_async_events_collector {
	/**
	 * invoked when an async request failed.
	 * @see ecsact_async_error_callback
	 * @see ecsact_async_error
	 * @see ecsact_execute_systems_error
	 */
	ecsact_async_error_callback error_callback;

	/**
	 * `callback_user_data` passed to `error_callback`
	 */
	void* error_callback_user_data;
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
 * Enqueues system execution options at the specified ticks. If multiple
 * invocations of `ecsact_async_enqueue_execution_options_at` happen for the
 * same tick(s) the execution options will be _merged_.
 *
 * @param list_length the length of @p tick_list and @p options_list
 * @param tick_list a sequential list of ticks the execution options in @p
 * options_list will be used during system exeuction. Length is determined by @p
 * list_length
 * @param options_list a sequential list of execution options. Length is
 * determined by @p list_length
 * @returns a request ID representing this async request. Later used in @ref
 * ecsact_async_error_callback if an error occurs
 */

ECSACT_ASYNC_API_FN(
	ecsact_async_request_id,
	ecsact_async_enqueue_execution_options_at
)
( //
	int                             list_length,
	const int*                      tick_list,
	const ecsact_execution_options* options_list
);

/**
 * Invokes the various callbacks in `execution_events` and `async_events` that
 * have been pending.
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

#define FOR_EACH_ECSACT_ASYNC_API_FN(fn, ...)                 \
	fn(ecsact_async_enqueue_execution_options, __VA_ARGS__);    \
	fn(ecsact_async_enqueue_execution_options_at, __VA_ARGS__); \
	fn(ecsact_async_flush_events, __VA_ARGS__);                 \
	fn(ecsact_async_connect, __VA_ARGS__);                      \
	fn(ecsact_async_disconnect, __VA_ARGS__);

#undef ECSACT_ASYNC_API
#undef ECSACT_ASYNC_API_FN
#endif // ECSACT_RUNTIME_ASYNC_H
