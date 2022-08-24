#pragma once

#include <string>
#include <string_view>
#include <type_traits>
#include <cstring>
#include "ecsact/codegen/write.h"

namespace ecsact::codegen {
	template<typename T>
	void write(T&& arg) {
		using NoRefT = std::remove_cvref_t<T>;

		if constexpr(std::is_same_v<NoRefT, std::string_view>) {
			ecsact_codegen_write(arg.data(), static_cast<int32_t>(arg.size()));
		} else if constexpr(std::is_same_v<NoRefT, std::string>) {
			ecsact_codegen_write(arg.data(), static_cast<int32_t>(arg.size()));
		} else if constexpr(std::is_convertible_v<NoRefT, const char*>) {
			ecsact_codegen_write(arg, std::strlen(arg));
		} else {
			auto str = std::to_string(std::forward<T>(arg));
			ecsact_codegen_write(str.data(), static_cast<int32_t>(str.size()));
		}
	}

	template<typename... T>
	void write(T&&... args) {
		(write<T>(std::forward<T>(args)), ...);
	}
}
