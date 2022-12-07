#pragma once

#include <algorithm>
#include <ranges>

#include "types.hh"

#include "ecsact/runtime/core.h"

namespace util {

auto component_to_component_id_view(auto components_view) {
	return std::ranges::views::transform(
		components_view,
		[](ecsact_component component) { return component.component_id; }
	);
}

auto get_cpp_entities(auto components_range) {
	return std::ranges::views::transform(
		components_range,
		[](auto cpp_component) { return cpp_component.entity_id; }
	);
}

auto get_cpp_component_ids(auto components_range) {
	return std::ranges::views::transform(
		components_range,
		[](types::cpp_execution_component cpp_component) {
			return cpp_component._id;
		}
	);
}

bool check_entity_duplicates(auto entities_view) {
	for(auto entity_id : entities_view) {
		auto view =
			std::views::filter(entities_view, [&entity_id](auto other_entity) {
				return entity_id == other_entity;
			});
		int same_entity_count = 0;
		for(auto itr = view.begin(); itr != view.end(); itr++) {
			same_entity_count++;
			if(same_entity_count > 1) {
				return true;
			}
		}
	}
	return false;
}

bool check_entity_merge_duplicates(
	auto entities_view,
	auto other_entities_view
) {
	for(auto entity_id : entities_view) {
		auto view =
			std::views::filter(other_entities_view, [&entity_id](auto other_entity) {
				return entity_id == other_entity;
			});

		int same_entity_count = 0;
		for(auto itr = view.begin(); itr != view.end(); itr++) {
			same_entity_count++;
			if(same_entity_count > 0) {
				return true;
			}
		}
	}
	return false;
}

ecsact_execution_options cpp_to_c_execution_options(
	types::cpp_execution_options options,
	const ecsact_registry_id&    registry_id
);

types::cpp_execution_options c_to_cpp_execution_options(
	const ecsact_execution_options options
);

ecsact_async_error validate_options(types::cpp_execution_options& options);

ecsact_async_error validate_merge_options(
	types::cpp_execution_options& options,
	types::cpp_execution_options& other_options
);

void merge_options(
	types::cpp_execution_options& tick_options,
	types::cpp_execution_options& other_options
);

} // namespace util
