#include <ranges>
#include <algorithm>

#include "tick_manager.hh"

void tick_manager::add_pending_options(types::pending_execution_options& pending
) {
	std::unique_lock lk(tick_m);
	pending_tick_map[tick].emplace_back(pending);
}

std::optional<types::cpp_execution_options> tick_manager::get_options_now() {
	std::optional<types::cpp_execution_options> cpp_options;

	if(validated_tick_map.contains(tick)) {
		std::unique_lock lk(tick_m);
		cpp_options = std::move(validated_tick_map.at(tick));
		validated_tick_map.erase(tick);
		tick++;
		return cpp_options;
	}
	return std::nullopt;
}

types::async_error tick_manager::validate_pending_options() {
	if(pending_tick_map.size() > 0) {
		std::unique_lock lk(tick_m);

		auto pending_options = std::move(pending_tick_map);
		pending_tick_map.clear();
		lk.unlock();

		auto  first_pending = pending_options.begin();
		auto& initial_options = first_pending->second.begin()->options;

		auto error = util::validate_options(initial_options);

		if(error != ECSACT_ASYNC_OK) {
			auto& req_id = first_pending->second.begin()->request_id;

			return types::async_error{
				.error = error,
				.request_ids = {req_id},
			};
		}

		if(pending_tick_map.size() > 1) {
			for(auto& [key, pending_list] : pending_options) {
				for(int i = 1; i < pending_list.size(); i++) {
					auto pending = pending_list[i];
					auto error = util::validate_options(pending.options);

					// A bit redundant, but distinctly different
					if(error != ECSACT_ASYNC_OK) {
						return types::async_error{
							.error = error,
							.request_ids = {pending.request_id},
						};
					}

					error =
						util::validate_merge_options(initial_options, pending.options);

					if(error != ECSACT_ASYNC_OK) {
						return types::async_error{
							.error = error,
							.request_ids = {pending.request_id},
						};
					}

					util::merge_options(initial_options, pending.options);
				}
			}
		}

		lk.lock();
		error = upsert_validated_tick(initial_options);
		lk.unlock();

		if(error != ECSACT_ASYNC_OK) {
			std::vector<ecsact_async_request_id> request_ids;
			// Add all request ids from pending_options
			return types::async_error{
				.error = error,
				.request_ids = request_ids,
			};
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
	types::cpp_execution_options& validated_options
) {
	if(validated_tick_map.contains(tick)) {
		auto existing_options = validated_tick_map[tick];
		auto error =
			util::validate_merge_options(existing_options, validated_options);
		if(error != ECSACT_ASYNC_OK) {
			return error;
		}
	} else {
		validated_tick_map.insert(std::pair(tick, validated_options));
	}

	return ECSACT_ASYNC_OK;
}
