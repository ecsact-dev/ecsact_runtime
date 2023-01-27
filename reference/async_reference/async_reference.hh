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
#include "reference/async_reference/request_id_factory/request_id_factory.hh"
#include "request_id_factory/request_id_factory.hh"

namespace ecsact::async_reference::detail {
class async_reference {
public:
	inline async_reference(
		request_id_factory& request_id_factory,
		async_callbacks&    async_callbacks
	)
		: request_id_factory(request_id_factory), async_callbacks(async_callbacks) {
	}

	inline ~async_reference() {
	}

	void enqueue_execution_options(
		ecsact_async_request_id         req_id,
		const ecsact_execution_options& options
	);

	void execute_systems();

	void invoke_execution_events(
		const ecsact_execution_events_collector* execution_evc
	);

	int32_t get_current_tick();

	void create_entity_request(ecsact_async_request_id req_id);
	void connect(ecsact_async_request_id req_id, const char* connection_string);

	void disconnect();

private:
	std::atomic_int32_t _last_request_id = 0;

	std::optional<ecsact_registry_id> registry_id;

	tick_manager        tick_manager;
	execution_callbacks exec_callbacks;
	entity_manager      entity_manager;

	detail::request_id_factory& request_id_factory;
	detail::async_callbacks&    async_callbacks;

	std::thread execution_thread;
	std::mutex  execution_m;

	std::atomic_bool is_connected = false;
	std::atomic_bool is_connected_notified = false;

	std::chrono::milliseconds tick_rate = {};
};
}; // namespace ecsact::async_reference::detail
