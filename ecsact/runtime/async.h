#ifndef ECSACT_RUNTIME_ASYNC_H
#define ECSACT_RUNTIME_ASYNC_H

#include <stdint.h>
#include "ecsact/runtime/common.h"
// TODO(zaucy): remove this include and move execution options to common.h?
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
	 *
	 */
	ECSACT_ASYNC_ERR_PERMISSION_DENIED,

	/**
	 *
	 */
	ECSACT_ASYNC_ERR_CONNECTION_CLOSED,

	ECSACT_ASYNC_ERR_COMPONENT_ACCESS
} ecsact_async_error;

typedef void (*ecsact_async_error_callback)(
	//
	ecsact_async_error           async_err,
	ecsact_execute_systems_error execute_err,
	ecsact_async_request_id      request_id,
	void*                        callback_user_data
);

typedef struct ecsact_async_events_collector {
	/**
	 *
	 */
	ecsact_async_error_callback error_callback;

	/**
	 *
	 */
	void* error_callback_user_data;
} ecsact_async_events_collector;

/**
 * Sends a request to execute an action as soon as possible.
 * @returns a request ID used to identify which async event is associated with
 *          this call. SEE: `ecsact_async_events_collector`
 */
// ECSACT_ASYNC_API(ecsact_async_request_id, ecsact_async_execute_action_now)
// 	( ecsact_system_id  action_id
// 	, const void*       action_data
// 	);

/**
 * Sends a request to execute an action at a given `tick`.
 * @returns a request ID used to identify which async event is associated with
 *          this call. SEE: `ecsact_async_events_collector`
 */
// ECSACT_ASYNC_API(ecsact_async_request_id, ecsact_async_execute_action_at)
// 	( ecsact_system_id  action_id
// 	, const void*       action_data
// 	, int32_t           tick
// 	);

/**
 * Enqueues system execution options that will be used during system execution
 * as soon as possible.
 * @param options - the options passed to `ecsact_execute_systems` in the ASYNC
 * module
 * @returns
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
 * @param options_list a squential list of execution options. Length is
 * determined by @p list_length
 * @returns a request ID TODO(zaucy): talk about errors
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
 */
ECSACT_ASYNC_API_FN(ecsact_async_request_id, ecsact_async_connect)
( //
	const char* connection_string
);

#define FOR_EACH_ECSACT_ASYNC_API_FN(fn, ...)       \
	fn(ecsact_async_execute_action_now, __VA_ARGS__); \
	fn(ecsact_async_execute_action_at, __VA_ARGS__);  \
	fn(ecsact_async_flush_events, __VA_ARGS__);       \
	fn(ecsact_async_connect, __VA_ARGS__)

#undef ECSACT_ASYNC_API
#undef ECSACT_ASYNC_API_FN
#endif // ECSACT_RUNTIME_ASYNC_H
