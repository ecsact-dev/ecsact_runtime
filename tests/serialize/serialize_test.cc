#include "gtest/gtest.h"
#include <vector>
#include <deque>
#include "ecsact/runtime/serialize.hh"
#include "ecsact/runtime/core.hh"
#include "serialize/serialize_test_generated/serialize_test.ecsact.hh"

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

TEST(TagComponentSerialize, Correctness) {
	auto tag_comp = serialize_test::ExampleTagComponent{};
	auto tag_comp_size =
		ecsact_serialize_component_size(serialize_test::ExampleTagComponent::id);
	EXPECT_EQ(tag_comp_size, 0);

	std::vector<uint8_t> serialized_comp(
		ecsact_serialize_component_size(serialize_test::ExampleTagComponent::id)
	);

	ecsact_serialize_component(
		serialize_test::ExampleTagComponent::id,
		&tag_comp,
		serialized_comp.data()
	);

	auto deserialized_comp = serialize_test::ExampleTagComponent{};

	ecsact_deserialize_component(
		serialize_test::ExampleTagComponent::id,
		serialized_comp.data(),
		&deserialized_comp
	);

	ecsact_serialize_component(
		serialize_test::ExampleTagComponent::id,
		nullptr,
		nullptr
	);

	ecsact_deserialize_component(
		serialize_test::ExampleTagComponent::id,
		nullptr,
		nullptr
	);
}

TEST(EntityDump, Success) {
	auto reg = ecsact::core::registry("EntityDumpSuccess");

	auto entity1 = reg.create_entity();
	reg.add_component<serialize_test::ExampleTagComponent>(entity1);

	auto entity2 = reg.create_entity();
	reg.add_component(entity2, serialize_test::ExampleComponent{42});
	reg.add_component<serialize_test::ExampleTagComponent>(entity2);

	auto dump_data = std::deque<std::byte>{};

	EXPECT_EQ(reg.count_entities(), 2);

	ecsact_dump_entities(
		reg.id(),
		[](const void* data, int32_t data_length, void* ud) {
			static_cast<decltype(&dump_data)>(ud)->insert(
				static_cast<decltype(&dump_data)>(ud)->end(),
				static_cast<const std::byte*>(data),
				static_cast<const std::byte*>(data) + data_length
			);
		},
		&dump_data
	);

	const auto dump_data_original_size = dump_data.size();

	EXPECT_EQ(reg.count_entities(), 2);
	EXPECT_FALSE(dump_data.empty());

	reg.clear();

	EXPECT_TRUE(reg.empty());

	auto restore_err = ecsact_restore_entities(
		reg.id(),
		[](void* out_data, int32_t data_max_length, void* ud) -> int32_t {
			if(static_cast<decltype(&dump_data)>(ud)->empty()) {
				return 0;
			}

			for(int32_t i = 0; data_max_length > i; ++i) {
				static_cast<std::byte*>(out_data)[i] =
					static_cast<decltype(&dump_data)>(ud)->front();
				static_cast<decltype(&dump_data)>(ud)->pop_front();
			}

			return data_max_length;
		},
		nullptr,
		&dump_data
	);

	ASSERT_EQ(restore_err, ECSACT_RESTORE_OK);

	EXPECT_TRUE(dump_data.empty()) //
		<< "ecsact_restore_entities did not read all dump data. There are "
		<< dump_data.size() << " bytes left. Started with "
		<< dump_data_original_size << " bytes.";
}
