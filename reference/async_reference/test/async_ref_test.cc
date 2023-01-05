#include <array>
#include <thread>
#include <chrono>
#include <atomic>

#include "gtest/gtest.h"

#include "ecsact/runtime/async.h"
#include "ecsact/runtime/dynamic.h"
#include "async_test.ecsact.hh"
#include "async_test.ecsact.systems.hh"

using namespace std::chrono_literals;

struct entity_cb_info {
	ecsact_entity_id entity;
	bool             wait;
};

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

TEST(AsyncRef, ConnectBad) {
	auto connect_req_id = ecsact_async_connect("bad");

	auto async_err_cb = //
		[](
			ecsact_async_error       async_err,
			int                      request_ids_length,
			ecsact_async_request_id* request_ids,
			void*                    callback_user_data
		) { ASSERT_EQ(async_err, ECSACT_ASYNC_ERR_PERMISSION_DENIED); };

	ecsact_async_events_collector async_evc{};
	async_evc.async_error_callback = async_err_cb;

	ecsact_async_flush_events(nullptr, &async_evc);
	ecsact_async_disconnect();
}

TEST(AsyncRef, InvalidConnectionString) {
	static auto req_id = ecsact_async_connect("good?bad_option=true&foo=baz");

	auto async_err_cb = //
		[](
			ecsact_async_error       async_err,
			int                      request_ids_length,
			ecsact_async_request_id* request_ids,
			void*                    callback_user_data
		) {
			ASSERT_EQ(async_err, ECSACT_ASYNC_INVALID_CONNECTION_STRING);
			ASSERT_EQ(request_ids_length, 1);
			ASSERT_EQ(request_ids[0], req_id);
		};

	ecsact_async_events_collector async_evc{};
	async_evc.async_error_callback = async_err_cb;

	ecsact_async_flush_events(nullptr, &async_evc);
}

TEST(AsyncRef, Disconnect) {
	auto connect_req_id = ecsact_async_connect("good");

	ecsact_async_disconnect();
}

TEST(AsyncRef, AddUpdateAndRemove) {
	using namespace std::chrono_literals;

	// In this test we're adding to components and then immediately updating one
	// and then removing it.

	// First we'll need to connect to the async API and create an entity. We'll
	// set our tick rate to 25ms.
	auto connect_req_id = ecsact_async_connect("good?tick_rate=25");
	auto entity_request = ecsact_async_create_entity();

	// This will store temporary state in our entity callback
	struct callback_data {
		std::atomic_bool wait = false;
		ecsact_entity_id entity = {};
		std::atomic_bool init_happened = false;
		std::atomic_bool update_happened = false;
		std::atomic_bool remove_happened = false;

		auto all_events_happened() -> bool {
			return init_happened && update_happened && remove_happened;
		}
	};

	auto cb_info = callback_data{};

	auto entity_cb = //
		[](
			ecsact_entity_id        entity_id,
			ecsact_async_request_id request_id,
			void*                   callback_user_data
		) {
			auto& cb_info = *static_cast<callback_data*>(callback_user_data);
			cb_info.wait = true;
			cb_info.entity = entity_id;
		};

	ecsact_async_events_collector entity_async_evc{};
	entity_async_evc.async_entity_callback = entity_cb;
	entity_async_evc.async_entity_callback_user_data = &cb_info;

	int check_count = 0;

	while(cb_info.wait != true) {
		ASSERT_LT(++check_count, 100);
		ecsact_async_flush_events(nullptr, &entity_async_evc);
		std::this_thread::sleep_for(25ms);
	}

	// Preparing add component data
	auto my_needed_component = async_test::NeededComponent{};
	auto my_update_component = async_test::ComponentUpdate{.value_to_update = 1};

	std::array add_components{
		ecsact_component{
			.component_id = async_test::NeededComponent::id,
			.component_data = &my_needed_component,
		},
		ecsact_component{
			.component_id = async_test::ComponentUpdate::id,
			.component_data = &my_update_component,
		},
	};
	auto add_components_entities = std::array{cb_info.entity, cb_info.entity};

	ASSERT_EQ(add_components_entities.size(), add_components.size());

	// Adding components
	auto add_options = ecsact_execution_options{};
	add_options.add_components_length = add_components.size();
	add_options.add_components_entities = add_components_entities.data();
	add_options.add_components = add_components.data();
	ecsact_async_enqueue_execution_options(add_options);

	// Prepare update component data
	my_update_component.value_to_update += 5;
	auto update_components = std::array{
		ecsact_component{
			.component_id = async_test::ComponentUpdate::id,
			.component_data = &my_update_component,
		},
	};
	auto component_update_entities = std::array{cb_info.entity};

	// Update components
	ecsact_execution_options update_options{};
	update_options.update_components_length = update_components.size();
	update_options.update_components_entities = component_update_entities.data();
	update_options.update_components = update_components.data();
	ecsact_async_enqueue_execution_options(update_options);

	// Prepare remove component data
	auto remove_components = std::array{async_test::ComponentUpdate::id};
	auto components_remove_entities = std::array{cb_info.entity};

	// Remove component
	auto remove_options = ecsact_execution_options{};
	remove_options.remove_components_length = remove_components.size();
	remove_options.remove_components_entities = components_remove_entities.data();
	remove_options.remove_components = remove_components.data();
	ecsact_async_enqueue_execution_options(remove_options);

	// Prepare the events collector for the flush to make sure we got all the
	// events we expected.
	auto evc = ecsact_execution_events_collector{};
	evc.init_callback_user_data = &cb_info;
	evc.update_callback_user_data = &cb_info;
	evc.remove_callback_user_data = &cb_info;
	evc.init_callback = //
		[](
			ecsact_event        event,
			ecsact_entity_id    entity_id,
			ecsact_component_id component_id,
			const void*         component_data,
			void*               callback_user_data
		) {
			auto& cb_info = *static_cast<callback_data*>(callback_user_data);
			cb_info.init_happened = true;
		};
	evc.update_callback = //
		[](
			ecsact_event        event,
			ecsact_entity_id    entity_id,
			ecsact_component_id component_id,
			const void*         component_data,
			void*               callback_user_data
		) {
			auto& cb_info = *static_cast<callback_data*>(callback_user_data);
			cb_info.update_happened = true;

			EXPECT_EQ(component_id, async_test::ComponentUpdate::id);
		};
	evc.remove_callback = //
		[](
			ecsact_event        event,
			ecsact_entity_id    entity_id,
			ecsact_component_id component_id,
			const void*         component_data,
			void*               callback_user_data
		) {
			auto& cb_info = *static_cast<callback_data*>(callback_user_data);
			cb_info.remove_happened = true;

			EXPECT_EQ(component_id, async_test::ComponentUpdate::id);
		};

	// Flush until we hit a maximum or get all our events we expect.
	check_count = 0;
	while(!cb_info.all_events_happened()) {
		ASSERT_LT(++check_count, 100) //
			<< "Not all events happened before maximum was reached\n"
			<< "  init = " << cb_info.init_happened << "\n"
			<< "  update = " << cb_info.update_happened << "\n"
			<< "  remove = " << cb_info.remove_happened << "\n";
		ecsact_async_flush_events(&evc, nullptr);
		std::this_thread::sleep_for(25ms);
	}

	ecsact_async_disconnect();

	// NOTE: Maybe merging enqueues is affecting the callbacks?
}

TEST(AsyncRef, TryMergeFailure) {
	using namespace std::chrono_literals;

	auto connect_req_id = ecsact_async_connect("good?tick_rate=25");

	auto my_needed_component = async_test::NeededComponent{};
	auto another_my_needed_component = async_test::NeededComponent{};

	ecsact_component needed_component{
		.component_id = async_test::NeededComponent::id,
		.component_data = &my_needed_component,
	};

	ecsact_component another_needed_component{
		.component_id = async_test::NeededComponent::id,
		.component_data = &another_my_needed_component,
	};

	auto entity_request = ecsact_async_create_entity();

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

	entity_cb_info cb_info{};

	ecsact_async_events_collector entity_async_evc{};
	entity_async_evc.async_entity_callback = entity_cb;
	entity_async_evc.async_entity_callback_user_data = &cb_info;

	int check_count = 0;

	while(cb_info.wait != true) {
		std::this_thread::sleep_for(25ms);
		ASSERT_LT(++check_count, 100);
		ecsact_async_flush_events(nullptr, &entity_async_evc);
	}

	struct callback_data {
		ecsact_async_request_id request_id;
		bool                    wait;
	};

	auto async_err_cb = //
		[](
			ecsact_async_error       async_err,
			int                      request_ids_length,
			ecsact_async_request_id* request_ids,
			void*                    callback_user_data
		) {
			ASSERT_EQ(async_err, ECSACT_ASYNC_ERR_EXECUTION_MERGE_FAILURE);
			auto& cb_data = *static_cast<callback_data*>(callback_user_data);
			auto  request_id = request_ids[0];
			ASSERT_EQ(cb_data.request_id, request_id);
			cb_data.wait = true;
		};

	std::array entities{cb_info.entity, cb_info.entity};
	std::array components{
		needed_component,
		another_needed_component,
	};

	ecsact_execution_options options{};
	options.add_components_entities = entities.data();
	options.add_components = components.data();
	options.add_components_length = entities.size();

	auto options_request = ecsact_async_enqueue_execution_options(options);

	callback_data cb_data{
		.request_id = options_request,
		.wait = false,
	};

	ecsact_async_events_collector async_evc{};
	async_evc.async_error_callback = async_err_cb;
	async_evc.async_error_callback_user_data = &cb_data;

	check_count = 0;

	while(cb_data.wait != true) {
		std::this_thread::sleep_for(25ms);
		ASSERT_LT(++check_count, 100);
		ecsact_async_flush_events(nullptr, &async_evc);
	}

	ecsact_async_disconnect();
}

TEST(AsyncRef, ReceiveMultipleEntities) {
	using namespace std::chrono_literals;

	auto connect_req_id = ecsact_async_connect("good?tick_rate=25");

	for(int i = 0; i < 10; i++) {
		ecsact_async_create_entity();
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
	entity_async_evc.async_entity_callback_user_data = &entity_info;

	int check_count = 0;

	while(counter < 10) {
		std::this_thread::sleep_for(25ms);
		ASSERT_LT(++check_count, 100);
		ecsact_async_flush_events(nullptr, &entity_async_evc);
	}

	ecsact_async_disconnect();
}

TEST(AsyncRef, TryAction) {
	using namespace std::chrono_literals;
	static std::atomic_bool reached_system = false;

	ecsact_set_system_execution_impl(
		ecsact_id_cast<ecsact_system_like_id>(async_test::TryEntity::id),
		[](ecsact_system_execution_context* context) { reached_system = true; }
	);

	ecsact_async_connect("good?tick_rate=25");

	async_test::NeededComponent my_needed_component{};

	ecsact_component needed_component{
		.component_id = async_test::NeededComponent::id,
		.component_data = &my_needed_component,
	};

	async_test::ComponentUpdate my_update_component{};

	auto update_comp_id = async_test::ComponentUpdate::id;
	my_update_component.value_to_update = 1;
	const void* update_component_data = &my_update_component;

	ecsact_component update_component{
		.component_id = update_comp_id,
		.component_data = update_component_data,
	};

	auto entity_request = ecsact_async_create_entity();

	entity_cb_info cb_info{};

	auto entity_cb = //
		[](
			ecsact_entity_id        entity_id,
			ecsact_async_request_id request_id,
			void*                   callback_user_data
		) {
			auto& entity_info = *static_cast<entity_cb_info*>(callback_user_data);
			entity_info.wait = true;
			entity_info.entity = entity_id;
		};

	ecsact_async_events_collector entity_async_evc{};
	entity_async_evc.async_entity_callback = entity_cb;
	entity_async_evc.async_entity_callback_user_data = &cb_info;

	int check_count = 0;

	while(cb_info.wait != true) {
		std::this_thread::sleep_for(25ms);
		ASSERT_LT(++check_count, 100);
		ecsact_async_flush_events(nullptr, &entity_async_evc);
	}

	async_test::TryEntity my_try_entity;

	my_try_entity.my_entity = cb_info.entity;

	std::vector<ecsact_action> actions{};

	ecsact_action my_action{
		.action_id = async_test::TryEntity::id,
		.action_data = &my_try_entity,
	};

	actions.push_back(my_action);

	ecsact_async_events_collector async_evc{};

	ecsact_execution_options options{};

	options.actions = actions.data();
	options.actions_length = actions.size();

	auto options_request = ecsact_async_enqueue_execution_options(options);

	check_count = 0;

	while(reached_system != true) {
		std::this_thread::sleep_for(25ms);
		ASSERT_LT(++check_count, 100);
	}
}
