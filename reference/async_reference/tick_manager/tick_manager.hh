#pragma once

#include <cstdint>
#include <map>
#include <mutex>
#include <optional>

#include "reference/async_reference/util/types.hh"
#include "reference/async_reference/util/util.hh"

class tick_manager {
public:
	ecsact_async_error try_add_options(types::cpp_execution_options& options);
	std::optional<types::cpp_execution_options> get_options_now();

	void increment_and_merge_tick();

private:
	int32_t    tick = 0;
	std::mutex tick_m;

	std::map<int32_t, types::cpp_execution_options> tick_map;
	std::map<int32_t, types::cpp_execution_options> temp_tick_map;
};
