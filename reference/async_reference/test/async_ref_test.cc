#include <array>

#include "gtest/gtest.h"

#include "ecsact/runtime/async.h"
#include "async_test.ecsact.hh"
#include "async_test.ecsact.systems.hh"

void async_test::AddComponent::impl(context& ctx) {
	ctx.add(async_test::ComponentAddRemove{
		.value = 10,
	});
}

void async_test::UpdateComponent::impl(context& ctx) {
}

void async_test::RemoveComponent::impl(context& ctx) {
}

void async_test::TryEntity::impl(context& ctx) {
	auto action = ctx.action();
}

// TODO(Kelwan): Add more tests for system errors when they're available
TEST(AsyncRef, ConnectBad) {
	auto connect_req_id = ecsact_async_connect("bad");

	auto async_err_cb = //
		[](
			ecsact_async_error      async_err,
			ecsact_async_request_id request_id,
			void*                   callback_user_data
		) { ASSERT_EQ(async_err, ECSACT_ASYNC_ERR_PERMISSION_DENIED); };

	ecsact_async_events_collector async_evc{};
	async_evc.async_error_callback = async_err_cb;

	ecsact_async_flush_events(nullptr, &async_evc);
}

TEST(AsyncRef, Disconnect) {
	auto connect_req_id = ecsact_async_connect("good");

	ecsact_async_disconnect();
}

TEST(AsyncRef, AddUpdateAndRemove) {
	auto connect_req_id = ecsact_async_connect("good");

	async_test::NeededComponent my_needed_component{};
	auto                        needed_comp_id = async_test::NeededComponent::id;
	const void*                 needed_component_data = &my_needed_component;

	ecsact_component needed_component{
		.component_id = needed_comp_id,
		.component_data = needed_component_data,
	};

	async_test::ComponentUpdate my_update_component{};
	auto                        update_comp_id = async_test::ComponentUpdate::id;
	my_update_component.value_to_update = 1;
	const void* update_component_data = &my_update_component;

	ecsact_component update_component{
		.component_id = update_comp_id,
		.component_data = update_component_data,
	};

	auto entity_request = ecsact_async_request_entity();

	bool             entity_wait = false;
	ecsact_entity_id entity;

	struct entity_cb_info {
		ecsact_entity_id& entity;
		bool&             wait;
	};

	auto entity_cb = //
		[](
			ecsact_entity_id        entity_id,
			ecsact_async_request_id request_id,
			void*                   callback_user_data
		) {
			entity_cb_info& entity_info =
				*static_cast<entity_cb_info*>(callback_user_data);
			entity_info.wait = true;
			entity_info.entity = entity_id;
		};

	entity_cb_info entity_info{.entity = entity, .wait = entity_wait};

	ecsact_async_events_collector async_evc{};
	async_evc.async_entity_callback = entity_cb;
	async_evc.async_entity_error_callback_user_data = &entity_info;

	while(entity_wait != true) {
		ecsact_async_flush_events(nullptr, &async_evc);
	}

	std::array<ecsact_component, 2> components = {
		needed_component,
		update_component};
	std::array<ecsact_entity_id, 2> components_entities = {entity, entity};

	ecsact_execution_options add_options{};

	add_options.add_components_length = components.size();
	add_options.add_components_entities = components_entities.data();
	add_options.add_components = components.data();

	ecsact_async_enqueue_execution_options(add_options);

	ecsact_async_flush_events(nullptr, nullptr);

	my_update_component.value_to_update += 5;

	std::array<ecsact_component, 1> update_components = {update_component};

	ecsact_execution_options update_options{};
	update_options.update_components_length = update_components.size();
	update_options.update_components_entities = components_entities.data();
	update_options.update_components = update_components.data();

	ecsact_async_enqueue_execution_options(add_options);

	ecsact_async_flush_events(nullptr, nullptr);

	std::array<ecsact_component_id, 1> remove_components = {update_comp_id};

	ecsact_execution_options remove_options{};
	remove_options.remove_components_length = 1;
	remove_options.remove_components_entities = components_entities.data();
	remove_options.remove_components = remove_components.data();

	ecsact_async_enqueue_execution_options(remove_options);

	ecsact_async_flush_events(nullptr, nullptr);

	ecsact_async_disconnect();
}

TEST(AsyncRef, TryMergeFailure) {
	auto connect_req_id = ecsact_async_connect("good");

	async_test::NeededComponent my_needed_component{};
	auto                        needed_comp_id = async_test::NeededComponent::id;
	const void*                 needed_component_data = &my_needed_component;

	async_test::NeededComponent another_my_needed_component{};
	auto        another_needed_comp_id = async_test::NeededComponent::id;
	const void* another_needed_component_data = &my_needed_component;

	ecsact_component needed_component{
		.component_id = needed_comp_id,
		.component_data = needed_component_data,
	};

	ecsact_component another_needed_component{
		.component_id = another_needed_comp_id,
		.component_data = another_needed_component_data,
	};

	auto entity_request = ecsact_async_request_entity();

	bool             entity_wait = false;
	ecsact_entity_id entity;

	struct entity_cb_info {
		ecsact_entity_id& entity;
		bool&             wait;
	};

	auto entity_cb = //
		[](
			ecsact_entity_id        entity_id,
			ecsact_async_request_id request_id,
			void*                   callback_user_data
		) {
			entity_cb_info& entity_info =
				*static_cast<entity_cb_info*>(callback_user_data);
			entity_info.wait = true;
			entity_info.entity = entity_id;
		};

	entity_cb_info entity_info{.entity = entity, .wait = entity_wait};

	ecsact_async_events_collector entity_async_evc{};
	entity_async_evc.async_entity_callback = entity_cb;
	entity_async_evc.async_entity_error_callback_user_data = &entity_info;

	while(entity_wait != true) {
		ecsact_async_flush_events(nullptr, &entity_async_evc);
	}

	auto async_err_cb = //
		[](
			ecsact_async_error      async_err,
			ecsact_async_request_id request_id,
			void*                   callback_user_data
		) { ASSERT_EQ(async_err, ECSACT_ASYNC_ERR_EXECUTION_MERGE_FAILURE); };

	ecsact_async_events_collector async_evc{};
	async_evc.async_error_callback = async_err_cb;

	std::array<ecsact_entity_id, 2> entities{entity, entity};
	std::array<ecsact_component, 2> components{
		needed_component,
		another_needed_component};

	ecsact_execution_options options{};
	options.add_components_entities = entities.data();
	options.add_components = components.data();
	options.add_components_length = entities.size();

	ecsact_async_enqueue_execution_options(options);

	ecsact_async_flush_events(nullptr, &async_evc);

	ecsact_async_disconnect();
}

TEST(AsyncRef, ReceiveMultipleEntities) {
	auto connect_req_id = ecsact_async_connect("good");

	for(int i = 0; i < 10; i++) {
		ecsact_async_request_entity();
	}

	int              counter = 0;
	ecsact_entity_id entity;

	struct entity_cb_info {
		ecsact_entity_id& entity;
		int&              counter;
	};

	auto entity_cb = //
		[](
			ecsact_entity_id        entity_id,
			ecsact_async_request_id request_id,
			void*                   callback_user_data
		) {
			entity_cb_info& entity_info =
				*static_cast<entity_cb_info*>(callback_user_data);
			entity_info.counter++;
		};

	entity_cb_info entity_info{.entity = entity, .counter = counter};

	ecsact_async_events_collector entity_async_evc{};
	entity_async_evc.async_entity_callback = entity_cb;
	entity_async_evc.async_entity_error_callback_user_data = &entity_info;

	while(counter < 10) {
		ecsact_async_flush_events(nullptr, &entity_async_evc);
	}

	ecsact_async_disconnect();
}
