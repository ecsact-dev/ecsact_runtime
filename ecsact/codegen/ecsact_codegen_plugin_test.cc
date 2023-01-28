#include <filesystem>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include "ecsact/codegen/plugin_validate.hh"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;
namespace fs = std::filesystem;

#define RED_TEXT(text) "\033[31m" text "\033[0m"
#define GREY_TEXT(text) "\033[90m" text "\033[0m"
#define GREEN_TEXT(text) "\033[32m" text "\033[0m"
#define YELLOW_TEXT(text) "\033[33m" text "\033[0m"

int main(int argc, char* argv[]) {
	if(argc <= 1) {
		std::cerr << "No arguments given.\n";
		return 1;
	}

	auto runfiles = Runfiles::Create(argv[0]);
	auto bwd = std::getenv("BUILD_WORKSPACE_DIRECTORY");

	int invalid_plugins = 0;
	std::cerr << GREY_TEXT("Checking ") << argc - 1 << GREY_TEXT(" plugin(s)\n");

	for(int i = 1; argc > i; ++i) {
		fs::path plugin_path = argv[i];
		if(runfiles && !fs::exists(plugin_path)) {
			plugin_path = runfiles->Rlocation(argv[i]);
		}
		if(bwd && !fs::exists(plugin_path)) {
			plugin_path = std::string(bwd) / fs::path(argv[i]);
		}

		std::cerr //
			<< GREY_TEXT("Validating plugin ") << plugin_path.string()
			<< GREY_TEXT(" ...");
		auto result = ecsact::codegen::plugin_validate(plugin_path);
		if(result.ok()) {
			std::cerr << "\b\b\b" GREEN_TEXT("VALID") "\n";
		} else {
			invalid_plugins += 1;
			std::cerr << "\b\b\b" RED_TEXT("INVALID") "\n";
		}

		for(auto err : result.errors) {
			std::cerr << RED_TEXT(" ERR ") << to_string(err) << "\n";
		}

		for(auto symbol : result.unused_symbols) {
			std::cerr << YELLOW_TEXT(" UNUSED ") << symbol << "\n";
		}
	}

	return invalid_plugins > 0 ? 1 : 0;
}
