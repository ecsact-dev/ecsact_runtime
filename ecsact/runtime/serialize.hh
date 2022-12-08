#pragma once

#include <cstddef>
#include <span>
#include <vector>
#include <type_traits>
#include "ecsact/runtime/serialize.h"

namespace ecsact {

template<typename T>
std::vector<std::byte> serialize(const T& component_or_action) {
	constexpr bool is_action =
		std::is_same_v<std::remove_cvref_t<decltype(T::id)>, ecsact_action_id>;
	constexpr bool is_component =
		std::is_same_v<std::remove_cvref_t<decltype(T::id)>, ecsact_component_id>;

	static_assert(
		is_action || is_component,
		"May only serialize components or actions"
	);

	std::vector<std::byte> bytes;

	if constexpr(is_action) {
		bytes.resize(ecsact_serialize_action_size(T::id));
		ecsact_serialize_action(
			T::id,
			&component_or_action,
			reinterpret_cast<uint8_t*>(bytes.data())
		);
	} else if constexpr(is_component) {
		bytes.resize(ecsact_serialize_component_size(T::id));
		ecsact_serialize_component(
			T::id,
			&component_or_action,
			reinterpret_cast<uint8_t*>(bytes.data())
		);
	}

	return bytes;
}

template<typename T>
T deserialize(std::span<std::byte> serialized_component_or_action) {
	constexpr bool is_action =
		std::is_same_v<std::remove_cvref_t<decltype(T::id)>, ecsact_action_id>;
	constexpr bool is_component =
		std::is_same_v<std::remove_cvref_t<decltype(T::id)>, ecsact_component_id>;

	static_assert(
		is_action || is_component,
		"May only deserialize components or actions"
	);

	T   result;
	int read_amount;

	if constexpr(is_action) {
		read_amount = ecsact_deserialize_action(
			T::id,
			reinterpret_cast<const uint8_t*>(serialized_component_or_action.data()),
			&result
		);
	} else if constexpr(is_component) {
		read_amount = ecsact_deserialize_component(
			T::id,
			reinterpret_cast<const uint8_t*>(serialized_component_or_action.data()),
			&result
		);
	}

	return result;
}

} // namespace ecsact
