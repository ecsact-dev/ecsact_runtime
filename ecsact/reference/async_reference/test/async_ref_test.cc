#include "gtest/gtest.h"

#include "ecsact/runtime/async.h"

TEST(AsyncRef, ConnectBad) {
	auto connect_req_id = ecsact_async_connect("bad");

	auto async_err_cb = //
		[](
			ecsact_async_error      async_err,
			ecsact_async_request_id request_id,
			void*                   callback_user_data
		) {

		};

	ecsact_async_events_collector async_evc{
		.async_error_callback = async_err_cb,
		.async_error_callback_user_data = nullptr,
		.system_error_callback = nullptr,
		.system_error_callback_user_data = nullptr,
	};
	ecsact_async_flush_events(nullptr, &async_evc);
}
