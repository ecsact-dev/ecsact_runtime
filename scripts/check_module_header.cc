#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <optional>
#include <cstdlib>
#include <string_view>
#include <unordered_set>
#include <boost/process.hpp>
#include "absl/strings/ascii.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;
namespace fs = std::filesystem;
namespace bp = boost::process;

struct gh_action_group {
	gh_action_group(std::string group_name) {
		std::cout << "::group::" << group_name << "\n";
	}

	~gh_action_group() {
		std::cout << "::endgroup::\n";
	}
};

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
		<< "::debug::Module Name: " << module_name << "\n"
		<< "::debug::Module Method Macro Name: " << module_fn_macro_name << "\n"
		<< "::debug::For Each Module Method Macro Name: " << for_each_fn_macro_name
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

	std::cout << "::debug::Found " << methods.size() << " methods\n";

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

auto find_method_definition_line( //
	fs::path    header_file,
	std::string function_name
) -> std::optional<int> {
	auto stream = std::fstream(
		header_file,
		std::ios_base::in | std::ios_base::app | std::ios_base::binary
	);
	auto line = std::string{};
	auto line_num = 1;
	while(std::getline(stream, line)) {
		auto idx = line.find(function_name + ")");
		if(idx != std::string::npos) {
			return line_num;
		}

		line_num += 1;
	}

	return std::nullopt;
}

auto report_needs_for_each_added( //
	fs::path    header_file,
	std::string added_fn
) -> void {
	auto line = find_method_definition_line(header_file, added_fn);
	if(line) {
		std::cout //
			<< "::error file=" << fs::relative(header_file).string()
			<< ",line=" << *line << "::" << added_fn
			<< " needs to be added to the FOR_EACH macro\n";
	}
}

auto report_needs_for_each_removal( //
	fs::path    header_file,
	std::string removed_fn
) -> void {
	auto line = find_method_definition_line(header_file, removed_fn);
	if(line) {
		std::cout //
			<< "::error file=" << fs::relative(header_file).string()
			<< ",line=" << *line << "::" << removed_fn
			<< " needs to be removed from the FOR_EACH macro\n";
	}
}

int main(int argc, char* argv[]) {
	using namespace std::string_literals;

	auto runfiles = Runfiles::Create(argv[0]);
	auto bwd = std::getenv("BUILD_WORKSPACE_DIRECTORY");

	if(bwd == nullptr) {
		std::cerr //
			<< "[ERROR] BUILD_WORKSPACE_DIRECTORY environment variable not "
				 "found\n";
		return 1;
	} else {
		fs::current_path(bwd);
	}

	auto clang_format = "clang-format"s;

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

	int  exit_code = 0;
	auto header_files = std::vector<fs::path>{};
	header_files.reserve(argc - 1);

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

		header_files.push_back(arg);
	}

	if(exit_code == 0 && header_files.empty()) {
		std::cout //
			<< "::debug::No specified headers. Checking all module headers.\n";

		auto non_module_headers = std::unordered_set{
			"common.h"s,
			"definitions.h"s,
			"dylib.h"s,
		};

		// Every C header in `ecsact/runtime` is a module header
		for(auto entry : fs::directory_iterator(fs::path{bwd} / "ecsact/runtime")) {
			if(non_module_headers.contains(entry.path().filename())) {
				continue;
			}

			// Only C headers are module headers
			if(entry.path().extension() != ".h") {
				continue;
			}

			header_files.push_back(entry.path());
		}
	}

	for(auto header_file : header_files) {
		auto relative_header_path = fs::relative(header_file);
		auto group = gh_action_group{
			"Ecsact Module Header: "s + relative_header_path.string()};

		std::cout << "::debug::Checking " << header_file.generic_string()
							<< " ...\n";
		check_module_header(header_file);

		std::string format_str = clang_format + " -i " + header_file.string();
		std::cout << "::debug::Running " << format_str << " ...\n";
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
			bp::args({"diff"s, "-U0", header_file.string()}),
			bp::std_out > diff_output
		);

		auto line = std::string{};
		auto found_line_info = false;
		while(!found_line_info && std::getline(diff_output, line)) {
			if(!line.starts_with("@@")) {
				continue;
			}

			found_line_info = true;
			auto line_start = line.find('-');
			auto line_end = line.find(' ', line_start);
			auto line_num = line.substr(line_start + 1, line_end - line_start - 1);

			std::cout //
				<< "::error file=" << relative_header_path.string()
				<< ",line=" << line_num << ",title=Out of date FOR_EACH macro for "
				<< relative_header_path.string()
				<< "::When adding or removing an Ecsact function from the API "
				<< "headers you must also update the FOR_EACH_ macro.\n";
			exit_code += 1;
		}

		auto removed_fns = std::unordered_set<std::string>{};
		auto added_fns = std::unordered_set<std::string>{};
		while(std::getline(diff_output, line)) {
			auto fn_start_idx = line.find("fn(");

			if(fn_start_idx == std::string::npos) {
				continue;
			}

			auto fn_name = line.substr(
				fn_start_idx + 3,
				line.find(',', fn_start_idx + 3) - fn_start_idx - 3
			);

			if(line.starts_with('-')) {
				removed_fns.insert(fn_name);
			} else if(line.starts_with('+')) {
				added_fns.insert(fn_name);
			}
		}

		for(auto added_fn : added_fns) {
			if(removed_fns.contains(added_fn)) {
				continue;
			}

			report_needs_for_each_added(header_file, added_fn);
		}

		for(auto removed_fn : removed_fns) {
			if(added_fns.contains(removed_fn)) {
				continue;
			}

			report_needs_for_each_removal(header_file, removed_fn);
		}
	}

	std::cout << "::debug::Done\n";
	return exit_code;
}
