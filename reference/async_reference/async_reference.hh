#pragma once

#include <map>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include <optional>
#include <variant>
#include <condition_variable>
#include "ecsact/runtime/core.hh"
#include "ecsact/runtime/async.h"

#include "reference/async_reference/detail/c_execution_options/c_execution_options.hh"
#include "reference/async_reference/util/types.hh"
#include "reference/async_reference/util/util.hh"
#include "reference/async_reference/tick_manager/tick_manager.hh"
#include "reference/async_reference/callbacks/execution_callbacks.hh"
#include "reference/async_reference/callbacks/async_callbacks.hh"
#include "reference/async_reference/entity_manager/entity_manager.hh"

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

	int32_t get_current_tick();

	ecsact_async_request_id connect(const char* connection_string);

	void disconnect();

private:
	std::atomic_int32_t _last_request_id = 0;

	std::optional<ecsact_registry_id> registry_id;

	tick_manager        tick_manager;
	execution_callbacks exec_callbacks;
	async_callbacks     async_callbacks;
	entity_manager      entity_manager;

	std::thread execution_thread;
	std::mutex  execution_m;

	std::atomic_bool is_connected = false;
	std::atomic_bool is_connected_notified = false;

	std::chrono::milliseconds tick_rate = {};

	ecsact_async_request_id next_request_id();
};
