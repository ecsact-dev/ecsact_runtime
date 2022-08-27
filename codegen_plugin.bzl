"""
Exports `cc_ecsact_codegen_plugin`
"""

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_test")
load("//bazel:copts.bzl", _copts = "copts")

def cc_ecsact_codegen_plugin(name = None, srcs = [], deps = [], defines = [], no_validate_test = False, **kwargs):
    """
    Create ecsact codegen plugin with C++

    Args:
        name: Passed to underling cc_binary
        srcs: Passed to underling cc_binary
        deps: Passed to underling cc_binary
        defines: Passed to underling cc_binary
        no_validate_test: Don't create plugin validation test (not recommended)
        **kwargs: Passed to underling cc_binary
    """
    cc_binary(
        name = name,
        srcs = srcs + ["@ecsact_runtime//dylib:dylib.cc"],
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

    if not no_validate_test:
        cc_test(
            name = "{}__validate".format(name),
            srcs = ["@ecsact_runtime//tests:ecsact_codegen_plugin_test.cc"],
            copts = _copts,
            data = [":{}".format(name)],
            args = ["$(location :{})".format(name)],
            deps = ["@ecsact_runtime//:codegen_plugin_validate"],
        )
