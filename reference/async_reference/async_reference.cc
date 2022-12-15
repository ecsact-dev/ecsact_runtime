#include <span>
#include <memory>
#include <ranges>
#include <algorithm>
#include <iterator>

#include "async_reference.hh"

ecsact_async_request_id async_reference::connect(const char* connection_string
) {
	std::string connect_str(connection_string);

	registry_id = ecsact_create_registry("async_reference_impl_reg");
	pending_registry_id =
		ecsact_create_registry("pending_async_reference_impl_reg");

	auto req_id = next_request_id();
	// The good and bad strings simulate the outcome of connections
	if(connect_str == "good") {
		is_connected = true;

		execute_systems();
	} else {
		// Same thing that happens in enqueue? Callback next flush?
		is_connected = false;
		is_connected_notified = false;
	}

	return req_id;
}

ecsact_async_request_id async_reference::enqueue_execution_options(
	const ecsact_execution_options& options
) {
	auto req_id = next_request_id();

	if(is_connected == false && is_connected_notified == false) {
		types::async_error async_err{
			.error = ECSACT_ASYNC_ERR_PERMISSION_DENIED,
			.request_ids = {req_id},
		};

		is_connected_notified = true;
		// Could block here
		async_callbacks.add(async_err);
		return req_id;
	}

	auto cpp_options =
		util::c_to_cpp_execution_options(options, *pending_registry_id);

	types::pending_execution_options pending_options{
		.request_id = req_id,
		.options = cpp_options,
	};

	// Could block here
	tick_manager.add_pending_options(pending_options);
	return req_id;
}

void async_reference::execute_systems() {
	execution_thread = std::thread([this] {
		while(is_connected == true) {
			// Could block here
			auto async_err = tick_manager.validate_pending_options();

			if(async_err.error != ECSACT_ASYNC_OK) {
				async_callbacks.add(async_err);

				disconnect();
			}

			auto cpp_options = tick_manager.get_options_now();

			ecsact_execution_events_collector collector;
			collector.init_callback = &execution_callbacks::init_callback;
			collector.update_callback = &execution_callbacks::update_callback;
			collector.remove_callback = &execution_callbacks::remove_callback;

			collector.init_callback_user_data = nullptr;
			collector.update_callback_user_data = nullptr;
			collector.remove_callback_user_data = nullptr;

			std::unique_ptr<ecsact_execution_options> options = nullptr;

			if(cpp_options) {
				options = std::make_unique<ecsact_execution_options>(
					util::cpp_to_c_execution_options(
						*cpp_options,
						*registry_id,
						*pending_registry_id
					)
				);
			}

			std::vector<ecsact_async_request_id> pending_entities;

			// Could block here
			std::unique_lock lk(pending_m);
			pending_entities = std::move(pending_entity_requests);
			pending_entity_requests.clear();
			lk.unlock();

			for(auto& entity_request_id : pending_entities) {
				auto entity = ecsact_create_entity(*registry_id);
				auto pended_entity = ecsact_create_entity(*pending_registry_id);

				types::entity created_entity{
					.entity_id = entity,
					.request_id = entity_request_id,
				};
				// Could block here
				async_callbacks.add(created_entity);
			}

			auto systems_error =
				ecsact_execute_systems(*registry_id, 1, options.get(), &collector);

			if(systems_error != ECSACT_EXEC_SYS_OK) {
				async_callbacks.add(systems_error);
				disconnect();
				return;
			}
		}
	});
}

void async_reference::flush_events(
	const ecsact_execution_events_collector* execution_events,
	const ecsact_async_events_collector*     async_events
) {
	async_callbacks.invoke(async_events);
	if(registry_id) {
		exec_callbacks.invoke(execution_events, *registry_id);
	}
}

ecsact_async_request_id async_reference::create_entity_request() {
	// NOTE: Add entity to both registries
	// Consider ensure entity
	auto req_id = next_request_id();
	if(is_connected == false && is_connected_notified == false) {
		types::async_error async_err{
			.error = ECSACT_ASYNC_ERR_PERMISSION_DENIED,
			.request_ids = {req_id},
		};

		async_callbacks.add(async_err);
		is_connected_notified = true;

		return req_id;
	}

	std::unique_lock lk(pending_m);
	pending_entity_requests.insert(pending_entity_requests.end(), req_id);
	return req_id;
}

void async_reference::disconnect() {
	is_connected = false;
	if(execution_thread.joinable()) {
		execution_thread.join();
	}
}

ecsact_async_request_id async_reference::next_request_id() {
	return static_cast<ecsact_async_request_id>(
		_last_request_id.fetch_add(1, std::memory_order_relaxed)
	);
}

ecsact_async_request_id async_reference::convert_request_id(int32_t id) {
	return static_cast<ecsact_async_request_id>(id);
}
