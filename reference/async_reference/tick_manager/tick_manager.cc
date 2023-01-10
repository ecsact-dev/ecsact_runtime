#include <ranges>
#include <algorithm>

#include "tick_manager.hh"

void tick_manager::add_pending_options(
	const types::pending_execution_options& pending
) {
	std::unique_lock lk(tick_m);
	pending_tick_map[tick].emplace_back(pending);
}

std::optional<types::cpp_execution_options>
tick_manager::move_and_increment_tick() {
	auto cpp_options = std::optional<types::cpp_execution_options>{};

	if(validated_tick_map.contains(tick)) {
		cpp_options = std::move(validated_tick_map.at(tick));
		validated_tick_map.erase(tick);
		tick++;
		return cpp_options;
	}
	tick++;
	return std::nullopt;
}

types::async_error tick_manager::validate_pending_options() {
	auto result = types::async_error{.error = ECSACT_ASYNC_OK, .request_ids = {}};

	if(pending_tick_map.empty()) {
		return result;
	}

	std::unique_lock lk(tick_m);

	auto pending_options = std::move(pending_tick_map);
	pending_tick_map.clear();
	lk.unlock();

	for(auto& [key, pending_list] : pending_options) {
		auto merged_options = types::cpp_execution_options{};

		for(int i = 0; i < pending_list.size(); i++) {
			merged_options = {};

			auto pending = pending_list[i];

			result.error = util::validate_options(pending.options);

			if(result.error != ECSACT_ASYNC_OK) {
				result.request_ids.push_back(pending.request_id);
				return result;
			}

			result.error = upsert_validated_tick(pending.options, key);

			if(result.error != ECSACT_ASYNC_OK) {
				auto requests = util::get_request_ids_from_pending_exec_options(
					pending_options.at(tick)
				);

				result.request_ids
					.insert(result.request_ids.end(), requests.begin(), requests.end());

				return result;
			}
		}
	}
	return result;
}

ecsact_async_error tick_manager::upsert_validated_tick(
	types::cpp_execution_options& validated_options,
	int32_t                       requested_tick
) {
	if(validated_tick_map.contains(requested_tick)) {
		auto& existing_options = validated_tick_map[requested_tick];
		auto  error =
			util::validate_merge_options(existing_options, validated_options);
		if(error == ECSACT_ASYNC_OK) {
			util::merge_options(validated_options, existing_options);
			existing_options = validated_options;
			return error;
		}
		return error;
	} else {
		validated_tick_map.insert(std::pair(requested_tick, validated_options));
		return ECSACT_ASYNC_OK;
	}
}
