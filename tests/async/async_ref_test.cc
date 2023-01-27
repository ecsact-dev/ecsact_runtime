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
using std::chrono::duration_cast;

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

void assert_time_past(
	std::chrono::time_point<std::chrono::high_resolution_clock> start_time,
	std::chrono::milliseconds                                   time_to_assert
) {
	auto wait_end = std::chrono::high_resolution_clock::now();
	auto time = duration_cast<std::chrono::milliseconds>(wait_end - start_time);
	ASSERT_LT(time, time_to_assert);
}

void assert_never_error(
	ecsact_async_error       async_err,
	int                      request_ids_length,
	ecsact_async_request_id* request_ids,
	void*                    callback_user_data
) {
	ASSERT_TRUE(false) << "Unexpected Ecsact Async Error";
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
	using clock = std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;

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

	auto start_tick = ecsact_async_get_current_tick();
	while(cb_info.wait != true) {
		ecsact_async_flush_events(nullptr, &entity_async_evc);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
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

	// Prepare the events collector for the flush to make sure we got all the
	// events we expected.
	auto evc = ecsact_execution_events_collector{};
	evc.init_callback_user_data = &cb_info;
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

			if(component_id == async_test::ComponentUpdate::id) {
				auto comp =
					static_cast<const async_test::ComponentUpdate*>(component_data);

				ASSERT_EQ(comp->value_to_update, 1);
			}
		};

	start_tick = ecsact_async_get_current_tick();
	while(!cb_info.init_happened) {
		ecsact_async_flush_events(&evc, nullptr);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	evc.init_callback = {};
	evc.init_callback_user_data = nullptr;
	evc.update_callback_user_data = &cb_info;
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
			ASSERT_EQ(component_id, async_test::ComponentUpdate::id);

			auto comp =
				static_cast<const async_test::ComponentUpdate*>(component_data);

			ASSERT_EQ(comp->value_to_update, 6);
		};

	// Update components
	ecsact_execution_options update_options{};
	update_options.update_components_length = update_components.size();
	update_options.update_components_entities = component_update_entities.data();
	update_options.update_components = update_components.data();
	ecsact_async_enqueue_execution_options(update_options);

	start_tick = ecsact_async_get_current_tick();
	while(!cb_info.update_happened) {
		ecsact_async_flush_events(&evc, nullptr);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	evc.update_callback = {};
	evc.update_callback_user_data = nullptr;

	// Prepare remove component data
	auto remove_components = std::array{async_test::ComponentUpdate::id};
	auto components_remove_entities = std::array{cb_info.entity};

	// Remove component
	auto remove_options = ecsact_execution_options{};
	remove_options.remove_components_length = remove_components.size();
	remove_options.remove_components_entities = components_remove_entities.data();
	remove_options.remove_components = remove_components.data();
	ecsact_async_enqueue_execution_options(remove_options);

	evc.remove_callback_user_data = &cb_info;
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
			ASSERT_EQ(component_id, async_test::ComponentUpdate::id);

			auto comp =
				static_cast<const async_test::ComponentUpdate*>(component_data);

			ASSERT_EQ(comp->value_to_update, 6);
		};

	start_tick = ecsact_async_get_current_tick();
	// Flush until we hit a maximum or get all our events we expect.
	while(!cb_info.all_events_happened()) {
		ecsact_async_flush_events(&evc, nullptr);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10)
			<< "Not all events happened before maximum was reached\n"
			<< "  init = " << cb_info.init_happened << "\n"
			<< "  update = " << cb_info.update_happened << "\n"
			<< "  remove = " << cb_info.remove_happened << "\n";
	}

	ecsact_async_disconnect();
}

TEST(AsyncRef, TryMergeFailure) {
	using namespace std::chrono_literals;
	using clock = std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;

	struct entity_cb_info {
		ecsact_entity_id entity;
		bool             wait;
	};

	ecsact_async_connect("good?tick_rate=25");

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

	auto start_tick = ecsact_async_get_current_tick();
	while(cb_info.wait != true) {
		ecsact_async_flush_events(nullptr, &entity_async_evc);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	struct merge_callback_data {
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
			auto& cb_data = *static_cast<merge_callback_data*>(callback_user_data);

			ASSERT_EQ(async_err, ECSACT_ASYNC_ERR_EXECUTION_MERGE_FAILURE);

			auto request_id = request_ids[0];
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

	merge_callback_data merge_cb_info{
		.request_id = options_request,
		.wait = false,
	};

	ecsact_async_events_collector async_evc{};
	async_evc.async_error_callback = async_err_cb;
	async_evc.async_error_callback_user_data = &merge_cb_info;

	start_tick = ecsact_async_get_current_tick();
	while(merge_cb_info.wait != true) {
		ecsact_async_flush_events(nullptr, &async_evc);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	ecsact_async_disconnect();
}

TEST(AsyncRef, ReceiveMultipleEntities) {
	using namespace std::chrono_literals;
	using clock = std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;

	auto connect_req_id = ecsact_async_connect("good?tick_rate=25");

	for(int i = 0; i < 10; i++) {
		ecsact_async_create_entity();
	}

	int              counter = 0;
	ecsact_entity_id entity;

	struct entity_cb_info {
		ecsact_entity_id&   entity;
		std::array<int, 10> entity_request_ids;
		int&                counter;
	};

	auto entity_cb = //
		[](
			ecsact_entity_id        entity_id,
			ecsact_async_request_id request_id,
			void*                   callback_user_data
		) {
			entity_cb_info& entity_info =
				*static_cast<entity_cb_info*>(callback_user_data);
			entity_info.entity_request_ids[entity_info.counter] = entity_info.counter;
			entity_info.counter++;
		};

	entity_cb_info entity_info{
		.entity = entity,
		.entity_request_ids = {},
		.counter = counter};

	ecsact_async_events_collector entity_async_evc{};
	entity_async_evc.async_entity_callback = entity_cb;
	entity_async_evc.async_entity_callback_user_data = &entity_info;

	auto start_tick = ecsact_async_get_current_tick();
	while(counter < 10) {
		ecsact_async_flush_events(nullptr, &entity_async_evc);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	for(int i = 0; i < entity_info.entity_request_ids.size(); i++) {
		ASSERT_EQ(i, entity_info.entity_request_ids[i]);
	}

	ecsact_async_disconnect();
}

TEST(AsyncRef, TryAction) {
	using namespace std::chrono_literals;
	using clock = std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;

	static std::atomic_bool reached_system = false;

	ecsact_async_connect("good?tick_rate=25");

	auto entity_request = ecsact_async_create_entity();

	struct entity_cb_info {
		ecsact_entity_id entity;
		bool             wait;
	};

	static entity_cb_info cb_info;

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
	entity_async_evc.async_error_callback = &assert_never_error;

	auto start_tick = ecsact_async_get_current_tick();
	while(cb_info.wait != true) {
		ecsact_async_flush_events(nullptr, &entity_async_evc);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	struct callback_info {
		bool wait = false;
	};

	callback_info init_cb_info{};

	// Prepare the events collector for the flush to make sure we got all the
	// events we expected.
	auto evc = ecsact_execution_events_collector{};
	evc.init_callback_user_data = &init_cb_info;
	evc.init_callback = //
		[](
			ecsact_event        event,
			ecsact_entity_id    entity_id,
			ecsact_component_id component_id,
			const void*         component_data,
			void*               callback_user_data
		) {
			auto  wait_end = clock::now();
			auto& info = *static_cast<callback_info*>(callback_user_data);

			info.wait = true;
		};

	// Declare components required for the action
	async_test::NeededComponent my_needed_component{};

	async_test::ComponentUpdate my_update_component{};
	my_update_component.value_to_update = 1;

	std::array add_component_entities{cb_info.entity, cb_info.entity};

	// Add components to an array
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

	ASSERT_EQ(add_component_entities.size(), add_components.size());

	ecsact_execution_options options{};

	options.add_components_length = add_components.size();
	options.add_components_entities = add_component_entities.data();
	options.add_components = add_components.data();

	ecsact_async_enqueue_execution_options(options);

	start_tick = ecsact_async_get_current_tick();
	while(!cb_info.wait) {
		ecsact_async_flush_events(&evc, nullptr);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	async_test::TryEntity my_try_entity;

	my_try_entity.my_entity = cb_info.entity;

	// Declare an action, add a check to see it's running
	ecsact_set_system_execution_impl(
		ecsact_id_cast<ecsact_system_like_id>(async_test::TryEntity::id),
		[](ecsact_system_execution_context* context) {
			async_test::TryEntity system_action{};
			ecsact_system_execution_context_action(context, &system_action);
			ASSERT_EQ(cb_info.entity, system_action.my_entity);
			reached_system = true;
		}
	);

	std::vector<ecsact_action> actions{};

	ecsact_action my_action{
		.action_id = async_test::TryEntity::id,
		.action_data = &my_try_entity,
	};

	actions.push_back(my_action);
	options = {};

	options.actions = actions.data();
	options.actions_length = actions.size();

	auto options_request = ecsact_async_enqueue_execution_options(options);

	start_tick = ecsact_async_get_current_tick();
	while(reached_system != true) {
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	ecsact_async_disconnect();
}

Test(AsyncRef, FlushNoEventsOrConnect) {
	ecsact_async_flush_events(nullptr, nullptr);
}

Test(AsyncRef, EnqueueErrorBeforeConnect) {
	// It doesn't matter what is in our options. We should get an error regardless
	// of it's content if we aren't connected.
	auto        options = ecsact_execution_options{};
	static auto req_id = ecsact_async_enqueue_execution_options(options);
	static auto async_error_happened = false;

	auto async_evc = ecsact_async_events_collector{};
	async_evc.async_error_callback = //
		[](
			ecsact_async_error       async_err,
			int                      request_ids_length,
			ecsact_async_request_id* request_ids,
			void*                    callback_user_data
		) {
			async_error_happened = true;
			ASSERT_EQ(request_ids_length, 1);
			ASSERT_EQ(request_ids[0], req_id);
			ASSERT_EQ(async_err, ECSACT_ASYNC_ERR_PERMISSION_DENIED);
			ASSERT_EQ(callback_user_data, nullptr);
		};

	// The reference implementation gives an error right away if not connected
	// but a different implementation of the async API may delay the error.
	ecsact_async_flush_events(nullptr, &async_evc);
	ASSERT_TRUE(async_error_happened);
}
