#pragma once

#include <cstdint>
#include <atomic>
#include "ecsact/runtime/async.h"

namespace ecsact::async_reference::detail {

class request_id_factory {
	std::atomic<std::int32_t> _last_request_id;

public:
	inline ecsact_async_request_id next_id() {
		return static_cast<ecsact_async_request_id>(
			_last_request_id.fetch_add(1, std::memory_order_relaxed)
		);
	}
};

} // namespace ecsact::async_reference::detail
