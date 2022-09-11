#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <optional>
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
	const std::string for_each_fn_macro_name =
		"FOR_EACH_ECSACT_"s + absl::AsciiStrToUpper(module_name) + "_API_FN"s;

	std::cout
		<< "[INFO] Module Name: " << module_name << "\n"
		<< "[INFO] Module Method Macro Name: " << module_fn_macro_name << "\n"
		<< "[INFO] For Each Module Method Macro Name: "
		<< for_each_fn_macro_name << "\n";

	std::vector<std::string> methods;

	std::fstream stream(
		module_header_path,
		std::ios_base::in | std::ios_base::app | std::ios_base::binary
	);
	std::string line;
	std::optional<std::streampos> for_each_begin;
	std::optional<std::streampos> for_each_end;
	while(std::getline(stream, line)) {
		if(line.starts_with(module_fn_macro_name + "(")) {
			std::vector<std::string> first_split =
				absl::StrSplit(line, absl::MaxSplits(',', 1));
			std::vector<std::string> second_split =
				absl::StrSplit(first_split[1], absl::MaxSplits(')', 1));

			auto method_name = absl::StripAsciiWhitespace(second_split[0]);
			methods.push_back(std::string(method_name));
		} else if(line.starts_with("//# BEGIN " + for_each_fn_macro_name)) {
			for_each_begin = stream.tellg();
		}
	}

	if(for_each_begin) {
		fs::resize_file(module_header_path, static_cast<int>(*for_each_begin));
		stream.clear();
		stream
			<< "#ifdef ECSACT_MSVC_TRADITIONAL\n"
			<< "#\tdefine " << for_each_fn_macro_name << "(fn, ...) "
			<< "ECSACT_MSVC_TRADITIONAL_ERROR()\n"
			<< "#else\n"
			<< "#\tdefine " << for_each_fn_macro_name << "(fn, ...)\\\n";

		auto methods_count = methods.size();
		for(int i=0; methods_count - 1 > i; ++i) {
			stream << "\t\tfn(" << methods[i] << ", __VA_ARGS__);\\\n";
		}
		if(methods_count > 0) {
			stream << "\t\tfn(" << methods[methods_count - 1] << ", __VA_ARGS__)\n";
		}
		stream
			<< "#endif\n\n"
			<< "#endif // ECSACT_RUNTIME_"
			<< absl::AsciiStrToUpper(module_name) << "_H" << "\n";
		stream.flush();
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

		std::cout << "[INFO] Checking " << arg.generic_string() << " ...\n";
		check_module_header(arg);
	}

	std::cout << "[INFO] Done\n";
	return exit_code;
}
