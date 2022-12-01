#include <map>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include <optional>

#include "reference/async_reference/util/types.hh"
#include "reference/async_reference/util/util.hh"

#include "ecsact/runtime/core.hh"
#include "ecsact/runtime/async.h"

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

	ecsact_entity_id create_entity();

	ecsact_async_request_id connect(const char* connection_string);
	void                    disconnect();

private:
	int32_t                                         tick = 0;
	std::atomic_int32_t                             request_id = 0;
	std::optional<ecsact_registry_id>               registry_id;
	std::map<int32_t, types::cpp_execution_options> tick_map;
	std::map<int32_t, types::cpp_execution_options> temp_tick_map;

	ecsact_execute_systems_error sys_error = ECSACT_EXEC_SYS_OK;
	types::async_error           async_error{.error = ECSACT_ASYNC_OK};

	std::thread      execution_thread;
	std::mutex       execute_m;
	std::atomic_bool is_connected = false;

	static std::vector<types::callback_info> init_callbacks_info;
	static std::vector<types::callback_info> update_callbacks_info;
	static std::vector<types::callback_info> remove_callbacks_info;

	static void init_callback(
		ecsact_event        event,
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data,
		void*               callback_user_data
	);

	static void update_callback(
		ecsact_event        event,
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data,
		void*               callback_user_data
	);

	static void remove_callback(
		ecsact_event        event,
		ecsact_entity_id    entity_id,
		ecsact_component_id component_id,
		const void*         component_data,
		void*               callback_user_data
	);

	void merge_tick_options(
		types::cpp_execution_options& tick_options,
		types::cpp_execution_options& other_options
	);

	ecsact_async_request_id increment_request_id() {
		return static_cast<ecsact_async_request_id>(
			request_id.fetch_add(1, std::memory_order_relaxed)
		);
	}
};
