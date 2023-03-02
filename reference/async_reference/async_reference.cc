#include <span>
#include <memory>
#include <ranges>
#include <algorithm>
#include <iterator>
#include <chrono>

#include "async_reference.hh"

using namespace ecsact::async_reference::detail;

struct parsed_connection_string {
	std::string                        host;
	std::map<std::string, std::string> options;
};

static auto str_subrange(std::string_view str, auto start, auto end) {
	return str.substr(start, end - start);
}

static auto parse_connection_string(std::string_view str)
	-> parsed_connection_string {
	auto result = parsed_connection_string{};

	auto options_start = str.find("?");

	if(options_start == std::string::npos) {
		// No options
		result.host = str;
		return result;
	}

	result.host = str.substr(0, options_start);

	auto amp_idx = options_start;

	while(amp_idx != std::string::npos) {
		auto next_amp_idx = str.find("&", amp_idx + 1);

		auto name_str = std::string{};
		auto value_str = std::string{};
		auto eq_idx = str.find("=", amp_idx);

		if(eq_idx >= next_amp_idx) {
			name_str = str_subrange(str, amp_idx + 1, next_amp_idx);
		} else {
			name_str = str_subrange(str, amp_idx + 1, eq_idx);
			value_str = str_subrange(str, eq_idx + 1, next_amp_idx);
		}

		result.options[name_str] = value_str;

		amp_idx = next_amp_idx;
	}

	return result;
}

void async_reference::connect(
	ecsact_async_request_id req_id,
	const char*             connection_string
) {
	std::string connect_str(connection_string);

	auto result = parse_connection_string(connect_str);

	if(result.options.contains("tick_rate")) {
		auto tick_str = result.options.at("tick_rate");

		int tick_int = std::stoi(tick_str);

		tick_rate = std::chrono::milliseconds(tick_int);
	}

	// The good and bad strings simulate the outcome of connections
	if(result.host != "good") {
		is_connected = false;
		is_connected_notified = false;
		return;
	}

	if(tick_rate.count() == 0) {
		types::async_error async_err{
			.error = ECSACT_ASYNC_INVALID_CONNECTION_STRING,
			.request_ids = {req_id},
		};

		async_callbacks.add(async_err);
		return;
	}

	registry_id = ecsact_create_registry("async_reference_impl_reg");
	is_connected = true;
	execute_systems();
}

void async_reference::enqueue_execution_options(
	ecsact_async_request_id         req_id,
	const ecsact_execution_options& options
) {
	if(is_connected == false && is_connected_notified == false) {
		types::async_error async_err{
			.error = ECSACT_ASYNC_ERR_PERMISSION_DENIED,
			.request_ids = {req_id},
		};

		is_connected_notified = true;
		async_callbacks.add(async_err);
		return;
	}

	auto cpp_options = util::c_to_cpp_execution_options(options);

	types::pending_execution_options pending_options{
		.request_id = req_id,
		.options = cpp_options,
	};

	tick_manager.add_pending_options(pending_options);
	return;
}

void async_reference::execute_systems() {
	execution_thread = std::thread([this] {
		using namespace std::chrono_literals;
		using clock = std::chrono::high_resolution_clock;
		using nanoseconds = std::chrono::nanoseconds;
		using std::chrono::duration_cast;

		nanoseconds execution_duration = {};

		while(is_connected == true) {
			auto async_err = tick_manager.validate_pending_options();

			const auto sleep_duration = tick_rate - execution_duration;

			std::this_thread::sleep_for(sleep_duration);

			if(async_err.error != ECSACT_ASYNC_OK) {
				async_callbacks.add(async_err);

				is_connected = false;
				break;
			}

			auto start = clock::now();

			auto cpp_options = tick_manager.move_and_increment_tick();

			// TODO(Kelwan): Add done callbacks so we can resolve all requests
			// https://github.com/ecsact-dev/ecsact_runtime/issues/102

			auto collector = exec_callbacks.get_collector();

			detail::c_execution_options c_exec_options;

			if(cpp_options) {
				util::cpp_to_c_execution_options(
					c_exec_options,
					*cpp_options,
					*registry_id
				);
			}
			auto options = c_exec_options.c();

			auto exec_lk = exec_callbacks.lock();
			auto systems_error =
				ecsact_execute_systems(*registry_id, 1, &options, collector);
			exec_lk.unlock();

			auto end = clock::now();
			execution_duration = duration_cast<nanoseconds>(end - start);

			if(systems_error != ECSACT_EXEC_SYS_OK) {
				async_callbacks.add(systems_error);
				is_connected = false;
				return;
			}
		}
	});
}

void async_reference::invoke_execution_events(
	const ecsact_execution_events_collector* execution_evc
) {
	if(registry_id) {
		exec_callbacks.invoke(execution_evc, *registry_id);
	}
}

int32_t async_reference::get_current_tick() {
	return tick_manager.get_current_tick();
}

void async_reference::disconnect() {
	is_connected = false;
	if(execution_thread.joinable()) {
		execution_thread.join();
	}
}
