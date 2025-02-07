#include <iostream>
#include <format>
#include "ecsact/runtime/common.h"

using test_formatter =
	decltype(std::declval<std::formatter<ecsact_registry_id>>());

auto main() -> int {
	auto id = ecsact_registry_id{};
	std::cout << std::format("the id is {}\n", id);
	std::cout << std::format("invalid id is {}\n", ECSACT_INVALID_ID(registry));
	return 0;
}
