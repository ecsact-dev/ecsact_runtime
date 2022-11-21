#include "ecsact/runtime/async.h"
#include "async_reference.hh"

namespace async {
async_reference reference;
}

ecsact_async_request_id ecsact_async_connect(const char* connection_string) {
	return async::reference.connect(connection_string);
}

void ecsact_async_flush_events(
	const ecsact_execution_events_collector* execution_events,
	const ecsact_async_events_collector*     async_events
) {
	async::reference.flush_events(execution_events, async_events);
}
