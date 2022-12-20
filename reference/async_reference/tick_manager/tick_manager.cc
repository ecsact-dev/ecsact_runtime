#include <ranges>
#include <algorithm>

#include "tick_manager.hh"

void tick_manager::add_pending_options(types::pending_execution_options& pending
) {
	std::unique_lock lk(tick_m);
	pending_tick_map[tick].emplace_back(pending);
}

std::optional<types::cpp_execution_options>
tick_manager::move_and_increment_tick() {
	std::optional<types::cpp_execution_options> cpp_options;

	if(validated_tick_map.contains(tick)) {
		std::unique_lock lk(tick_m);
		cpp_options = std::move(validated_tick_map.at(tick));
		validated_tick_map.erase(tick);
		tick++;
		return cpp_options;
	}
	tick++;
	return std::nullopt;
}

types::async_error tick_manager::validate_pending_options() {
	if(pending_tick_map.size() > 0) {
		std::unique_lock lk(tick_m);

		auto pending_options = std::move(pending_tick_map);
		pending_tick_map.clear();
		lk.unlock();

		for(auto& [key, pending_list] : pending_options) {
			types::cpp_execution_options merged_options{};

			for(int i = 0; i < pending_list.size(); i++) {
				merged_options = {};

				auto pending = pending_list[i];

				auto error = util::validate_options(pending.options);

				if(error != ECSACT_ASYNC_OK) {
					return types::async_error{
						.error = error,
						.request_ids = {pending.request_id},
					};
				}

				error = util::validate_merge_options(merged_options, pending.options);

				if(error != ECSACT_ASYNC_OK) {
					return types::async_error{
						.error = error,
						.request_ids = {pending.request_id},
					};
				}

				util::merge_options(merged_options, pending.options);

				lk.lock();
				error = upsert_validated_tick(merged_options, key);
				lk.unlock();

				if(error != ECSACT_ASYNC_OK) {
					auto requests = util::get_request_ids_from_pending_exec_options(
						pending_options.at(tick)
					);

					std::vector<ecsact_async_request_id> request_ids;
					request_ids
						.insert(request_ids.end(), requests.begin(), requests.end());

					return types::async_error{
						.error = error,
						.request_ids = request_ids,
					};
				}
			}
		}

		return types::async_error{
			.error = ECSACT_ASYNC_OK,
			.request_ids = {},
		};
	}
	return types::async_error{
		.error = ECSACT_ASYNC_OK,
		.request_ids = {},
	};
}

ecsact_async_error tick_manager::upsert_validated_tick(
	types::cpp_execution_options& validated_options,
	int32_t                       requested_tick
) {
	if(validated_tick_map.contains(requested_tick)) {
		auto existing_options = validated_tick_map[requested_tick];
		auto error =
			util::validate_merge_options(existing_options, validated_options);
		if(error != ECSACT_ASYNC_OK) {
			return error;
		}
	} else {
		validated_tick_map.insert(std::pair(requested_tick, validated_options));
	}

	return ECSACT_ASYNC_OK;
}
