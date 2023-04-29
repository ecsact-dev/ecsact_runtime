#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <optional>
#include <cstdlib>
#include <string_view>
#include <boost/process.hpp>
#include "absl/strings/ascii.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;
namespace fs = std::filesystem;
namespace bp = boost::process;

void remove_empties(std::vector<std::string>& str_list) {
	for(auto itr = str_list.begin(); itr != str_list.end();) {
		*itr = absl::StripAsciiWhitespace(*itr);
		if(itr->empty()) {
			itr = str_list.erase(itr);
		} else {
			++itr;
		}
	}
}

void check_module_header(fs::path module_header_path) {
	using namespace std::string_literals;

	const std::string module_name =
		module_header_path.filename().replace_extension("").string();
	const std::string module_fn_macro_name =
		"ECSACT_"s + absl::AsciiStrToUpper(module_name) + "_API_FN";
	const std::string for_each_fn_macro_name =
		"FOR_EACH_ECSACT_"s + absl::AsciiStrToUpper(module_name) + "_API_FN"s;

	std::cout //
		<< "[INFO] Module Name: " << module_name << "\n"
		<< "[INFO] Module Method Macro Name: " << module_fn_macro_name << "\n"
		<< "[INFO] For Each Module Method Macro Name: " << for_each_fn_macro_name
		<< "\n";

	std::vector<std::string> methods;

	std::fstream stream(
		module_header_path,
		std::ios_base::in | std::ios_base::app | std::ios_base::binary
	);
	std::string                   line;
	std::optional<std::streampos> for_each_begin;
	while(std::getline(stream, line)) {
		if(line.starts_with(module_fn_macro_name + "(")) {
			std::vector<std::string> first_split =
				absl::StrSplit(line, absl::MaxSplits(',', 1));
			std::vector<std::string> second_split;

			remove_empties(first_split);

			if(first_split.size() > 1) {
				second_split = absl::StrSplit(first_split[1], absl::MaxSplits(')', 1));
				remove_empties(second_split);
			}

			while(first_split.size() < 2 || second_split.empty()) {
				std::string next_line;
				if(!std::getline(stream, next_line)) {
					break;
				}

				line += "\n" + next_line;
				first_split = absl::StrSplit(line, absl::MaxSplits(',', 1));
				remove_empties(first_split);

				if(first_split.size() > 1 && second_split.empty()) {
					second_split =
						absl::StrSplit(first_split[1], absl::MaxSplits(')', 1));
					remove_empties(second_split);
				}
			}
			auto method_name = absl::StripAsciiWhitespace(second_split[0]);
			if(!method_name.starts_with("ecsact_")) {
				throw std::runtime_error(
					"Invalid method name "s + std::string(method_name)
				);
			}
			methods.push_back(std::string(method_name));
		} else if(line.starts_with("// # BEGIN " + for_each_fn_macro_name)) {
			for_each_begin = stream.tellg();
		}
	}

	std::cout << "[INFO] Found " << methods.size() << " methods\n";

	if(for_each_begin) {
		fs::resize_file(module_header_path, static_cast<int>(*for_each_begin));
		stream.clear();
		stream //
			<< "#ifdef ECSACT_MSVC_TRADITIONAL\n"
			<< "#\tdefine " << for_each_fn_macro_name << "(fn, ...) "
			<< "ECSACT_MSVC_TRADITIONAL_ERROR()\n"
			<< "#else\n"
			<< "#\tdefine " << for_each_fn_macro_name << "(fn, ...)\\\n";

		auto methods_count = methods.size();
		for(int i = 0; methods_count - 1 > i; ++i) {
			stream << "\t\tfn(" << methods[i] << ", __VA_ARGS__);\\\n";
		}
		if(methods_count > 0) {
			stream << "\t\tfn(" << methods[methods_count - 1] << ", __VA_ARGS__)\n";
		}
		stream //
			<< "#endif\n\n"
			<< "#endif // ECSACT_RUNTIME_" << absl::AsciiStrToUpper(module_name)
			<< "_H"
			<< "\n";
		stream.flush();
	}
}

int main(int argc, char* argv[]) {
	using namespace std::string_literals;

	auto runfiles = Runfiles::Create(argv[0]);
	auto bwd = std::getenv("BUILD_WORKSPACE_DIRECTORY");

	if(bwd == nullptr) {
		std::cerr //
			<< "[WARN] BUILD_WORKSPACE_DIRECTORY environment variable not "
				 "found\n";
	} else {
		fs::current_path(bwd);
	}

	std::string clang_format = "clang-format";

	for(int i = 1; argc > i; ++i) {
		std::string arg(argv[i]);
		if(arg == "--clang-format") {
			if(argc > i + 1) {
				clang_format = std::string(argv[i + 1]);
				break;
			} else {
				std::cerr << "--clang-format missing value\n";
				return 1;
			}
		}
	}

	if(runfiles != nullptr && clang_format == "clang-format") {
		clang_format = runfiles->Rlocation("llvm_toolchain/clang-format");
	}

	int exit_code = 0;
	for(int i = 1; argc > i; ++i) {
		if(argv[i][0] == '-') {
			i += 1;
			continue;
		}

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

		std::string format_str = clang_format + " -i " + arg.string();
		std::cout << "[INFO] Running " << format_str << " ...\n";
		auto format_exit_code = std::system(format_str.c_str());

		if(format_exit_code != 0) {
			std::cerr //
				<< clang_format << " exited with exit code " << format_exit_code
				<< "\n";
			return format_exit_code;
		}
		
		auto diff_output = bp::ipstream{};
		auto diff_proc = bp::child(
			bp::exe(bp::search_path("git")),
			bp::args({"diff"s, "-U0", arg.string()}),
			bp::std_out > diff_output
		);

		auto line = std::string{};
		while(std::getline(diff_output, line)) {
			if(!line.starts_with("@@")) continue;
			
			auto line_start = line.find('-');
			auto line_end = line.find(' ', line_start);
			auto line_num = line.substr(line_start + 1, line_end - line_start - 1);

			std::cout //
				<< "::error file=" << arg.string() << ",line=" << line_num
				<< "::When adding or removing an Ecsact function from the API "
				<< "headers you must also update the FOR_EACH_ macro.\n";
		}
	}

	std::cout << "[INFO] Done\n";
	return exit_code;
}
