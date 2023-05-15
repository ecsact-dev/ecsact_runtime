#pragma once

#include <cstddef>
#include <span>
#include <vector>
#include <type_traits>
#include "ecsact/runtime/serialize.h"
#include "ecsact/runtime/core.h"

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
 * Serializes an ecsact_component when the type is unknown. Size and serialize
 * fn are passed in to work around various linking configurations.
 * @returns serialized action bytes
 */
inline std::vector<std::byte> serialize(
	const ecsact_component&                   component,
	decltype(ecsact_serialize_component_size) size_fn,
	decltype(ecsact_serialize_component)      serialize_fn
) {
	std::vector<std::byte> out_component;
	out_component.resize(size_fn(component.component_id));

	serialize_fn(
		component.component_id,
		component.component_data,
		reinterpret_cast<uint8_t*>(out_component.data())
	);
	return out_component;
}

/**
 * Serializes an ecsact_component when the type is unknown.
 * @returns serialized action bytes
 */
inline std::vector<std::byte> serialize(const ecsact_component& component) {
	return serialize(
		component,
		ecsact_serialize_component_size,
		ecsact_serialize_component
	);
}

/**
 * Serializes an ecsact_action when the type is unknown. Size and serialize
 * fn are passed in to work around various linking configurations.
 * @returns serialized component bytes
 */
inline std::vector<std::byte> serialize(
	const ecsact_action&                   action,
	decltype(ecsact_serialize_action_size) size_fn,
	decltype(ecsact_serialize_action)      serialize_fn
) {
	std::vector<std::byte> out_action;
	out_action.resize(size_fn(action.action_id));

	serialize_fn(
		action.action_id,
		action.action_data,
		reinterpret_cast<uint8_t*>(out_action.data())
	);
	return out_action;
}

/**
 * Serializes an ecsact_action when the type is unknown.
 * @returns serialized component bytes
 */
inline std::vector<std::byte> serialize(const ecsact_action& action) {
	return serialize(
		action,
		ecsact_serialize_action_size,
		ecsact_serialize_action
	);
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

/**
 * Deserializes an ecsact_component when the type is unknown. The deserialize
 * function is passed in to work around various linker configurations.
 * @returns an ecsact_action
 */
inline std::vector<std::byte> deserialize(
	const ecsact_action_id&             id,
	std::vector<std::byte>&             serialized_action,
	decltype(ecsact_deserialize_action) deserialize_fn
) {
	std::vector<std::byte> action_data;
	action_data.resize(serialized_action.size());

	deserialize_fn(
		id,
		reinterpret_cast<uint8_t*>(serialized_action.data()),
		action_data.data()
	);
	return action_data;
}

/**
 * Deserializes an ecsact_component when the type is unknown.
 * @returns an ecsact_action
 */
inline std::vector<std::byte> deserialize(
	const ecsact_action_id& id,
	std::vector<std::byte>& serialized_action
) {
	return deserialize(id, serialized_action, &ecsact_deserialize_action);
}

/**
 * Deserializes an ecsact_component when the type is unknown.
 * @returns an ecsact_action
 */
inline std::vector<std::byte> deserialize(
	const ecsact_component_id&             id,
	std::vector<std::byte>&                serialized_component,
	decltype(ecsact_deserialize_component) deserialize_fn
) {
	std::vector<std::byte> component_data;
	component_data.resize(serialized_component.size());

	deserialize_fn(
		id,
		reinterpret_cast<uint8_t*>(serialized_component.data()),
		component_data.data()
	);
	return component_data;
}

/**
 * Deserializes an ecsact_component when the type is unknown. The deserialize
 * function is passed in to work around various linker configurations.
 * @returns an ecsact_component_id
 */
inline std::vector<std::byte> deserialize(
	const ecsact_component_id& id,
	std::vector<std::byte>&    serialized_component
) {
	return deserialize(id, serialized_component, ecsact_deserialize_component);
}

} // namespace ecsact
