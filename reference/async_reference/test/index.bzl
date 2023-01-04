"""
"""

load("@rules_cc//cc:defs.bzl", "cc_test")
load("@rules_ecsact//ecsact:defs.bzl", "ecsact_codegen")

def ecsact_reference_async_module_test(name = None, srcs = [], deps = [], **kwargs):
    ecsact_codegen(
        name = "{}_support_generated_headers".format(name),
        srcs = ["@ecsact_runtime//reference/async_reference/test:async_test.ecsact"],
        plugins = [
            "@ecsact//codegen_plugins:cpp_header",
            "@ecsact//codegen_plugins:cpp_systems_header",
            "@ecsact//codegen_plugins:systems_header",
            "@ecsact//codegen_plugins:cpp_systems_source",
            "@ecsact_runtime//reference/serialize_reference/codegen",
        ],
    )

    cc_test(
        name = name,
        srcs = srcs + [
            ":{}_support_generated_headers".format(name),
            "@ecsact_runtime//reference/async_reference/test:async_ref_test.cc",
            "@ecsact_runtime//reference/async_reference:async_reference.cc",
            "@ecsact_runtime//reference/async_reference:async_reference.hh",
            "@ecsact_runtime//reference/async_reference:async.cc",
        ],
        deps = deps + [
            "@ecsact_runtime//:core",
            "@ecsact_runtime//:async",
            "@ecsact_runtime//:dynamic",
            "@ecsact_runtime//reference/async_reference:async",
            "@com_google_googletest//:gtest",
            "@com_google_googletest//:gtest_main",
            "@ecsact_runtime//reference/serialize_reference",
        ],
        **kwargs
    )
