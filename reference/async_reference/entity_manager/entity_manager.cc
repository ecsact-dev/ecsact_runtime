#include "entity_manager.hh"

void entity_manager::process_entities(
	async_callbacks&          callbacks,
	const ecsact_registry_id& registry_id
) {
	std::vector<ecsact_async_request_id> pending_entities;

	std::unique_lock lk(pending_m);
	pending_entities = std::move(pending_entity_requests);
	pending_entity_requests.clear();
	lk.unlock();

	for(auto& entity_request_id : pending_entities) {
		auto entity = ecsact_create_entity(registry_id);

		types::entity created_entity{
			.entity_id = entity,
			.request_id = entity_request_id,
		};

		callbacks.add(created_entity);
	}
}

void entity_manager::request_entity(const ecsact_async_request_id& req_id) {
	std::unique_lock lk(pending_m);
	pending_entity_requests.push_back(req_id);
}
