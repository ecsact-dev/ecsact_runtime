#include "gtest/gtest.h"
#include <vector>
#include "ecsact/runtime/serialize.h"
#include "serialize_test_generated/serialize_test.ecsact.hh"

TEST(Serialize, Correctness) {
	auto comp = serialize_test::ExampleComponent{};
	comp.num = 42;

	std::vector<uint8_t> serialized_comp(
		ecsact_serialize_component_size(serialize_test::ExampleComponent::id)
	);

	ecsact_serialize_component(
		serialize_test::ExampleComponent::id,
		&comp,
		serialized_comp.data()
	);

	auto deserialized_comp = serialize_test::ExampleComponent{};

	ecsact_deserialize_component(
		serialize_test::ExampleComponent::id,
		serialized_comp.data(),
		&deserialized_comp
	);

	EXPECT_EQ(deserialized_comp.num, comp.num);
}
