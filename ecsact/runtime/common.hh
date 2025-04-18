#pragma once

#include <concepts>
#include <type_traits>
#include "ecsact/runtime/common.h"

namespace ecsact {

template<typename T>
concept component_like = requires(T) {
	{
		ecsact_id_cast<ecsact_component_like_id>(T::id)
	} -> std::convertible_to<ecsact_component_like_id>;
};

template<typename T>
concept component = requires(T) {
	{ T::id } -> std::convertible_to<ecsact_component_id>;
};

template<typename T>
concept tag_component = component<T> && std::is_empty_v<T>;

template<typename T>
concept tag_component_like = component_like<T> && std::is_empty_v<T>;

template<typename T>
concept non_tag_component = component<T> && !std::is_empty_v<T>;

template<typename T>
concept non_tag_component_like = component_like<T> && !std::is_empty_v<T>;

template<typename T>
concept non_tag_component_with_indexed_fields =
	non_tag_component<T> && !std::is_empty_v<typename T::IndexedFields>;

template<typename T>
concept non_tag_component_without_indexed_fields =
	non_tag_component<T> && std::is_empty_v<typename T::IndexedFields>;

template<typename T>
concept component_without_indexed_fields =
	component<T> && std::is_empty_v<typename T::IndexedFields>;

template<typename T>
concept component_with_indexed_fields =
	component<T> && std::is_empty_v<typename T::IndexedFields>;

} // namespace ecsact
