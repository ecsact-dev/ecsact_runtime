#include <array>
#include <atomic>
#include <chrono>
#include <thread>

#include "gtest/gtest.h"

#include "async_test.ecsact.hh"
#include "async_test.ecsact.systems.hh"
#include "ecsact/runtime/async.hh"
#include "ecsact/runtime/core.hh"
#include "ecsact/runtime/dynamic.h"

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
	// santity check
	ctx.action();
}

void assert_time_past(
	std::chrono::time_point<std::chrono::high_resolution_clock> start_time,
	std::chrono::milliseconds                                   time_to_assert
) {
	auto wait_end = std::chrono::high_resolution_clock::now();
	auto time = duration_cast<std::chrono::milliseconds>(wait_end - start_time);
	ASSERT_LT(time, time_to_assert);
}

static bool _error_happened = false;

void assert_never_async_error(
	ecsact_async_error       async_err,
	int                      request_ids_length,
	ecsact_async_request_id* request_ids,
	void*                    callback_user_data
) {
	_error_happened = true;
	EXPECT_EQ(async_err, ECSACT_ASYNC_OK) //
		<< "Unexpected Ecsact Async Error";
}

void assert_never_system_error(
	ecsact_execute_systems_error execute_err,
	void*                        callback_user_data
) {
	_error_happened = true;
	EXPECT_EQ(execute_err, ECSACT_EXEC_SYS_OK) //
		<< "Unexpected Ecsact System Error";
}

void flush_events_never_error(const ecsact_execution_events_collector* exec_evc
) {
	_error_happened = false;
	auto async_evc = ecsact_async_events_collector{};
	async_evc.async_error_callback = &assert_never_async_error;
	async_evc.system_error_callback = &assert_never_system_error;
	ecsact_async_flush_events(exec_evc, &async_evc);
}

#define FLUSH_EVENTS_NEVER_ERROR(exec_evc) \
	flush_events_never_error(exec_evc);      \
	ASSERT_FALSE(_error_happened)

TEST(AsyncRef, ConnectBad) {
	auto connect_req_id = ecsact::async::connect("bad");
	auto async_evc = ecsact::async::async_events_collector<>{};
	async_evc.set_async_error_callback(
		[&](ecsact_async_error err, std::span<ecsact_async_request_id> req_ids) {
			ASSERT_EQ(req_ids.size(), 1);
			ASSERT_EQ(req_ids[0], connect_req_id);
			ASSERT_EQ(err, ECSACT_ASYNC_ERR_PERMISSION_DENIED);
		}
	);

	ecsact::async::flush_events(async_evc);
	ecsact::async::disconnect();
}

TEST(AsyncRef, InvalidConnectionString) {
	auto connect_req_id = ecsact::async::connect("good?bad_option=true&foo=baz");
	auto async_evc = ecsact::async::async_events_collector<>{};
	async_evc.set_async_error_callback(
		[&](ecsact_async_error err, std::span<ecsact_async_request_id> req_ids) {
			ASSERT_EQ(req_ids.size(), 1);
			ASSERT_EQ(req_ids[0], connect_req_id);
			ASSERT_EQ(err, ECSACT_ASYNC_INVALID_CONNECTION_STRING);
		}
	);

	ecsact::async::flush_events(async_evc);
	ecsact::async::disconnect();
}

TEST(AsyncRef, Disconnect) {
	// Ignoring request ID
	static_cast<void>(ecsact::async::connect("good"));
	ecsact::async::disconnect();
}

TEST(AsyncRef, AddUpdateAndRemove) {
	using namespace std::chrono_literals;
	using std::chrono::duration_cast;

	// In this test we're adding to components and then immediately updating one
	// and then removing it.

	// First we'll need to connect to the async API and create an entity. We'll
	// set our tick rate to 25ms.
	ecsact_async_connect("good?delta_speed=25");

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
			ecsact_event                 event,
			ecsact_entity_id             entity_id,
			ecsact_placeholder_entity_id placeholder_entity_id,
			void*                        callback_user_data
		) {
			auto& cb_info = *static_cast<callback_data*>(callback_user_data);

			cb_info.wait = true;
			cb_info.entity = entity_id;
		};

	ecsact::core::execution_options options{};

	auto evc = ecsact_execution_events_collector{};
	evc.entity_created_callback = entity_cb;
	evc.entity_created_callback_user_data = &cb_info;

	auto my_needed_component = async_test::NeededComponent{};

	options.create_entity().add_component(&my_needed_component);

	ecsact_async_enqueue_execution_options(options.c());

	auto start_tick = ecsact_async_get_current_tick();
	while(cb_info.wait != true) {
		std::this_thread::yield();
		ecsact_async_flush_events(&evc, nullptr);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	// Reset events collector and options
	evc.entity_created_callback = {};
	evc.entity_created_callback_user_data = nullptr;
	options.clear();

	// Preparing add component data
	auto my_update_component = async_test::ComponentUpdate{.value_to_update = 1};

	options.add_component(cb_info.entity, &my_update_component);

	// Adding components
	ecsact_async_enqueue_execution_options(options.c());

	// Prepare the events collector for the flush to make sure we got all the
	// events we expected.
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
		std::this_thread::yield();
		ecsact_async_flush_events(&evc, nullptr);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	evc.init_callback = {};
	evc.init_callback_user_data = nullptr;
	options.clear();

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

	// Prepare update component data
	my_update_component.value_to_update += 5;

	// Update components
	options.update_component(cb_info.entity, &my_update_component);

	ecsact_async_enqueue_execution_options(options.c());

	start_tick = ecsact_async_get_current_tick();
	while(!cb_info.update_happened) {
		std::this_thread::yield();
		ecsact_async_flush_events(&evc, nullptr);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	options.clear();
	evc.update_callback = {};
	evc.update_callback_user_data = nullptr;

	// Remove component
	options.remove_component<async_test::ComponentUpdate>(cb_info.entity);

	ecsact_async_enqueue_execution_options(options.c());

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
		std::this_thread::yield();
		ecsact_async_flush_events(&evc, nullptr);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10)
			<< "Not all events happened before maximum was reached\n"
			<< "  init = " << cb_info.init_happened << "\n"
			<< "  update = " << cb_info.update_happened << "\n"
			<< "  remove = " << cb_info.remove_happened << "\n";
	}

	options.clear();
	ecsact::async::disconnect();
}

TEST(AsyncRef, TryMergeFailure) {
	using namespace std::chrono_literals;
	using std::chrono::duration_cast;

	struct entity_cb_info {
		ecsact_entity_id entity;
		bool             wait;
	};

	ecsact_async_connect("good?delta_speed=25");

	struct merge_callback_data {
		ecsact_async_request_id request_id;
		bool                    wait;
	};

	auto entity_cb = //
		[](
			ecsact_event                 event,
			ecsact_entity_id             entity_id,
			ecsact_placeholder_entity_id placeholder_entity_id,
			void*                        callback_user_data
		) {
			entity_cb_info& entity_info =
				*static_cast<entity_cb_info*>(callback_user_data);

			entity_info.wait = true;
			entity_info.entity = entity_id;
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

	entity_cb_info cb_info{};

	ecsact::core::execution_options options{};

	auto my_needed_component = async_test::NeededComponent{};
	auto another_my_needed_component = async_test::NeededComponent{};

	options.create_entity()
		.add_component(&my_needed_component)
		.add_component(&another_my_needed_component);

	auto request_id = ecsact_async_enqueue_execution_options(options.c());

	auto merge_data = merge_callback_data{};
	merge_data.request_id = request_id;
	merge_data.wait = false;

	auto evc = ecsact_execution_events_collector{};
	evc.entity_created_callback = entity_cb;
	evc.entity_created_callback_user_data = &cb_info;

	auto async_evc = ecsact_async_events_collector{};

	async_evc.async_error_callback = async_err_cb;
	async_evc.async_error_callback_user_data = &merge_data;

	auto start_tick = ecsact_async_get_current_tick();
	while(merge_data.wait != true) {
		std::this_thread::yield();
		ecsact_async_flush_events(&evc, &async_evc);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	options.clear();

	ecsact_async_disconnect();
}

TEST(AsyncRef, CreateMultipleEntitiesAndDestroy) {
	using namespace std::chrono_literals;
	using std::chrono::duration_cast;

	ecsact_async_connect("good?delta_speed=25");

	auto options = ecsact::core::execution_options{};

	options.create_entity(static_cast<ecsact_placeholder_entity_id>(42));

	for(int i = 0; i < 10; i++) {
		ecsact_async_enqueue_execution_options(options.c());
	}

	struct entity_cb_info {
		std::array<int, 10>                          entity_request_ids;
		std::array<ecsact_entity_id, 10>             entity_ids;
		std::array<ecsact_placeholder_entity_id, 10> placeholder_ids;
		int&                                         counter;
	};

	auto entity_cb = //
		[](
			ecsact_event                 event,
			ecsact_entity_id             entity_id,
			ecsact_placeholder_entity_id placeholder_entity_id,
			void*                        callback_user_data
		) {
			entity_cb_info& entity_info =
				*static_cast<entity_cb_info*>(callback_user_data);
			entity_info.entity_request_ids[entity_info.counter] = entity_info.counter;
			entity_info.entity_ids[entity_info.counter] = entity_id;
			entity_info.placeholder_ids[entity_info.counter] = placeholder_entity_id;
			entity_info.counter++;
		};

	int counter = 0;

	entity_cb_info entity_info{.entity_request_ids = {}, .counter = counter};

	auto evc = ecsact_execution_events_collector{};
	evc.entity_created_callback = entity_cb;
	evc.entity_created_callback_user_data = &entity_info;

	auto start_tick = ecsact_async_get_current_tick();
	while(counter < 10) {
		std::this_thread::yield();
		ecsact_async_flush_events(&evc, nullptr);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	for(int i = 0; i < entity_info.entity_request_ids.size(); i++) {
		ASSERT_EQ(i, entity_info.entity_request_ids[i]);
		ASSERT_EQ(
			static_cast<ecsact_placeholder_entity_id>(42),
			entity_info.placeholder_ids[i]
		);
	}

	struct destroy_cb_info {
		ecsact_entity_id entity_id;
		bool             wait;
	};

	auto destroy_entity_cb = //
		[](
			ecsact_event                 event,
			ecsact_entity_id             entity_id,
			ecsact_placeholder_entity_id placeholder_entity_id,
			void*                        callback_user_data
		) {
			destroy_cb_info& destroy_cb =
				*static_cast<destroy_cb_info*>(callback_user_data);
			assert(entity_id == destroy_cb.entity_id);
			destroy_cb.wait = true;
		};

	options.clear();
	evc.entity_created_callback = nullptr;
	evc.entity_created_callback_user_data = nullptr;

	destroy_cb_info cb_info{
		.entity_id = entity_info.entity_ids[5],
		.wait = false,
	};

	evc.entity_destroyed_callback = destroy_entity_cb;
	evc.entity_destroyed_callback_user_data = &cb_info;

	options.destroy_entity(cb_info.entity_id);

	ecsact_async_enqueue_execution_options(options.c());

	start_tick = ecsact_async_get_current_tick();
	while(cb_info.wait != true) {
		std::this_thread::yield();
		ecsact_async_flush_events(&evc, nullptr);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	ecsact_async_disconnect();
}

TEST(AsyncRef, TryAction) {
	using namespace std::chrono_literals;
	using std::chrono::duration_cast;

	static std::atomic_bool reached_system = false;

	ecsact_async_connect("good?delta_speed=25");

	struct entity_cb_info {
		ecsact_entity_id entity;
		bool             wait;
	};

	static entity_cb_info cb_info;

	auto entity_cb = //
		[](
			ecsact_event                 event,
			ecsact_entity_id             entity_id,
			ecsact_placeholder_entity_id placeholder_entity_id,
			void*                        callback_user_data
		) {
			cb_info.wait = true;
			cb_info.entity = entity_id;
		};

	// Declare components required for the action
	async_test::NeededComponent my_needed_component{};

	async_test::ComponentUpdate my_update_component{.value_to_update = 1};

	auto options = ecsact::core::execution_options{};

	options.create_entity()
		.add_component(&my_needed_component)
		.add_component(&my_update_component);

	ecsact_async_enqueue_execution_options(options.c());

	auto evc = ecsact_execution_events_collector{};
	evc.entity_created_callback = entity_cb;
	evc.entity_created_callback_user_data = &cb_info;

	auto start_tick = ecsact_async_get_current_tick();
	while(cb_info.wait != true) {
		std::this_thread::yield();
		ecsact_async_flush_events(&evc, nullptr);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	options.clear();

	async_test::TryEntity my_try_entity;

	my_try_entity.my_entity = cb_info.entity;

	// Declare an action, add a check to see it's running
	reached_system = false;
	ecsact_set_system_execution_impl(
		ecsact_id_cast<ecsact_system_like_id>(async_test::TryEntity::id),
		[](ecsact_system_execution_context* context) {
			async_test::TryEntity system_action{};
			ecsact_system_execution_context_action(context, &system_action);
			ASSERT_EQ(cb_info.entity, system_action.my_entity);
			reached_system = true;
		}
	);

	options.push_action(&my_try_entity);

	ecsact_async_enqueue_execution_options(options.c());

	start_tick = ecsact_async_get_current_tick();
	while(reached_system != true) {
		std::this_thread::yield();
		FLUSH_EVENTS_NEVER_ERROR(nullptr);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	ecsact_async_disconnect();
}

TEST(AsyncRef, FlushNoEventsOrConnect) {
	ecsact::async::flush_events();
}

TEST(AsyncRef, EnqueueErrorBeforeConnect) {
	auto options = ecsact::core::execution_options{};
	auto req_id = ecsact::async::enqueue_execution_options(options);
	auto async_error_happened = false;
	auto async_evc = ecsact::async::async_events_collector<>{};
	async_evc.set_async_error_callback(
		[&](ecsact_async_error err, std::span<ecsact_async_request_id> req_ids) {
			async_error_happened = true;
			ASSERT_EQ(req_ids.size(), 1);
			ASSERT_EQ(req_ids[0], req_id);
			ASSERT_EQ(err, ECSACT_ASYNC_ERR_PERMISSION_DENIED);
		}
	);

	// The reference implementation gives an error right away if not connected
	// but a different implementation of the async API may delay the error.
	ecsact::async::flush_events(async_evc);
	ASSERT_TRUE(async_error_happened);
}

TEST(AsyncRef, RemoveTagComponent) {
	ecsact_async_connect("good?delta_speed=25");

	struct callback_info {
		std::atomic_bool wait = false;
		ecsact_entity_id entity = {};
	};

	auto cb_info = callback_info{};

	auto entity_cb = //
		[](
			ecsact_event                 event,
			ecsact_entity_id             entity_id,
			ecsact_placeholder_entity_id placeholder_entity_id,
			void*                        callback_user_data
		) {
			auto& cb_info = *static_cast<callback_info*>(callback_user_data);

			cb_info.wait = true;
			cb_info.entity = entity_id;
		};

	ecsact::core::execution_options options{};

	auto evc = ecsact_execution_events_collector{};
	evc.entity_created_callback = entity_cb;
	evc.entity_created_callback_user_data = &cb_info;

	auto my_needed_component = async_test::NeededComponent{};

	options.create_entity().add_component(&my_needed_component);

	ecsact_async_enqueue_execution_options(options.c());

	auto start_tick = ecsact_async_get_current_tick();
	while(cb_info.wait != true) {
		std::this_thread::yield();
		ecsact_async_flush_events(&evc, nullptr);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	options.clear();
	evc.entity_created_callback = {};
	evc.entity_created_callback_user_data = nullptr;

	// Remove component
	options.remove_component<async_test::NeededComponent>(cb_info.entity);

	ecsact_async_enqueue_execution_options(options.c());

	cb_info.wait = false;

	evc.remove_callback_user_data = &cb_info;
	evc.remove_callback = //
		[](
			ecsact_event        event,
			ecsact_entity_id    entity_id,
			ecsact_component_id component_id,
			const void*         component_data,
			void*               callback_user_data
		) {
			auto& cb_info = *static_cast<callback_info*>(callback_user_data);

			cb_info.wait = true;
			ASSERT_EQ(component_id, async_test::NeededComponent::id);
		};

	start_tick = ecsact_async_get_current_tick();
	while(cb_info.wait != true) {
		std::this_thread::yield();
		ecsact_async_flush_events(&evc, nullptr);
		auto current_tick = ecsact_async_get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	ecsact_async_disconnect();
}
