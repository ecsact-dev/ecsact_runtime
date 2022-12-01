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

TEST(AsyncRef, ConnectBad) {
	auto connect_req_id = ecsact_async_connect("bad");

	auto async_err_cb = //
		[](
			ecsact_async_error      async_err,
			ecsact_async_request_id request_id,
			void*                   callback_user_data
		) { ASSERT_EQ(async_err, ECSACT_ASYNC_ERR_PERMISSION_DENIED); };

	ecsact_async_events_collector async_evc{
		.async_error_callback = async_err_cb,
		.async_error_callback_user_data = nullptr,
		.system_error_callback = nullptr,
		.system_error_callback_user_data = nullptr,
	};
	ecsact_async_flush_events(nullptr, &async_evc);
}

TEST(AsyncRef, Disconnect) {
	auto connect_req_id = ecsact_async_connect("good");

	ecsact_async_disconnect();
}

TEST(AsyncRef, AddComponentSucceed) {
	auto connect_req_id = ecsact_async_connect("good");

	async_test::ComponentAddRemove my_component{};
	auto        add_comp_id = async_test::ComponentAddRemove::id;
	const void* component_data = &my_component;

	ecsact_component component{
		.component_id = add_comp_id,
		.component_data = component_data,
	};

	auto entity = ecsact_async_create_entity();

	std::array<ecsact_component, 1> add_components = {component};
	std::array<ecsact_entity_id, 1> add_components_entities = {entity};

	ecsact_execution_options options{
		.add_components_length = 1,
		.add_components_entities = add_components_entities.data(),
		.add_components = add_components.data(),
		.update_components_length = 0,
		.update_components_entities = nullptr,
		.update_components = nullptr,
		.remove_components_length = 0,
		.remove_components_entities = nullptr,
		.remove_components = nullptr,
		.actions_length = 0,
		.actions = nullptr,
	};

	ecsact_async_enqueue_execution_options(options);

	auto sys_error_cb = //
		[](ecsact_execute_systems_error execute_err, void* callback_user_data) {
			ASSERT_EQ(execute_err, ECSACT_EXEC_SYS_OK);
		};

	ecsact_async_events_collector async_evc{
		.async_error_callback = nullptr,
		.async_error_callback_user_data = nullptr,
		.system_error_callback = sys_error_cb,
		.system_error_callback_user_data = nullptr,
	};

	ecsact_async_flush_events(nullptr, &async_evc);

	ecsact_async_disconnect();
}

TEST(AsyncRef, ActionSucceed) {
	auto connect_req_id = ecsact_async_connect("good");

	async_test::NeededComponent my_component{};
	auto                        add_comp_id = async_test::NeededComponent::id;
	const void*                 component_data = &my_component;

	ecsact_component component{
		.component_id = add_comp_id,
		.component_data = component_data,
	};

	auto entity = ecsact_async_create_entity();

	std::array<ecsact_component, 1> components = {component};
	std::array<ecsact_entity_id, 1> entities = {entity};

	ecsact_execution_options add_component_options{
		.add_components_length = 1,
		.add_components_entities = entities.data(),
		.add_components = components.data(),
		.update_components_length = 0,
		.update_components_entities = nullptr,
		.update_components = nullptr,
		.remove_components_length = 0,
		.remove_components_entities = nullptr,
		.remove_components = nullptr,
		.actions_length = 0,
		.actions = nullptr,
	};

	ecsact_async_enqueue_execution_options(add_component_options);

	ecsact_async_flush_events(nullptr, nullptr);

	async_test::AddComponent my_action{};
	auto                     action_id = async_test::AddComponent::id;
	const void*              action_data = &my_action;

	ecsact_action action{
		.action_id = action_id,
		.action_data = action_data,
	};

	std::array<ecsact_action, 1> actions = {action};

	ecsact_execution_options add_action_options{
		.add_components_length = 0,
		.add_components_entities = nullptr,
		.add_components = nullptr,
		.update_components_length = 0,
		.update_components_entities = nullptr,
		.update_components = nullptr,
		.remove_components_length = 0,
		.remove_components_entities = nullptr,
		.remove_components = nullptr,
		.actions_length = 1,
		.actions = actions.data(),
	};

	auto sys_error_cb = //
		[](ecsact_execute_systems_error execute_err, void* callback_user_data) {
			ASSERT_EQ(execute_err, ECSACT_EXEC_SYS_OK);
		};

	ecsact_async_events_collector async_evc{
		.async_error_callback = nullptr,
		.async_error_callback_user_data = nullptr,
		.system_error_callback = sys_error_cb,
		.system_error_callback_user_data = nullptr,
	};

	ecsact_async_enqueue_execution_options(add_action_options);

	ecsact_async_flush_events(nullptr, &async_evc);

	ecsact_async_disconnect();
}

TEST(AsyncReg, ActionEntitySucceed) {
	auto connect_req_id = ecsact_async_connect("good");

	async_test::NeededComponent my_component{};
	auto                        add_comp_id = async_test::NeededComponent::id;
	const void*                 component_data = &my_component;

	ecsact_component component{
		.component_id = add_comp_id,
		.component_data = component_data,
	};

	auto entity = ecsact_async_create_entity();

	std::array<ecsact_component, 1> components = {component};
	std::array<ecsact_entity_id, 1> entities = {entity};

	ecsact_execution_options add_component_options{
		.add_components_length = 1,
		.add_components_entities = entities.data(),
		.add_components = components.data(),
		.update_components_length = 0,
		.update_components_entities = nullptr,
		.update_components = nullptr,
		.remove_components_length = 0,
		.remove_components_entities = nullptr,
		.remove_components = nullptr,
		.actions_length = 0,
		.actions = nullptr,
	};

	ecsact_async_enqueue_execution_options(add_component_options);

	ecsact_async_flush_events(nullptr, nullptr);

	async_test::TryEntity my_action{};
	auto                  action_id = async_test::TryEntity::id;
	my_action.my_entity = entity;
	const void* action_data = &my_action;

	ecsact_action action{
		.action_id = action_id,
		.action_data = action_data,
	};

	std::array<ecsact_action, 1> actions = {action};

	ecsact_execution_options add_action_options{
		.add_components_length = 0,
		.add_components_entities = nullptr,
		.add_components = nullptr,
		.update_components_length = 0,
		.update_components_entities = nullptr,
		.update_components = nullptr,
		.remove_components_length = 0,
		.remove_components_entities = nullptr,
		.remove_components = nullptr,
		.actions_length = 1,
		.actions = actions.data(),
	};

	auto sys_error_cb = //
		[](ecsact_execute_systems_error execute_err, void* callback_user_data) {
			ASSERT_EQ(execute_err, ECSACT_EXEC_SYS_OK);
		};

	ecsact_async_events_collector async_evc{
		.async_error_callback = nullptr,
		.async_error_callback_user_data = nullptr,
		.system_error_callback = sys_error_cb,
		.system_error_callback_user_data = nullptr,
	};

	ecsact_async_enqueue_execution_options(add_action_options);

	ecsact_async_flush_events(nullptr, &async_evc);

	ecsact_async_disconnect();
}

TEST(AsyncRef, ActionEntityFail) {
	auto connect_req_id = ecsact_async_connect("good");

	async_test::NeededComponent my_component{};
	auto                        add_comp_id = async_test::NeededComponent::id;
	const void*                 component_data = &my_component;

	ecsact_component component{
		.component_id = add_comp_id,
		.component_data = component_data,
	};

	auto entity = ecsact_async_create_entity();

	std::array<ecsact_component, 1> components = {component};
	std::array<ecsact_entity_id, 1> entities = {entity};

	ecsact_execution_options add_component_options{
		.add_components_length = 1,
		.add_components_entities = entities.data(),
		.add_components = components.data(),
		.update_components_length = 0,
		.update_components_entities = nullptr,
		.update_components = nullptr,
		.remove_components_length = 0,
		.remove_components_entities = nullptr,
		.remove_components = nullptr,
		.actions_length = 0,
		.actions = nullptr,
	};

	ecsact_async_enqueue_execution_options(add_component_options);

	ecsact_async_flush_events(nullptr, nullptr);

	auto bad_entity = ecsact_entity_id{25};

	async_test::TryEntity my_action{};
	auto                  action_id = async_test::TryEntity::id;
	my_action.my_entity = bad_entity;
	const void* action_data = &my_action;

	ecsact_action action{
		.action_id = action_id,
		.action_data = action_data,
	};

	std::array<ecsact_action, 1> actions = {action};

	ecsact_execution_options add_action_options{
		.add_components_length = 0,
		.add_components_entities = nullptr,
		.add_components = nullptr,
		.update_components_length = 0,
		.update_components_entities = nullptr,
		.update_components = nullptr,
		.remove_components_length = 0,
		.remove_components_entities = nullptr,
		.remove_components = nullptr,
		.actions_length = 1,
		.actions = actions.data(),
	};

	static bool wait = false;

	auto sys_error_cb = //
		[](ecsact_execute_systems_error execute_err, void* callback_user_data) {
			wait = true;
			ASSERT_EQ(execute_err, ECSACT_EXEC_SYS_OK);
		};

	ecsact_async_events_collector async_evc{
		.async_error_callback = nullptr,
		.async_error_callback_user_data = nullptr,
		.system_error_callback = sys_error_cb,
		.system_error_callback_user_data = nullptr,
	};

	ecsact_async_enqueue_execution_options(add_action_options);

	ecsact_async_flush_events(nullptr, &async_evc);

	while(!wait) {
	}
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

	auto entity = ecsact_async_create_entity();

	std::array<ecsact_component, 2> components = {
		needed_component,
		update_component};
	std::array<ecsact_entity_id, 1> components_entities = {entity};

	ecsact_execution_options add_options{
		.add_components_length = components.size(),
		.add_components_entities = components_entities.data(),
		.add_components = components.data(),
		.update_components_length = 0,
		.update_components_entities = nullptr,
		.update_components = nullptr,
		.remove_components_length = 1,
		.remove_components_entities = nullptr,
		.remove_components = nullptr,
		.actions_length = 0,
		.actions = nullptr,
	};

	ecsact_async_enqueue_execution_options(add_options);

	ecsact_async_flush_events(nullptr, nullptr);

	my_update_component.value_to_update += 5;

	std::array<ecsact_component, 1> update_components = {update_component};

	ecsact_execution_options update_options{
		.add_components_length = 0,
		.add_components_entities = nullptr,
		.add_components = nullptr,
		.update_components_length = update_components.size(),
		.update_components_entities = components_entities.data(),
		.update_components = update_components.data(),
		.remove_components_length = 0,
		.remove_components_entities = nullptr,
		.remove_components = nullptr,
		.actions_length = 0,
		.actions = nullptr,
	};

	ecsact_async_enqueue_execution_options(add_options);

	ecsact_async_flush_events(nullptr, nullptr);

	std::array<ecsact_component_id, 1> remove_components = {update_comp_id};

	ecsact_execution_options remove_options{
		.add_components_length = 0,
		.add_components_entities = nullptr,
		.add_components = nullptr,
		.update_components_length = 0,
		.update_components_entities = nullptr,
		.update_components = nullptr,
		.remove_components_length = 1,
		.remove_components_entities = components_entities.data(),
		.remove_components = remove_components.data(),
		.actions_length = 0,
		.actions = nullptr,
	};

	ecsact_async_enqueue_execution_options(remove_options);

	ecsact_async_flush_events(nullptr, nullptr);

	ecsact_async_disconnect();
}
