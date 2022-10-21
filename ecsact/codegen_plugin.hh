#pragma once

#include <cassert>
#include <type_traits>
#include <string>
#include <string_view>
#include <cstring>
#include <iterator>
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
	const ecsact_package_id         package_id;
	const ecsact_codegen_write_fn_t write_fn;
	int                             indentation = 0;

	std::string get_indent_str() {
		return std::string(indentation, '\t');
	}

	void write_(const char* str_data, int32_t str_data_len) {
		assert(indentation >= 0);

		if(indentation <= 0) {
			write_fn(str_data, str_data_len);
		} else {
			std::string_view str(str_data, str_data_len);
			auto             indent_str = get_indent_str();
			auto             nl_idx = str.find('\n');
			while(nl_idx != std::string_view::npos) {
				write_fn(str.data(), nl_idx + 1);
				write_fn(indent_str.data(), indent_str.size());
				str =
					std::string_view(str.data() + nl_idx + 1, str.size() - nl_idx - 1);
				nl_idx = str.find('\n');
			}

			if(!str.empty()) {
				write_fn(str.data(), static_cast<int32_t>(str.size()));
			}
		}
	}

	template<typename T>
	void write(T&& arg) {
		using NoRefT = std::remove_cvref_t<T>;

		if constexpr(std::is_same_v<NoRefT, std::string_view>) {
			write_(arg.data(), static_cast<int32_t>(arg.size()));
		} else if constexpr(std::is_same_v<NoRefT, std::string>) {
			write_(arg.data(), static_cast<int32_t>(arg.size()));
		} else if constexpr(std::is_convertible_v<NoRefT, const char*>) {
			write_(arg, std::strlen(arg));
		} else {
			auto str = std::to_string(std::forward<T>(arg));
			write_(str.data(), static_cast<int32_t>(str.size()));
		}
	}

	template<typename... T>
	void write(T&&... args) {
		(write<T>(std::forward<T>(args)), ...);
	}

	void write_each(auto&& delim, auto&& range, auto&& callback) {
		auto begin = std::begin(range);
		auto end = std::end(range);
		if(begin != end) {
			callback(*begin);
			for(auto itr = std::next(begin); itr != end; ++itr) {
				write(delim);
				callback(*itr);
			}
		}
	}
};

} // namespace ecsact
