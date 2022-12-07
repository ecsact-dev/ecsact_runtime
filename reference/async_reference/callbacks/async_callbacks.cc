#include "async_callbacks.hh"

void async_callbacks::add(const types::async_requests type) {
	std::unique_lock lk(async_m);
	requests.insert(requests.end(), type);
}

bool async_callbacks::invoke(const ecsact_async_events_collector& async_events
) {
	bool breaking_error = false;

	std::vector<types::async_requests> pending_requests;

	std::unique_lock lk(async_m);
	pending_requests = std::move(requests);
	requests.clear();
	lk.unlock();

	for(auto& request : pending_requests) {
		std::visit(
			[request, &async_events, &breaking_error](auto&& error) {
				using T = std::decay_t<decltype(error)>;
				if constexpr(std::is_same_v<T, types::async_error>) {
					async_events.async_error_callback(
						error.error,
						static_cast<ecsact_async_request_id>(error.request_id),
						async_events.async_error_callback_user_data
					);
					breaking_error = true;
				} else if constexpr(std::is_same_v<T, ecsact_execute_systems_error>) {
					async_events.system_error_callback(
						error,
						async_events.system_error_callback_user_data
					);
					breaking_error = true;
				} else if constexpr(std::is_same_v<T, types::entity>) {
					async_events.async_entity_callback(
						*error.entity_id,
						error.request_id,
						async_events.async_entity_error_callback_user_data
					);
				}
			},
			request
		);
	}
	return breaking_error;
}

void async_callbacks::add_many(const std::vector<types::async_requests>& types
) {
	std::unique_lock lk(async_m);
	requests.insert(requests.end(), types.begin(), types.end());
}
