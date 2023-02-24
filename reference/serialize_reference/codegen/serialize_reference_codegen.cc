#include <filesystem>
#include "ecsact/codegen_plugin.hh"
#include "ecsact/runtime/meta.hh"
#include "ecsact/lang-support/lang-cc.hh"
namespace fs = std::filesystem;

constexpr auto GENERATED_TOP = R"(
// GENERATED FILE - DO NOT EDIT

#include <map>
#include <optional>
#include "ecsact/runtime/common.h"
)";

constexpr auto GENERATED_VARIABLES = R"(
namespace ecsact_serialize_ref::detail {
	std::map<ecsact_component_id, int>& component_sizes();
	std::map<ecsact_action_id, int>& action_sizes();
}

namespace {
	class ref_serialize_pkg_init {
	public:
		ref_serialize_pkg_init();
		~ref_serialize_pkg_init() {}
	} ref_serialize_pkg_init;
}
)";

void ecsact_codegen_plugin(
	ecsact_package_id         pkg_id,
	ecsact_codegen_write_fn_t write
) {
	using ecsact::cc_lang_support::cpp_identifier;

	ecsact::codegen_plugin_context ctx{pkg_id, write};

	fs::path package_hh_path = ecsact::meta::package_file_path(pkg_id);
	package_hh_path.replace_extension(
		package_hh_path.extension().string() + ".hh"
	);

	ctx.write(GENERATED_TOP);
	ctx.write("#include \"", package_hh_path.filename().string(), "\"\n\n");
	ctx.write(GENERATED_VARIABLES);

	ctx.write("ref_serialize_pkg_init::ref_serialize_pkg_init() {");
	ctx.indentation += 1;
	ctx.write("\n");

	ctx.write(
		"auto& _comp_sizes = ecsact_serialize_ref::detail::component_sizes();\n",
		"auto& _act_sizes = ecsact_serialize_ref::detail::action_sizes();\n"
	);

	for(auto comp_id : ecsact::meta::get_component_ids(pkg_id)) {
		auto comp_name =
			ecsact::meta::decl_full_name(ecsact_id_cast<ecsact_decl_id>(comp_id));
		auto cpp_comp_name = cpp_identifier(comp_name);
		auto field_ids = ecsact::meta::get_field_ids(comp_id);

		if(field_ids.empty()) {
			ctx.write(
				"_comp_sizes[static_cast<ecsact_component_id>(",
				static_cast<int32_t>(comp_id),
				")] = 0;\n"
			);
		} else {
			ctx.write(
				"_comp_sizes[static_cast<ecsact_component_id>(",
				static_cast<int32_t>(comp_id),
				")] = sizeof(",
				cpp_comp_name,
				");\n"
			);
		}
	}

	for(auto act_id : ecsact::meta::get_action_ids(pkg_id)) {
		auto act_name =
			ecsact::meta::decl_full_name(ecsact_id_cast<ecsact_decl_id>(act_id));
		auto cpp_act_name = cpp_identifier(act_name);
		auto field_ids = ecsact::meta::get_field_ids(act_id);
		if(field_ids.empty()) {
			ctx.write(
				"_act_sizes[static_cast<ecsact_action_id>(",
				static_cast<int32_t>(act_id),
				")] = 0;\n"
			);
		} else {
			ctx.write(
				"_act_sizes[static_cast<ecsact_action_id>(",
				static_cast<int32_t>(act_id),
				")] = sizeof(",
				cpp_act_name,
				");\n"
			);
		}
	}
	ctx.indentation -= 1;

	ctx.write("\n}\n");
}
