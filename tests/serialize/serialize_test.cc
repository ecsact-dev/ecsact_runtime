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

	// There are 3 overloads for deserialize.
	// (1) when you only care about the component and don't care about how much of
	// the span you've read from.
	auto comp = ecsact::deserialize<ExampleComponent>(serialized_comp);
	EXPECT_EQ(comp.num, 42);

	// (2) when you want the amount read from the span and you already have the
	// integer allocated
	int read_amount = 0;
	comp = ecsact::deserialize<ExampleComponent>(serialized_comp, read_amount);
	EXPECT_EQ(comp.num, 42);
	EXPECT_GT(read_amount, 0);

	// (3) when you want the amount read from the span and you already have the
	// component allocated. Also better for template deduction (notice no angle
	// brackets (<>)).
	read_amount = ecsact::deserialize(serialized_comp, comp);
	EXPECT_EQ(comp.num, 42);
	EXPECT_GT(read_amount, 0);
}

TEST(ActionSerialize, Cpp) {
	using serialize_test::ExampleAction;
	auto serialized_act = ecsact::serialize(ExampleAction{.input = 42});
	auto act = ecsact::deserialize<ExampleAction>(serialized_act);
	EXPECT_EQ(act.input, 42);
}
