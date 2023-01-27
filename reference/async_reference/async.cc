#include <optional>

#include "ecsact/runtime/async.h"
#include "async_reference.hh"

namespace async {
std::optional<async_reference> reference;
}

ecsact_async_request_id ecsact_async_connect(const char* connection_string) {
	async::reference.emplace();
	return async::reference->connect(connection_string);
}

void ecsact_async_disconnect() {
	if(async::reference) {
		async::reference->disconnect();
		async::reference.reset();
	}
}

void ecsact_async_flush_events(
	const ecsact_execution_events_collector* execution_events,
	const ecsact_async_events_collector*     async_events
) {
	if(async::reference) {
		async::reference->flush_events(execution_events, async_events);
	}
}

ecsact_async_request_id ecsact_async_create_entity() {
	if(async::reference) {
		return async::reference->create_entity_request();
	}
	return static_cast<ecsact_async_request_id>(15);
}

ecsact_async_request_id ecsact_async_enqueue_execution_options(
	const ecsact_execution_options options
) {
	if(async::reference) {
		return async::reference->enqueue_execution_options(options);
	}
	return {};
}

int32_t ecsact_async_get_current_tick() {
	if(async::reference) {
		return async::reference->get_current_tick();
	}
	return {};
}
