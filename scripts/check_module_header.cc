#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string_view>
#include "absl/strings/ascii.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"

namespace fs = std::filesystem;

void check_module_header(fs::path module_header_path) {
	using namespace std::string_literals;

	const std::string module_name =
		module_header_path.filename().replace_extension("").string();
	const std::string module_fn_macro_name =
		"ECSACT_"s + absl::AsciiStrToUpper(module_name) + "_API_FN";

	std::cout
		<< "[INFO] Module Name: " << module_name << "\n"
		<< "[INFO] Module Method Macro Name: " << module_fn_macro_name << "\n";

	std::ifstream stream(module_header_path);
	std::string line;
	while(std::getline(stream, line)) {
		if(line.starts_with(module_fn_macro_name + "(")) {
			std::vector<std::string> first_split =
				absl::StrSplit(line, absl::MaxSplits(',', 1));
			std::vector<std::string> second_split =
				absl::StrSplit(first_split[1], absl::MaxSplits(')', 1));

			auto method_name = absl::StripAsciiWhitespace(second_split[0]);
			std::cout << "Found method: " << method_name << "\n";
		}
	}
}

int main(int argc, char* argv[]) {
	auto bwd = std::getenv("BUILD_WORKSPACE_DIRECTORY");
	if(bwd == nullptr) {
		std::cerr << "BUILD_WORKSPACE_DIRECTORY environment variable not found\n";
		return 1;
	}

	int exit_code = 0;
	for(int i=1; argc > i; ++i) {
		fs::path arg(argv[i]);
		if(!arg.is_absolute()) {
			arg = bwd / arg;
		}
		if(!fs::exists(arg)) {
			std::cerr << "[ERROR] Cannot find file " << arg.generic_string() << "\n";
			exit_code += 1;
			continue;
		}

		std::cout << "[INFO] Checking " << arg.generic_string() << "...\n";
		check_module_header(arg);
	}

	std::cout << "[INFO] Done\n";
	return exit_code;
}
