#include "async_callbacks.hh"

using namespace ecsact::async_reference::detail;

void async_callbacks::add(const types::async_requests type) {
	std::unique_lock lk(async_m);
	requests.push_back(type);
}

static auto _invoke(
	const ecsact_async_events_collector* async_evc,
	types::async_error                   req
) -> void {
	if(async_evc->async_error_callback == nullptr) {
		return;
	}

	async_evc->async_error_callback(
		req.error,
		req.request_ids.size(),
		req.request_ids.data(),
		async_evc->async_error_callback_user_data
	);
}

static auto _invoke(
	const ecsact_async_events_collector* async_evc,
	ecsact_execute_systems_error         err
) -> void {
	if(async_evc->system_error_callback == nullptr) {
		return;
	}

	async_evc->system_error_callback(
		err,
		async_evc->system_error_callback_user_data
	);
}

static auto _invoke(
	const ecsact_async_events_collector* async_evc,
	types::async_request_complete        req
) -> void {
	if(async_evc->async_request_done_callback == nullptr) {
		return;
	}

	async_evc->async_request_done_callback(
		req.request_ids.size(),
		req.request_ids.data(),
		async_evc->async_request_done_callback_user_data
	);
}

void async_callbacks::invoke(const ecsact_async_events_collector* async_evc) {
	if(requests.empty()) {
		return;
	}

	if(async_evc == nullptr) {
		auto lk = std::scoped_lock(async_m);
		requests.clear();
		return;
	}

	auto pending_requests = std::vector<types::async_requests>{};

	{
		auto lk = std::scoped_lock(async_m);
		pending_requests = std::move(requests);
		requests.clear();
	}

	for(auto& request : pending_requests) {
		std::visit(
			[async_evc](auto&& req) { _invoke(async_evc, std::move(req)); },
			request
		);
	}
}

void async_callbacks::add_many(const std::vector<types::async_requests>& types
) {
	std::unique_lock lk(async_m);
	requests.insert(requests.end(), types.begin(), types.end());
}
