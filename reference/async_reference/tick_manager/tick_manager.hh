#pragma once

#include <cstdint>
#include <map>
#include <mutex>
#include <optional>

#include "reference/async_reference/util/types.hh"
#include "reference/async_reference/util/util.hh"

class tick_manager {
public:
	void add_pending_options(const types::pending_execution_options& options);
	std::optional<types::cpp_execution_options> move_and_increment_tick();

	types::async_error validate_pending_options();

private:
	int32_t    tick = 0;
	std::mutex tick_m;

	std::map<int32_t, types::cpp_execution_options> validated_tick_map;
	std::map<int32_t, std::vector<types::pending_execution_options>>
		pending_tick_map;

	ecsact_async_error upsert_validated_tick(
		types::cpp_execution_options& validated_options,
		int32_t                       requested_tick
	);
};
