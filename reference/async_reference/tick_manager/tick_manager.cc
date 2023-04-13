#include <ranges>
#include <algorithm>

#include "tick_manager.hh"

using namespace ecsact::async_reference::detail;

void tick_manager::add_pending_options(
	const types::pending_execution_options& pending
) {
	std::unique_lock lk(tick_m);
	pending_options.emplace_back(pending);
}

auto tick_manager::move_and_increment_tick()
	-> std::optional<types::cpp_execution_options> {
	auto cpp_options = std::optional<types::cpp_execution_options>{};

	if(validated_options.has_value()) {
		cpp_options = std::move(validated_options);
		validated_options = {};
		tick++;
		return cpp_options;
	}
	tick++;
	return std::nullopt;
}

auto tick_manager::validate_pending_options()
	-> std::variant<types::async_error, types::async_request_complete> {
	if(pending_options.empty()) {
		return types::async_request_complete{};
	}

	auto current_pending_options =
		std::vector<types::pending_execution_options>{};

	if(pending_options.size() > 0) {
		auto lk = std::scoped_lock(tick_m);
		current_pending_options = std::move(pending_options);
		pending_options.clear();
	}

	auto requests_ids =
		util::get_request_ids_from_pending_exec_options(current_pending_options);

	for(auto& pending : current_pending_options) {
		auto async_error = upsert_validated_options(pending.options);

		if(async_error != ECSACT_ASYNC_OK) {
			auto error = types::async_error{
				.error = async_error,
				.request_ids = {},
			};

			error.request_ids.insert(
				error.request_ids.end(),
				requests_ids.begin(),
				requests_ids.end()
			);

			return error;
		}
	}

	auto req_complete = types::async_request_complete{};
	req_complete.request_ids.insert(
		req_complete.request_ids.end(),
		requests_ids.begin(),
		requests_ids.end()
	);

	return req_complete;
}

int32_t tick_manager::get_current_tick() {
	return tick;
}

ecsact_async_error tick_manager::upsert_validated_options(
	types::cpp_execution_options& new_options
) {
	auto error = util::validate_options(new_options);

	if(error != ECSACT_ASYNC_OK) {
		return error;
	}

	error = util::validate_merge_options(validated_options, new_options);
	if(error == ECSACT_ASYNC_OK) {
		util::merge_options(validated_options, new_options);
		return error;
	}
	return error;
}
