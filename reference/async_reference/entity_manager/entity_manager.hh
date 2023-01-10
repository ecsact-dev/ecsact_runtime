#pragma once

#include <vector>
#include <mutex>

#include "ecsact/runtime/core.hh"
#include "reference/async_reference/callbacks/async_callbacks.hh"
#include "reference/async_reference/util/types.hh"

class entity_manager {
public:
	void process_entities(
		async_callbacks&          callbacks,
		const ecsact_registry_id& registry_id
	);

	void request_entity(const ecsact_async_request_id& req_id);

private:
	std::mutex pending_m;

	std::vector<ecsact_async_request_id> pending_entity_requests;
};
