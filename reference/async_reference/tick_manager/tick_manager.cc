#include <ranges>
#include <algorithm>

#include "tick_manager.hh"

ecsact_async_error tick_manager::try_add_options(
	types::cpp_execution_options& options
) {
	std::unique_lock lk(tick_m, std::try_to_lock);
	if(lk) {
		if(!tick_map.contains(tick)) {
			tick_map.insert(std::pair(tick, options));
		} else {
			ecsact_async_error error;

			auto existing_options = tick_map.at(tick);
			error = util::validate_merge_options(existing_options, options);
			if(error == ECSACT_ASYNC_OK) {
				util::merge_options(existing_options, options);
			} else {
				return error;
			}
		}
	} else {
		temp_tick_map.insert(std::pair(tick + 1, options));
		return ECSACT_ASYNC_OK;
	}
}

std::optional<types::cpp_execution_options> tick_manager::get_options_now() {
	std::optional<types::cpp_execution_options> cpp_options;

	// Consider pop and move
	std::unique_lock lk(tick_m);
	if(tick_map.contains(tick)) {
		cpp_options = tick_map.at(tick);
	}
	return std::nullopt;
}

void tick_manager::increment_and_merge_tick() {
	std::unique_lock lk(tick_m);
	tick_map.erase(tick);
	tick++;

	for(auto& [key, val] : temp_tick_map) {
		if(tick_map.contains(key)) {
			auto& tick_options = tick_map.at(key);
			auto& temp_options = temp_tick_map.at(key);
			util::merge_options(tick_options, temp_options);
			temp_tick_map.erase(key);
		} else {
			tick_map.insert(std::pair(key, val));
		}
	}
	lk.unlock();
}
