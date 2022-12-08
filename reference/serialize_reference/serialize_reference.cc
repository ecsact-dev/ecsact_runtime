#include <map>
#include <optional>
#include "ecsact/runtime/common.h"
#include "ecsact/runtime/serialize.h"

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
