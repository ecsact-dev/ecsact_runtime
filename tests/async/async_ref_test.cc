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

void flush_with_condition(bool& condition, auto&& evc) {
	auto start_tick = ecsact::async::get_current_tick();
	while(condition != true) {
		std::this_thread::yield();
		ecsact::async::flush_events(evc);
		auto current_tick = ecsact::async::get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}
}

void flush_with_condition(bool& condition, auto&& evc, auto&& async_evc) {
	auto start_tick = ecsact::async::get_current_tick();
	while(condition != true) {
		std::this_thread::yield();
		ecsact::async::flush_events(evc, async_evc);
		auto current_tick = ecsact::async::get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}
}

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
	static_cast<void>(ecsact::async::connect("good?delta_time=25"));

	ecsact_entity_id entity = {};

	bool entity_wait = false;

	// Declare an execution options that can affect the system execution loop
	ecsact::core::execution_options options{};

	// Declare an event collector that gathers events from the async execution
	auto evc = ecsact::core::execution_events_collector<>{};

	evc.set_entity_created_callback(
		[&](ecsact_entity_id entity_id, ecsact_placeholder_entity_id) {
			entity_wait = true;
			entity = entity_id;
		}
	);

	// Declare a component to be added to an entity
	auto my_needed_component = async_test::NeededComponent{};

	// Use the builder on options to add a component
	options.create_entity().add_component(&my_needed_component);

	// Queue the options to be used in the async execution
	// It will continue or assert until the expected callback is invoked
	static_cast<void>(ecsact::async::enqueue_execution_options(options));

	// Flush for feedback from the async execution
	// An event is expected
	flush_with_condition(entity_wait, evc);

	options.clear();
	evc.clear();

	auto my_update_component = async_test::ComponentUpdate{.value_to_update = 1};

	bool init_happened = false;
	options.add_component(entity, &my_update_component);

	static_cast<void>(ecsact::async::enqueue_execution_options(options));

	evc.set_init_callback<async_test::ComponentUpdate>(
		[&](ecsact_entity_id id, const async_test::ComponentUpdate& component) {
			init_happened = true;
			ASSERT_EQ(component.value_to_update, 1);
		}
	);

	flush_with_condition(init_happened, evc);

	options.clear();
	evc.clear();

	bool update_happened = false;

	evc.set_update_callback<async_test::ComponentUpdate>(
		[&](ecsact_entity_id id, const async_test::ComponentUpdate& component) {
			update_happened = true;
			ASSERT_EQ(component.value_to_update, 6);
		}
	);

	my_update_component.value_to_update += 5;

	options.update_component(entity, &my_update_component);

	static_cast<void>(ecsact::async::enqueue_execution_options(options));

	flush_with_condition(update_happened, evc);

	options.clear();
	evc.clear();

	bool remove_happened = false;

	// Remove component
	options.remove_component<async_test::ComponentUpdate>(entity);

	static_cast<void>(ecsact::async::enqueue_execution_options(options));

	evc.set_remove_callback<async_test::ComponentUpdate>(
		[&](ecsact_entity_id id, const async_test::ComponentUpdate& component) {
			remove_happened = true;
			ASSERT_EQ(component.value_to_update, 6);
		}
	);

	auto start_tick = ecsact::async::get_current_tick();
	// Flush until we hit a maximum or get all our events we expect.
	while(init_happened && update_happened && remove_happened) {
		std::this_thread::yield();
		ecsact::async::flush_events(evc);
		auto current_tick = ecsact::async::get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10) //
			<< "Not all events happened before maximum was "
				 "reached\n"
			<< "  init = " << init_happened << "\n"
			<< "  update = " << update_happened << "\n"
			<< "  remove = " << remove_happened << "\n";
	}

	options.clear();
	evc.clear();
	ecsact::async::disconnect();
}

TEST(AsyncRef, TryMergeFailure) {
	using namespace std::chrono_literals;
	using std::chrono::duration_cast;

	static_cast<void>(ecsact::async::connect("good?delta_time=25"));

	ecsact::core::execution_options options{};

	auto my_needed_component = async_test::NeededComponent{};
	auto another_my_needed_component = async_test::NeededComponent{};

	options.create_entity()
		.add_component(&my_needed_component)
		.add_component(&another_my_needed_component);

	auto request_id = ecsact::async::enqueue_execution_options(options);

	bool wait = false;
	auto entity = ecsact_entity_id{};

	auto evc = ecsact::core::execution_events_collector{};

	evc.set_entity_created_callback(
		[&](ecsact_entity_id entity_id, ecsact_placeholder_entity_id) {
			entity = entity_id;
		}
	);

	auto async_evc = ecsact::async::async_events_collector{};

	async_evc.set_async_error_callback(
		[&](
			ecsact_async_error                 err,
			std::span<ecsact_async_request_id> request_ids
		) {
			ASSERT_EQ(err, ECSACT_ASYNC_ERR_EXECUTION_MERGE_FAILURE);

			auto error_id = request_ids[0];
			ASSERT_EQ(error_id, request_id);
			wait = true;
		}
	);

	flush_with_condition(wait, evc, async_evc);

	options.clear();
	evc.clear();

	ecsact::async::disconnect();
}

TEST(AsyncRef, CreateMultipleEntitiesAndDestroy) {
	using namespace std::chrono_literals;
	using std::chrono::duration_cast;

	static_cast<void>(ecsact::async::connect("good?delta_time=25"));

	auto options = ecsact::core::execution_options{};

	options.create_entity(static_cast<ecsact_placeholder_entity_id>(42));

	for(int i = 0; i < 10; i++) {
		static_cast<void>(ecsact::async::enqueue_execution_options(options));
	}

	struct entity_cb_info {
		std::array<int, 10>                          entity_request_ids;
		std::array<ecsact_entity_id, 10>             entity_ids;
		std::array<ecsact_placeholder_entity_id, 10> placeholder_ids;
	};

	int counter = 0;

	entity_cb_info entity_info = {};

	auto evc = ecsact::core::execution_events_collector{};

	evc.set_entity_created_callback(
		[&](
			ecsact_entity_id             entity_id,
			ecsact_placeholder_entity_id placeholder_entity_id
		) {
			entity_info.entity_request_ids[counter] = counter;
			entity_info.entity_ids[counter] = entity_id;
			entity_info.placeholder_ids[counter] = placeholder_entity_id;
			counter++;
		}
	);

	auto start_tick = ecsact::async::get_current_tick();
	while(counter < 10) {
		std::this_thread::yield();
		ecsact::async::flush_events(evc);
		auto current_tick = ecsact::async::get_current_tick();
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

	options.clear();
	evc.clear();

	auto entity_to_destroy = entity_info.entity_ids[5];
	bool wait = false;

	evc.set_entity_destroyed_callback([&](ecsact_entity_id entity_id) {
		assert(entity_id == entity_to_destroy);
		wait = true;
	});

	options.destroy_entity(entity_to_destroy);

	static_cast<void>(ecsact::async::enqueue_execution_options(options));

	flush_with_condition(wait, evc);

	ecsact::async::disconnect();
}

TEST(AsyncRef, TryAction) {
	using namespace std::chrono_literals;
	using std::chrono::duration_cast;

	static std::atomic_bool reached_system = false;

	static_cast<void>(ecsact::async::connect("good?delta_time=25"));

	// Declare components required for the action
	async_test::NeededComponent my_needed_component{};

	async_test::ComponentUpdate my_update_component{.value_to_update = 1};

	auto options = ecsact::core::execution_options{};

	options.create_entity()
		.add_component(&my_needed_component)
		.add_component(&my_update_component);

	static_cast<void>(ecsact::async::enqueue_execution_options(options));

	auto evc = ecsact::core::execution_events_collector{};

	static auto entity = ecsact_entity_id{};
	auto        wait = false;

	evc.set_entity_created_callback(
		[&](ecsact_entity_id entity_id, ecsact_placeholder_entity_id) {
			wait = true;
			entity = entity_id;
		}
	);

	flush_with_condition(wait, evc);

	options.clear();

	async_test::TryEntity my_try_entity;

	my_try_entity.my_entity = entity;

	// Declare an action which takes in a function as the implementation
	reached_system = false;
	ecsact_set_system_execution_impl(
		ecsact_id_cast<ecsact_system_like_id>(async_test::TryEntity::id),
		[](ecsact_system_execution_context* context) {
			async_test::TryEntity system_action{};
			ecsact_system_execution_context_action(context, &system_action);
			ASSERT_EQ(entity, system_action.my_entity);
			reached_system = true;
		}
	);

	// Push the action to execution options
	options.push_action(&my_try_entity);

	static_cast<void>(ecsact::async::enqueue_execution_options(options));

	auto start_tick = ecsact::async::get_current_tick();
	while(reached_system != true) {
		std::this_thread::yield();
		FLUSH_EVENTS_NEVER_ERROR(nullptr);
		auto current_tick = ecsact::async::get_current_tick();
		auto tick_diff = current_tick - start_tick;
		ASSERT_LT(tick_diff, 10);
	}

	ecsact::async::disconnect();
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
	static_cast<void>(ecsact::async::connect("good?delta_time=25"));

	auto entity = ecsact_entity_id{};
	auto wait = false;

	ecsact::core::execution_options options{};

	auto evc = ecsact::core::execution_events_collector{};

	evc.set_entity_created_callback(
		[&](ecsact_entity_id entity_id, ecsact_placeholder_entity_id) {
			entity = entity_id;
			wait = true;
		}
	);

	auto my_needed_component = async_test::NeededComponent{};

	options.create_entity().add_component(&my_needed_component);

	static_cast<void>(ecsact::async::enqueue_execution_options(options));
	;

	flush_with_condition(wait, evc);

	options.clear();
	evc.clear();

	// Remove component
	options.remove_component<async_test::NeededComponent>(entity);

	static_cast<void>(ecsact::async::enqueue_execution_options(options));

	wait = false;

	evc.set_remove_callback<async_test::NeededComponent>(
		[&](ecsact_entity_id, const async_test::NeededComponent& component) {
			ASSERT_EQ(component.id, async_test::NeededComponent::id);
			wait = true;
		}
	);

	flush_with_condition(wait, evc);

	options.clear();
	evc.clear();

	ecsact::async::disconnect();
}
