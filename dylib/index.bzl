"""
Exports `cc_ecsact_dylib`
"""

load("@rules_cc//cc:defs.bzl", "cc_library")
load("//:index.bzl", "ecsact_runtime_modules")

def cc_ecsact_dylib(name = None, srcs = [], ecsact_modules = [], deps = [], defines = [], **kwargs):
    """
    Create `cc_library` that exposes ecsact module function pointers

    Args:
        name: Passed to underlying cc_library
        srcs: Passed to underlying cc_library
        ecsact_modules: List of modules that should be exposed in the dylib
        deps: Passed to underlying cc_library
        defines: Passed to underlying cc_library
        **kwargs: Passed to underlying cc_library
    """

    for ecsact_module in ecsact_modules:
        if not ecsact_module in ecsact_runtime_modules:
            fail("Invalid ecsact module '{}'\nAllowed values: {}".format(ecsact_module, ",".join(ecsact_runtime_modules)))

    cc_library(
        name = name,
        deps = deps + ["@ecsact_runtime//:dylib", "@ecsact_runtime//dylib:util"] + ["@ecsact_runtime//:{}".format(m) for m in ecsact_modules],
        srcs = srcs + ["@ecsact_runtime//dylib:dylib.cc"],
        local_defines = ["ECSACT_{}_API_EXPORT".format(m.upper()) for m in ecsact_modules],
        defines =  #
            defines +
            ["ECSACT_{}_API_LOAD_AT_RUNTIME".format(m.upper()) for m in ecsact_modules],
        **kwargs
    )
