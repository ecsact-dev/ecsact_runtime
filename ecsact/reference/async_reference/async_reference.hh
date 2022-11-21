#include <map>
#include <vector>
#include <string>
#include <mutex>
#include <thread>

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
	int32_t            tick = 0;
	int32_t            request_id = 0;
	ecsact_registry_id registry_id =
		ecsact_create_registry("async_reference_impl_reg");
	std::map<int32_t, ecsact_execution_options> tick_map;

	ecsact_execute_systems_error sys_error = ECSACT_EXEC_SYS_OK;
	async_error                  async_error;

	std::thread execution_thread;
	std::mutex  execute_m;

	static std::vector<callback_info> init_callbacks_info;
	static std::vector<callback_info> update_callbacks_info;
	static std::vector<callback_info> remove_callbacks_info;

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

	// Store these data changes and send them on a flush event.

	// NOTE:
	//  If init and remove occurs and state doesn't change, delete the init data
	//  and don't send it to the user
};
