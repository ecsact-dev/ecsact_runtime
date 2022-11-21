"""
"""

load("@rules_cc//cc:defs.bzl", "cc_test")

def ecsact_reference_async_module_test(srcs = [], deps = [], **kwargs):
    cc_test(
        srcs = srcs + [
            "@ecsact_runtime//ecsact/reference/async_reference/test:async_ref_test.cc",
            "@ecsact_runtime//ecsact/reference/async_reference:async_reference.cc",
            "@ecsact_runtime//ecsact/reference/async_reference:async_reference.hh",
            "@ecsact_runtime//ecsact/reference/async_reference:async.cc",
        ],
        deps = deps + [
            "@ecsact_runtime//:core",
            "@ecsact_runtime//:async",
            "@ecsact_runtime//ecsact/reference/async_reference:async",
            "@com_google_googletest//:gtest",
            "@com_google_googletest//:gtest_main",
        ],
        **kwargs
    )
