#include <map>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include <optional>

#include "ecsact/runtime/core.hh"
#include "ecsact/runtime/async.h"

struct callback_info {
	ecsact_event        event;
	ecsact_entity_id    entity_id;
	ecsact_component_id component_id;
};

struct async_error {
	ecsact_async_error error = ECSACT_ASYNC_OK;
	int32_t            request_id;
};

struct cpp_execution_options {
	std::optional<std::vector<ecsact_entity_id>>    add_components_entities;
	std::optional<std::vector<ecsact_component>>    add_components;
	std::optional<std::vector<ecsact_entity_id>>    update_components_entities;
	std::optional<std::vector<ecsact_component>>    update_components;
	std::optional<std::vector<ecsact_entity_id>>    remove_components_entities;
	std::optional<std::vector<ecsact_component_id>> remove_components;
	std::optional<std::vector<ecsact_action>>       actions;
};

class async_reference {
public:
	ecsact_async_request_id enqueue_execution_options(
		const ecsact_execution_options options
	);

	void execute_systems();

	void flush_events(
		const ecsact_execution_events_collector* execution_events,
		const ecsact_async_events_collector*     async_events
	);

	ecsact_async_request_id connect(const char* connection_string);
	void                    disconnect();

private:
	int32_t             tick = 0;
	std::atomic_int32_t request_id = 0;
	ecsact_registry_id  registry_id =
		ecsact_create_registry("async_reference_impl_reg");
	std::map<int32_t, cpp_execution_options> tick_map;
	std::map<int32_t, cpp_execution_options> temp_tick_map;

	ecsact_execute_systems_error sys_error = ECSACT_EXEC_SYS_OK;
	async_error                  async_error;

	std::thread      execution_thread;
	std::mutex       execute_m;
	std::atomic_bool is_connected = false;

	static std::vector<callback_info> init_callbacks_info;
	static std::vector<callback_info> update_callbacks_info;
	static std::vector<callback_info> remove_callbacks_info;

	// Still don't understand
	// Someone sends a `now` action tick
	// The execute systems is locked, this means the action they sent is already
	// behind. What purpose is the temp map supposed to serve if it's already
	// off by 1 tick? The only thing we can do is throw
	// This only has value if the options is ahead of the current tick
	// And if it is now, how am I supposed to check it?

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
		cpp_execution_options& tick_options,
		cpp_execution_options& other_options
	);

	cpp_execution_options c_to_cpp_execution_options(
		const ecsact_execution_options options
	);

	ecsact_execution_options* cpp_to_c_execution_options(
		cpp_execution_options options
	);
};
