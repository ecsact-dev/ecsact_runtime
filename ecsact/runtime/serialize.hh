#pragma once

#include <cstddef>
#include <span>
#include <vector>
#include <type_traits>
#include "ecsact/runtime/serialize.h"

namespace ecsact {

/**
 * Calls `ecsact_serialize_action` or `ecsact_serialize_component` based on the
 * type of @p component_or_action.
 * @returns serialized action or component bytes
 */
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

/**
 * Calls `ecsact_deserialize_action` or `ecsact_deserialize_component` based on
 * the type of @tp T.
 * @returns the deserialized action or component
 */
template<typename T>
T deserialize(
	std::span<std::byte> serialized_component_or_action,
	int&                 out_read_amount
) {
	constexpr bool is_action =
		std::is_same_v<std::remove_cvref_t<decltype(T::id)>, ecsact_action_id>;
	constexpr bool is_component =
		std::is_same_v<std::remove_cvref_t<decltype(T::id)>, ecsact_component_id>;

	static_assert(
		is_action || is_component,
		"May only deserialize components or actions"
	);

	T result;

	if constexpr(is_action) {
		out_read_amount = ecsact_deserialize_action(
			T::id,
			reinterpret_cast<const uint8_t*>(serialized_component_or_action.data()),
			&result
		);
	} else if constexpr(is_component) {
		out_read_amount = ecsact_deserialize_component(
			T::id,
			reinterpret_cast<const uint8_t*>(serialized_component_or_action.data()),
			&result
		);
	}

	return result;
}

/**
 * Calls `ecsact_deserialize_action` or `ecsact_deserialize_component` based on
 * the type of @tp T.
 * @NOTE: the read amount is discard. See other overload to get read amount.
 * @returns the deserialized action or component
 */
template<typename T>
T deserialize(std::span<std::byte> serialized_component_or_action) {
	[[maybe_unused]] int discarded_read_amount;
	return ::ecsact::deserialize<T>(
		serialized_component_or_action,
		discarded_read_amount
	);
}

/**
 * Calls `ecsact_deserialize_action` or `ecsact_deserialize_component` based on
 * the type of @tp T.
 * @returns number of bytes read from the @p serialized_component_or_action
 */
template<typename T>
int deserialize(
	std::span<std::byte> serialized_component_or_action,
	T&                   out_component
) {
	int read_amount;
	out_component =
		::ecsact::deserialize<T>(serialized_component_or_action, read_amount);
	return read_amount;
}
} // namespace ecsact
