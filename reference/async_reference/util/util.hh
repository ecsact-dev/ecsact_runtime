#pragma once

#include <algorithm>
#include <ranges>
#include <vector>
#include <concepts>

#include "types.hh"

#include "ecsact/runtime/core.h"
#include "ecsact/runtime/serialize.hh"
#include "reference/async_reference/detail/c_execution_options/c_execution_options.hh"

namespace util {

auto component_to_component_id_view(auto& components_view) {
	return std::ranges::views::transform(
		components_view,
		[](ecsact_component& component) { return component.component_id; }
	);
}

auto get_cpp_entities(auto& components_range) {
	return std::ranges::views::transform(
		components_range,
		[](auto cpp_component) { return cpp_component.entity_id; }
	);
}

auto get_cpp_component_ids(auto& components_range) {
	return std::ranges::views::transform(
		components_range,
		[](types::cpp_execution_component& cpp_component) {
			return cpp_component._id;
		}
	);
}

auto get_request_ids_from_pending_exec_options(auto& pending_exec_options) {
	return std::ranges::views::transform(
		pending_exec_options,
		[](auto& exec_options) { return exec_options.request_id; }
	);
}

bool check_entity_duplicates(auto entities_view) {
	for(auto entity_id : entities_view) {
		auto duplicate_view =
			std::views::filter(entities_view, [&entity_id](auto other_entity) {
				return entity_id == other_entity;
			});
		int same_entity_count = 0;
		for(auto _ : duplicate_view) {
			same_entity_count++;
			if(same_entity_count > 1) {
				return true;
			}
		}
	}
	return false;
}

bool check_entity_merge_duplicates(
	auto& entities_view,
	auto& other_entities_view
) {
	for(auto entity_id : entities_view) {
		auto duplicate_view =
			std::views::filter(other_entities_view, [&entity_id](auto other_entity) {
				return entity_id == other_entity;
			});

		int same_entity_count = 0;
		for(auto _ : duplicate_view) {
			same_entity_count++;
			if(same_entity_count > 0) {
				return true;
			}
		}
	}
	return false;
}

void cpp_to_c_execution_options(
	detail::c_execution_options&  out_c_execution_options,
	types::cpp_execution_options& options,
	const ecsact_registry_id&     registry_id
);

types::cpp_execution_options c_to_cpp_execution_options(
	const ecsact_execution_options& options
);

ecsact_async_error validate_options(types::cpp_execution_options& options);

ecsact_async_error validate_merge_options(
	const types::cpp_execution_options& options,
	const types::cpp_execution_options& other_options
);

void merge_options(
	types::cpp_execution_options&       tick_options,
	const types::cpp_execution_options& other_options
);

void merge_options(
	types::pending_execution_options&       pending,
	const types::pending_execution_options& other_pending
);

} // namespace util
