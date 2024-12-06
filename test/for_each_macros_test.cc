#include <iostream>
#include "gtest/gtest.h"
#include "ecsact/runtime.h"

TEST(ForEach, ForEachLambda) {
#define AS_PTR_VARIABLE(method_name, unused) \
	[[maybe_unused]] decltype(&method_name) method_name##_ptr = nullptr;

	FOR_EACH_ECSACT_API_FN(AS_PTR_VARIABLE, unused);

#undef AS_PTR_VARIABLE
}
