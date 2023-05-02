"""
Exports `cc_ecsact_codegen_plugin`
"""

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_test")
load("//bazel:copts.bzl", _copts = "copts")
load(
    "@ecsact_codegen//:codegen_plugin.bzl",
    _EcsactCodegenPluginInfo = "EcsactCodegenPluginInfo",
    _cc_ecsact_codegen_plugin = "cc_ecsact_codegen_plugin",
    _ecsact_codegen_plugin = "ecsact_codegen_plugin",
)

EcsactCodegenPluginInfo = _EcsactCodegenPluginInfo

def _deprecated_notice():
    print("codegen_plugin.bzl from ecsact_runtime is deprecated. Please use the ecsact_codegen repository instead.")

def ecsact_codegen_plugin(**kwargs):
    _deprecated_notice()
    return _ecsact_codegen_plugin(**kwargs)

def cc_ecsact_codegen_plugin(**kwargs):
    _deprecated_notice()
    return _cc_ecsact_codegen_plugin(**kwargs)
