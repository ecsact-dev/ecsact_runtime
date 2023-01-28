"""
Exports `cc_ecsact_codegen_plugin`
"""

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_test")
load("//bazel:copts.bzl", _copts = "copts")

EcsactCodegenPluginInfo = provider(
    doc = "",
    fields = {
        "output_extension": "Plugin name. Also used as output file extension. This must match the extension specified by the plugin.",
        "plugin": "Path to plugin or name of builtin plugin",
        "data": "Files needed at runtime",
    },
)

def _ecsact_codegen_plugin(ctx):
    return [
        EcsactCodegenPluginInfo(
            output_extension = ctx.attr.output_extension,
            plugin = ctx.attr.plugin,
            data = [],
        ),
    ]

ecsact_codegen_plugin = rule(
    implementation = _ecsact_codegen_plugin,
    doc = "Bazel info necessary for ecsact codegen plugin to be used with `ecsact_codegen`. Default plugins are available at `@ecsact//codegen_plugins:*`.",
    attrs = {
        "output_extension": attr.string(mandatory = True, doc = "Plugin name. Also used as output file extension. This must match the extension specified by the plugin."),
        "plugin": attr.string(mandatory = True, doc = "Path to plugin or name of builtin plugin."),
    },
)

def _cc_ecsact_codegen_plugin_impl(ctx):
    plugin = None
    files = ctx.attr.cc_binary[DefaultInfo].files

    for file in files.to_list():
        if file.extension == "so":
            plugin = file
        if file.extension == "dll":
            plugin = file

    return [
        EcsactCodegenPluginInfo(
            output_extension = ctx.attr.output_extension,
            plugin = plugin,
            data = [plugin],
        ),
    ]

_cc_ecsact_codegen_plugin = rule(
    implementation = _cc_ecsact_codegen_plugin_impl,
    attrs = {
        "cc_binary": attr.label(mandatory = True),
        "output_extension": attr.string(mandatory = True),
    },
)

_generated_src = """
#include "ecsact/codegen_plugin.h"

const char* ecsact_codegen_plugin_name() {{
	return "{output_extension}";
}}
"""

def _cc_ecsact_codegen_plugin_src_impl(ctx):
    output_cc_src = ctx.actions.declare_file("plugin_name.cc".format(ctx.attr.name))
    ctx.actions.write(
        output = output_cc_src,
        content = _generated_src.format(output_extension = ctx.attr.output_extension),
    )

    return [
        DefaultInfo(files = depset([output_cc_src])),
    ]

_cc_ecsact_codegen_plugin_src = rule(
    implementation = _cc_ecsact_codegen_plugin_src_impl,
    attrs = {
        "output_extension": attr.string(mandatory = True),
    },
)

def cc_ecsact_codegen_plugin(name = None, srcs = [], deps = [], defines = [], no_validate_test = False, output_extension = None, **kwargs):
    """Create ecsact codegen plugin with C++

    NOTE: ecsact_codegen_plugin_name() is automatically generated for you based
          on the `output_extension` attribute.

    Args:
        name: Passed to underling cc_binary
        srcs: Passed to underling cc_binary
        deps: Passed to underling cc_binary
        defines: Passed to underling cc_binary
        output_extension: File extension the plugin writes to
        no_validate_test: Don't create plugin validation test (not recommended)
        **kwargs: Passed to underling cc_binary
    """
    cc_binary(
        name = "{}_bin".format(name),
        srcs = srcs + [
            "@ecsact_runtime//dylib:dylib.cc",
            ":{}__pn".format(name),
        ],
        deps = deps + [
            "@ecsact_runtime//:dylib",
            "@ecsact_runtime//dylib:meta",
            "@ecsact_runtime//:codegen_plugin",
            "@ecsact_runtime//:codegen_plugin_cc",
        ],
        defines = defines + ["ECSACT_META_API_LOAD_AT_RUNTIME"],
        linkshared = True,
        **kwargs
    )

    _cc_ecsact_codegen_plugin_src(
        name = "{}__pn".format(name),
        output_extension = output_extension,
    )

    _cc_ecsact_codegen_plugin(
        name = name,
        cc_binary = ":{}_bin".format(name),
        output_extension = output_extension,
    )

    if not no_validate_test:
        cc_test(
            name = "{}__validate".format(name),
            srcs = ["@ecsact_runtime//:ecsact/codegen/ecsact_codegen_plugin_test.cc"],
            args = ["$(rootpath :{}_bin)".format(name)],
            copts = _copts,
            data = [":{}_bin".format(name)],
            deps = [
                "@bazel_tools//tools/cpp/runfiles",
                "@ecsact_runtime//:codegen_plugin_validate",
            ],
        )
