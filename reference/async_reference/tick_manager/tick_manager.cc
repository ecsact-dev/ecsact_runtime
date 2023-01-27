#include <ranges>
#include <algorithm>

#include "tick_manager.hh"

void tick_manager::add_pending_options(
	const types::pending_execution_options& pending
) {
	std::unique_lock lk(tick_m);
	pending_options.emplace_back(pending);
}

std::optional<types::cpp_execution_options>
tick_manager::move_and_increment_tick() {
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

types::async_error tick_manager::validate_pending_options() {
	auto result = types::async_error{.error = ECSACT_ASYNC_OK, .request_ids = {}};

	if(pending_options.empty()) {
		return result;
	}

	std::vector<types::pending_execution_options> current_pending_options;

	if(pending_options.size() > 0) {
		std::unique_lock lk(tick_m);
		current_pending_options = std::move(pending_options);
		pending_options.clear();
		lk.unlock();
	}

	for(auto& pending : current_pending_options) {
		result.error = upsert_validated_options(pending.options);

		if(result.error != ECSACT_ASYNC_OK) {
			auto requests =
				util::get_request_ids_from_pending_exec_options(current_pending_options
				);

			result.request_ids
				.insert(result.request_ids.end(), requests.begin(), requests.end());

			return result;
		}
	}
	return result;
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
