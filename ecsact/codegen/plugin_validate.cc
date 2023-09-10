#include "ecsact/codegen/plugin_validate.hh"

#include <boost/dll/shared_library.hpp>
#include <boost/dll/library_info.hpp>
#include "ecsact/runtime/meta.h"
#include "ecsact/runtime/dylib.h"

namespace fs = std::filesystem;
namespace dll = boost::dll;
using ecsact::codegen::plugin_validate_result;

plugin_validate_result ecsact::codegen::plugin_validate(fs::path plugin_path) {
	const auto platform_plugin_suffix = dll::shared_library::suffix().string();
	plugin_validate_result result;

	if(auto ext = plugin_path.extension(); !ext.empty()) {
		if(ext != platform_plugin_suffix) {
			if(ext == ".so" || ext == ".dll") {
				result.errors.push_back(plugin_error::unsupported_file_extension);
			} else {
				result.errors.push_back(plugin_error::invalid_file_extension);
			}
			return result;
		}
	} else {
		plugin_path.replace_extension(platform_plugin_suffix);
	}

	if(!fs::exists(plugin_path)) {
		result.errors.push_back(plugin_error::not_found);
		return result;
	}

	std::error_code     ec;
	dll::shared_library plugin;
	plugin.load(plugin_path.string(), ec);
	if(ec) {
		result.errors.push_back(plugin_error::load_fail);
		return result;
	}

	dll::library_info plugin_info(plugin.location());
	auto              symbols = plugin_info.symbols();

	if(symbols.empty()) {
		result.errors.push_back(plugin_error::no_symbols);
	}

	if(!plugin.has("ecsact_codegen_plugin")) {
		result.errors.push_back(plugin_error::missing_plugin_entry);
	}

	if(!plugin.has("ecsact_codegen_plugin_name")) {
		result.errors.push_back(plugin_error::missing_plugin_name);
	}

	if(!plugin.has("ecsact_dylib_set_fn_addr")) {
		result.errors.push_back(plugin_error::no_dylib);
	}

	if(plugin.has("ecsact_dylib_has_fn")) {
		bool has_ecsact_meta_fns = false;
		bool is_missing_any_meta_fns = false;

		auto has_fn = plugin.get<bool(const char*)>("ecsact_dylib_has_fn");

		auto check_meta_fn = [&](const char* fn_name) {
			if(has_fn(fn_name)) {
				has_ecsact_meta_fns = true;
			} else {
				is_missing_any_meta_fns = true;
			}
		};

#define CALL_CHECK_META_FN(fn_name, unused) check_meta_fn(#fn_name)
		FOR_EACH_ECSACT_META_API_FN(CALL_CHECK_META_FN);
#undef CALL_CHECK_META_FN

		if(!has_ecsact_meta_fns) {
			result.errors.push_back(plugin_error::no_meta_dylib_methods);
		}
	}

	for(auto symbol : symbols) {
		if(symbol == "ecsact_codegen_plugin") {
			continue;
		}
		if(symbol == "ecsact_codegen_plugin_name") {
			continue;
		}
		if(symbol == "ecsact_dylib_set_fn_addr") {
			continue;
		}
		if(symbol == "ecsact_dylib_has_fn") {
			continue;
		}

		result.unused_symbols.push_back(symbol);
	}

	return result;
}
