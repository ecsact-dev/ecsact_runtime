#pragma once

#include <string>
#include <span>
#include <concepts>
#include <utility>
#include "ecsact/runtime/core.hh"
#include "ecsact/runtime/async.h"

namespace ecsact::async {

template<
	template<class R, class... Args> typename CallbackContainer = std::function>
class async_events_collector {
public:
	using async_error_callback_t = CallbackContainer<
		void(ecsact_async_error, std::span<ecsact_async_request_id>)>;

	using async_requests_done_callback_t =
		CallbackContainer<void(std::span<ecsact_async_request_id>)>;

	using system_error_callback_t =
		CallbackContainer<void(ecsact_execute_systems_error)>;

	/**
	 * Set async error callback. If callback is already set it will be
	 * overwritten.
	 */
	auto set_async_error_callback( //
		async_error_callback_t callback
	) -> async_events_collector& {
		_async_error_cb = std::move(callback);
		return *this;
	}

	/**
	 * Set async error callback. If callback is already set it will be
	 * overwritten.
	 */
	auto set_system_error_callback( //
		system_error_callback_t callback
	) -> async_events_collector& {
		_system_error_cb = std::move(callback);
		return *this;
	}

	/**
	 * Set async error callback. If callback is already set it will be
	 * overwritten.
	 */
	auto set_async_requests_done_callback( //
		async_requests_done_callback_t callback
	) -> async_events_collector& {
		_async_requests_done_cb = std::move(callback);
		return *this;
	}

	auto c() const -> const ecsact_async_events_collector {
		auto evc = ecsact_async_events_collector{};
		auto user_data =
			static_cast<void*>(const_cast<async_events_collector*>(this));

		if(_async_error_cb.has_value()) {
			evc.async_error_callback = &async_events_collector::async_error_callback;
			evc.async_error_callback_user_data = user_data;
		}

		if(_system_error_cb.has_value()) {
			evc.system_error_callback =
				&async_events_collector::system_error_callback;
			evc.system_error_callback_user_data = user_data;
		}

		if(_async_requests_done_cb.has_value()) {
			evc.async_request_done_callback =
				&async_events_collector::async_requests_done_callback;
			evc.async_request_done_callback_user_data = user_data;
		}

		return evc;
	}

	auto clear() -> void {
		_async_error_cb = std::nullopt;
		_system_error_cb = std::nullopt;
	}

	auto empty() const -> bool {
		return _async_error_cb.empty() && _system_error_cb.empty();
	}

private:
	std::optional<async_error_callback_t>         _async_error_cb;
	std::optional<system_error_callback_t>        _system_error_cb;
	std::optional<async_requests_done_callback_t> _async_requests_done_cb;

	static void async_error_callback(
		ecsact_async_error       async_err,
		int                      request_ids_length,
		ecsact_async_request_id* request_ids,
		void*                    callback_user_data
	) {
		auto self = static_cast<async_events_collector*>(callback_user_data);

		if(self->_async_error_cb.has_value()) {
			auto request_ids_span = std::span{
				request_ids,
				static_cast<size_t>(request_ids_length),
			};
			self->_async_error_cb.value()(async_err, request_ids_span);
		}
	}

	static void system_error_callback(
		ecsact_execute_systems_error err,
		void*                        callback_user_data
	) {
		auto self = static_cast<async_events_collector*>(callback_user_data);

		if(self->_system_error_cb.has_value()) {
			self->_system_error_cb.value()(err);
		}
	}

	static void async_requests_done_callback(
		int                      request_ids_length,
		ecsact_async_request_id* request_ids,
		void*                    callback_user_data
	) {
		auto self = static_cast<async_events_collector*>(callback_user_data);

		if(self->_async_requests_done_cb.has_value()) {
			auto request_ids_span = std::span{
				request_ids,
				static_cast<size_t>(request_ids_length),
			};
			self->_async_requests_done_cb.value()(request_ids_span);
		}
	}
};

[[nodiscard]] ECSACT_ALWAYS_INLINE auto connect(
	const std::string& connection_string
) -> ecsact_async_request_id {
	return ecsact_async_connect(connection_string.c_str());
}

ECSACT_ALWAYS_INLINE auto disconnect() -> void {
	ecsact_async_disconnect();
}

[[nodiscard]] ECSACT_ALWAYS_INLINE auto get_current_tick() -> int32_t {
	return ecsact_async_get_current_tick();
}

[[nodiscard]] ECSACT_ALWAYS_INLINE auto enqueue_execution_options(
	ecsact::core::execution_options& options
) -> ecsact_async_request_id {
	return ecsact_async_enqueue_execution_options(options.c());
}

ECSACT_ALWAYS_INLINE auto flush_events() -> void {
	ecsact_async_flush_events(nullptr, nullptr);
}

template<typename ExecutionEventsCollector>
	requires(std::convertible_to<
						decltype(std::declval<ExecutionEventsCollector>().c()),
						const ecsact_execution_events_collector>)
ECSACT_ALWAYS_INLINE auto flush_events( //
	ExecutionEventsCollector&& evc
) -> void {
	const ecsact_execution_events_collector evc_c = evc.c();
	ecsact_async_flush_events(&evc_c, nullptr);
}

template<typename AsyncEventsCollector>
	requires(std::convertible_to<
						decltype(std::declval<AsyncEventsCollector>().c()),
						const ecsact_async_events_collector>)
ECSACT_ALWAYS_INLINE auto flush_events( //
	AsyncEventsCollector&& async_evc
) -> void {
	const ecsact_async_events_collector async_evc_c = async_evc.c();
	ecsact_async_flush_events(nullptr, &async_evc_c);
}

template<typename ExecutionEventsCollector, typename AsyncEventsCollector>
ECSACT_ALWAYS_INLINE auto flush_events(
	ExecutionEventsCollector&& evc,
	AsyncEventsCollector&&     async_evc
) -> void {
	const ecsact_execution_events_collector evc_c = evc.c();
	const ecsact_async_events_collector     async_evc_c = async_evc.c();
	ecsact_async_flush_events(&evc_c, &async_evc_c);
}
} // namespace ecsact::async
