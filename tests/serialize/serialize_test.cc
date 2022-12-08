#include "gtest/gtest.h"
#include <vector>
#include "ecsact/runtime/serialize.hh"
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

TEST(ComponentSerialize, Cpp) {
	using serialize_test::ExampleComponent;
	auto serialized_comp = ecsact::serialize(ExampleComponent{.num = 42});
	auto comp = ecsact::deserialize<ExampleComponent>(serialized_comp);
	EXPECT_EQ(comp.num, 42);
}

TEST(ActionSerialize, Cpp) {
	using serialize_test::ExampleAction;
	auto serialized_act = ecsact::serialize(ExampleAction{.input = 42});
	auto act = ecsact::deserialize<ExampleAction>(serialized_act);
	EXPECT_EQ(act.input, 42);
}
