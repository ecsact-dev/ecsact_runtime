#pragma once

#include <variant>
#include <map>
#include <mutex>

#include "reference/async_reference/util/types.hh"
#include "reference/async_reference/util/util.hh"

namespace ecsact::async_reference::detail {

class async_callbacks {
public:
	void invoke(const ecsact_async_events_collector* async_events);
	void add(const types::async_requests type);
	void add_many(const std::vector<types::async_requests>& types);

private:
	std::vector<types::async_requests> requests;

	std::mutex async_m;
};

} // namespace ecsact::async_reference::detail
