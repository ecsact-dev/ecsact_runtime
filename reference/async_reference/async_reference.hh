#pragma once

#include <map>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include <optional>
#include <variant>

#include "reference/async_reference/util/types.hh"
#include "reference/async_reference/util/util.hh"
#include "reference/async_reference/tick_manager/tick_manager.hh"
#include "reference/async_reference/callbacks/execution_callbacks.hh"
#include "reference/async_reference/callbacks/async_callbacks.hh"

#include "ecsact/runtime/core.hh"
#include "ecsact/runtime/async.h"

// Have one container to hold ALL requests
// Use an std::variant to account for entity and other types
// Separate classes, you are simulating global state by having so much
// functionality that don't involve each other

// Async possibilities:
// 	Tick manager
// 	Requests (Async callbacks)
//

class async_reference {
public:
	ecsact_async_request_id enqueue_execution_options(
		const ecsact_execution_options& options
	);

	void execute_systems();

	void flush_events(
		const ecsact_execution_events_collector* execution_events,
		const ecsact_async_events_collector*     async_events
	);

	ecsact_async_request_id create_entity_request();

	ecsact_async_request_id connect(const char* connection_string);

	void disconnect();

private:
	std::atomic_int32_t _last_request_id = 0;

	std::optional<ecsact_registry_id> registry_id;
	std::optional<ecsact_registry_id> pending_registry_id;

	tick_manager        tick_manager;
	execution_callbacks exec_callbacks;
	async_callbacks     async_callbacks;

	std::vector<ecsact_async_request_id> pending_entity_requests;

	std::thread      execution_thread;
	std::mutex       pending_m;
	std::atomic_bool is_connected = false;

	ecsact_async_request_id next_request_id();
	ecsact_async_request_id convert_request_id(int32_t id);
};
