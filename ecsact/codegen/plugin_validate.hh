#pragma once

#include <vector>
#include <string>
#include <filesystem>

namespace ecsact::codegen {

enum class plugin_error {
	/**
	 * File not found
	 */
	not_found = 1,

	/**
	 * Plugin failed to load
	 */
	load_fail,

	/**
	 * File extension is not valid
	 */
	invalid_file_extension,

	/**
	 * File extension is valid, but unsupported in current platform
	 */
	unsupported_file_extension,

	/**
	 * Missing `ecsact_codegen_plugin` method
	 */
	missing_plugin_entry,

	/**
	 * Missing `ecsact_codegen_plugin_name` method
	 */
	missing_plugin_name,

	/**
	 * Plugin contains no symbols
	 */
	no_symbols,

	/**
	 * Missing dylib 'ecsact_dylib_set_fn_addr' method
	 */
	no_dylib,

	/**
	 * Plugin is utilising no meta module runtime methods
	 */
	no_meta_dylib_methods,
};

constexpr auto to_string(plugin_error err) {
	switch(err) {
		case plugin_error::not_found:
			return "File path does not exist";
		case plugin_error::load_fail:
			return "Failed to load plugin";
		case plugin_error::invalid_file_extension:
			return "Invalid file extension";
		case plugin_error::unsupported_file_extension:
			return "File extension unsupported on this platform";
		case plugin_error::missing_plugin_entry:
			return "Missing 'ecsact_codegen_plugin' method";
		case plugin_error::missing_plugin_name:
			return "Missing 'ecsact_codegen_plugin_name' method";
		case plugin_error::no_symbols:
			return "Plugin contains no exported symbols";
		case plugin_error::no_dylib:
			return "Missing 'ecsact_dylib_set_fn_addr' method";
		case plugin_error::no_meta_dylib_methods:
			return "No ecsact meta module function dylib pointers found";
	}

	return "TODO (Unhandled plugin error code)";
}

struct plugin_validate_result {
	std::vector<plugin_error> errors;
	std::vector<std::string>  unused_symbols;

	inline bool ok() const noexcept {
		return errors.empty();
	}

	inline operator bool() const noexcept {
		return ok();
	}
};

/**
 * Validate plugin
 */
plugin_validate_result plugin_validate(std::filesystem::path plugin_path);

} // namespace ecsact::codegen
