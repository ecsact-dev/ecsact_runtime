#include "async_callbacks.hh"

void async_callbacks::add(const types::async_requests type) {
	std::unique_lock lk(async_m);
	requests.push_back(type);
}

void async_callbacks::invoke(const ecsact_async_events_collector* async_events
) {
	if(async_events == nullptr) {
		std::unique_lock lk(async_m);
		requests.clear();
		return;
	}

	std::vector<types::async_requests> pending_requests;

	std::unique_lock lk(async_m);
	pending_requests = std::move(requests);
	requests.clear();
	lk.unlock();

	for(auto& request : pending_requests) {
		std::visit(
			[&async_events](auto&& error) {
				using T = std::decay_t<decltype(error)>;
				if constexpr(std::is_same_v<T, types::async_error>) {
					if(async_events->async_error_callback == nullptr) {
						return;
					}
					async_events->async_error_callback(
						error.error,
						error.request_ids.size(),
						error.request_ids.data(),
						async_events->async_error_callback_user_data
					);
				} else if constexpr(std::is_same_v<T, ecsact_execute_systems_error>) {
					if(async_events->system_error_callback == nullptr) {
						return;
					}
					async_events->system_error_callback(
						error,
						async_events->system_error_callback_user_data
					);
				} else if constexpr(std::is_same_v<T, types::entity>) {
					if(async_events->async_entity_callback == nullptr) {
						return;
					}
					async_events->async_entity_callback(
						*error.entity_id,
						error.request_id,
						async_events->async_entity_callback_user_data
					);
				}
			},
			request
		);
	}
}

void async_callbacks::add_many(const std::vector<types::async_requests>& types
) {
	std::unique_lock lk(async_m);
	requests.insert(requests.end(), types.begin(), types.end());
}
