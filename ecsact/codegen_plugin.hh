#pragma once

#include <type_traits>
#include <string>
#include <string_view>
#include <cstring>
#include "ecsact/runtime/common.h"
#include "ecsact/codegen_plugin.h"

namespace ecsact {
	/**
	 * Helper type to give a more C++ friendly write function
	 * @example
	 *   void ecsact_codegen_plugin
	 *     ( ecsact_package_id          package_id
	 *     , ecsact_codegen_write_fn_t  write_fn
	 *     )
	 *   {
	 *     ecsact::codegen_plugin_context ctx{package_id, write_fn};
	 *     ctx.write("Hello, World!\n");
	 *   }
	 */
	struct codegen_plugin_context {
		const ecsact_package_id package_id;
		const ecsact_codegen_write_fn_t write_fn;

		template<typename T>
		void write(T&& arg) {
			using NoRefT = std::remove_cvref_t<T>;

			if constexpr(std::is_same_v<NoRefT, std::string_view>) {
				write_fn(arg.data(), static_cast<int32_t>(arg.size()));
			} else if constexpr(std::is_same_v<NoRefT, std::string>) {
				write_fn(arg.data(), static_cast<int32_t>(arg.size()));
			} else if constexpr(std::is_convertible_v<NoRefT, const char*>) {
				write_fn(arg, std::strlen(arg));
			} else {
				auto str = std::to_string(std::forward<T>(arg));
				write_fn(str.data(), static_cast<int32_t>(str.size()));
			}
		}

		template<typename... T>
		void write(T&&... args) {
			(write<T>(std::forward<T>(args)), ...);
		}
	};
}
