#include <map>
#include <optional>
#include <type_traits>
#include <set>
#include <vector>
#include "ecsact/runtime/core.hh"
#include "ecsact/runtime/common.h"
#include "ecsact/runtime/serialize.hh"

namespace {
std::map<ecsact_component_id, int>* _ref_serialize_component_sizes;
std::map<ecsact_action_id, int>*    _ref_serialize_action_sizes;
} // namespace

namespace ecsact_serialize_ref::detail {

std::map<ecsact_component_id, int>& component_sizes() {
	if(!_ref_serialize_component_sizes) {
		_ref_serialize_component_sizes = new std::map<ecsact_component_id, int>();
	}
	return *_ref_serialize_component_sizes;
}

std::map<ecsact_action_id, int>& action_sizes() {
	if(!_ref_serialize_action_sizes) {
		_ref_serialize_action_sizes = new std::map<ecsact_action_id, int>();
	}
	return *_ref_serialize_action_sizes;
}

} // namespace ecsact_serialize_ref::detail

int ecsact_serialize_component_size(ecsact_component_id comp_id) {
	return _ref_serialize_component_sizes->at(comp_id);
}

int ecsact_serialize_action_size(ecsact_action_id action_id) {
	return _ref_serialize_action_sizes->at(action_id);
}

int ecsact_serialize_action(
	ecsact_action_id action_id,
	const void*      in_action_data,
	uint8_t*         out_bytes
) {
	auto size = _ref_serialize_action_sizes->at(action_id);
	memcpy(out_bytes, in_action_data, size);
	return size;
}

int ecsact_deserialize_action(
	ecsact_action_id action_id,
	const uint8_t*   in_bytes,
	void*            out_action_data
) {
	auto size = _ref_serialize_action_sizes->at(action_id);
	memcpy(out_action_data, in_bytes, size);
	return size;
}

int ecsact_serialize_component(
	ecsact_component_id component_id,
	const void*         in_component_data,
	uint8_t*            out_bytes
) {
	auto size = _ref_serialize_component_sizes->at(component_id);
	memcpy(out_bytes, in_component_data, size);
	return size;
}

int ecsact_deserialize_component(
	ecsact_component_id component_id,
	const uint8_t*      in_bytes,
	void*               out_component_data
) {
	auto size = _ref_serialize_component_sizes->at(component_id);
	memcpy(out_component_data, in_bytes, size);
	return size;
}

#if defined(ECSACT_CORE_API) || defined(ECSACT_CORE_API_LOAD_AT_RUNTIME)

void ecsact_dump_entities(
	ecsact_registry_id            registry_id,
	ecsact_dump_entities_callback callback,
	void*                         callback_user_data
) {
	const auto registry = ecsact::core::registry(registry_id);
	const auto entities = registry.get_entities();
	const auto entities_count = static_cast<int32_t>(entities.size());

	callback(&entities_count, sizeof(entities_count), callback_user_data);

	for(auto entity : entities) {
		const auto components_count = registry.count_components(entity);
		callback(&components_count, sizeof(components_count), callback_user_data);

		auto component_ids = std::vector<ecsact_component_id>{};
		auto component_datas = std::vector<const void*>{};

		component_ids.resize(components_count);
		component_datas.resize(components_count);

		ecsact_get_components(
			registry_id,
			entity,
			components_count,
			component_ids.data(),
			component_datas.data(),
			nullptr
		);

		for(int i = 0; components_count > i; ++i) {
			const auto component_id = component_ids[i];
			const auto component_data = component_datas[i];
			const auto component_size = ecsact_serialize_component_size(component_id);

			auto serialized_component_data =
				ecsact::serialize(ecsact_component{component_id, component_data});

			callback(&component_id, sizeof(component_id), callback_user_data);
			callback(&component_size, sizeof(component_size), callback_user_data);
			callback(
				serialized_component_data.data(),
				static_cast<int32_t>(serialized_component_data.size()),
				callback_user_data
			);
		}
	}
}

ecsact_restore_error ecsact_restore_entities(
	ecsact_registry_id                       registry_id,
	ecsact_restore_entities_callback         callback,
	const ecsact_execution_events_collector* events_collector,
	void*                                    callback_user_data
) {
	using comp_pair = std::pair<ecsact_entity_id, ecsact_component_id>;

	auto new_entities = std::vector<ecsact_entity_id>();
	auto destroyed_entities = std::vector<ecsact_entity_id>();
	auto new_components = std::vector<comp_pair>{};
	auto updated_components = std::vector<comp_pair>{};
	auto removed_components = std::vector<comp_pair>{};
	auto registry = ecsact::core::registry(registry_id);
	auto entities = registry.get_entities();
	auto read_amount = int32_t{};

#	define READ_CALLBACK_INTO_VAR(v)                                                \
		static_assert(std::is_integral_v<decltype(v)> || std::is_enum_v<decltype(v)>); \
		read_amount = callback(&v, sizeof(v), callback_user_data);                     \
		if(read_amount == 0) return ECSACT_RESTORE_ERR_UNEXPECTED_END;                 \
		if(read_amount != sizeof(v))                                                   \
		return ECSACT_RESTORE_ERR_UNEXPECTED_READ_LENGTH

	auto ent_count = int32_t{};
	READ_CALLBACK_INTO_VAR(ent_count);

	if(entities.size() > ent_count) {
		for(auto i = ent_count; entities.size() > i; ++i) {
			auto comp_ids = std::vector<ecsact_component_id>{};
			ecsact_each_component(
				registry_id,
				entities[i],
				[](ecsact_component_id comp_id, const void*, void* ud) {
					static_cast<decltype(&comp_ids)>(ud)->push_back(comp_id);
				},
				&comp_ids
			);

			destroyed_entities.push_back(entities[i]);
			for(auto comp_id : comp_ids) {
				removed_components.push_back({entities[i], comp_id});
			}
		}
	} else if(entities.size() < ent_count) {
		auto entities_original_size = entities.size();
		entities.resize(ent_count);
		for(auto i = entities_original_size; ent_count > i; ++i) {
			entities[i] = registry.create_entity();
			new_entities.push_back(entities[i]);
		}
	}

	for(int32_t ent_idx = 0; ent_count > ent_idx; ++ent_idx) {
		auto entity_id = entities[ent_idx];

		auto existing_comp_ids = std::set<ecsact_component_id>{};
		ecsact_each_component(
			registry_id,
			entity_id,
			[](ecsact_component_id comp_id, const void*, void* ud) {
				static_cast<decltype(&existing_comp_ids)>(ud)->insert(comp_id);
			},
			&existing_comp_ids
		);

		auto components_count = int32_t{};
		READ_CALLBACK_INTO_VAR(components_count);

		for(int i = 0; components_count > i; ++i) {
			auto component_id = ecsact_component_id{};
			auto component_size = int32_t{};
			READ_CALLBACK_INTO_VAR(component_id);
			READ_CALLBACK_INTO_VAR(component_size);

			if(component_size != ecsact_serialize_component_size(component_id)) {
				return ECSACT_RESTORE_ERR_INVALID_FORMAT;
			}

			auto serialized_component_data = std::vector<std::byte>{};
			if(component_size != 0) {
				serialized_component_data.resize(component_size);
				read_amount = callback(
					serialized_component_data.data(),
					static_cast<int32_t>(serialized_component_data.size()),
					callback_user_data
				);

				if(read_amount == 0) {
					return ECSACT_RESTORE_ERR_UNEXPECTED_END;
				}

				if(read_amount != serialized_component_data.size()) {
					return ECSACT_RESTORE_ERR_UNEXPECTED_READ_LENGTH;
				}
			}

			auto component_data =
				ecsact::deserialize(component_id, serialized_component_data);

			if(ecsact_has_component(registry_id, entity_id, component_id)) {
				if(component_size != 0) {
					ecsact_update_component(
						registry_id,
						entity_id,
						component_id,
						component_data.data()
					);
				}
				updated_components.push_back({entity_id, component_id});
			} else {
				ecsact_add_component(
					registry_id,
					entity_id,
					component_id,
					component_data.data()
				);
				new_components.push_back({entity_id, component_id});
			}

			existing_comp_ids.erase(component_id);
		}

		for(auto comp_id : existing_comp_ids) {
			removed_components.push_back({entity_id, comp_id});
		}
	}

	if(events_collector != nullptr) {
		if(events_collector->entity_created_callback != nullptr) {
			for(auto entity : new_entities) {
				events_collector->entity_created_callback(
					ECSACT_EVENT_CREATE_ENTITY,
					entity,
					{}, // no placeholder
					events_collector->entity_created_callback_user_data
				);
			}
		}

		if(events_collector->init_callback != nullptr) {
			for(auto&& [entity, comp_id] : new_components) {
				events_collector->init_callback(
					ECSACT_EVENT_INIT_COMPONENT,
					entity,
					comp_id,
					ecsact_get_component(registry_id, entity, comp_id),
					events_collector->init_callback_user_data
				);
			}
		}

		if(events_collector->update_callback != nullptr) {
			for(auto&& [entity, comp_id] : updated_components) {
				events_collector->update_callback(
					ECSACT_EVENT_UPDATE_COMPONENT,
					entity,
					comp_id,
					ecsact_get_component(registry_id, entity, comp_id),
					events_collector->update_callback_user_data
				);
			}
		}

		if(events_collector->remove_callback != nullptr) {
			for(auto&& [entity, comp_id] : removed_components) {
				events_collector->remove_callback(
					ECSACT_EVENT_REMOVE_COMPONENT,
					entity,
					comp_id,
					ecsact_get_component(registry_id, entity, comp_id),
					events_collector->remove_callback_user_data
				);

				ecsact_remove_component(registry_id, entity, comp_id);
			}
		}

		if(events_collector->entity_destroyed_callback != nullptr) {
			for(auto entity : destroyed_entities) {
				events_collector->entity_destroyed_callback(
					ECSACT_EVENT_DESTROY_ENTITY,
					entity,
					{},
					events_collector->entity_destroyed_callback_user_data
				);

				ecsact_destroy_entity(registry_id, entity);
			}
		}
	}

#	undef READ_CALLBACK_INTO_VAR

	return ECSACT_RESTORE_OK;
}

#endif // defined(ECSACT_CORE_API) || defined(ECSACT_CORE_API_LOAD_AT_RUNTIME)
