load("@bazel_skylib//lib:selects.bzl", "selects")

# Ecsact repositories currently only support clang, cl, and emscripten
copts = selects.with_or({
    (Label("//bazel:compiler_emscripten")): [
        "-std=c++20",
    ],
    ("@rules_cc//cc/compiler:clang"): [
        "-std=c++20",
        "-fexperimental-library",
    ],
    ("@rules_cc//cc/compiler:msvc-cl", "@rules_cc//cc/compiler:clang-cl"): [
        "/std:c++20",
        "/permissive-",
        "/Zc:preprocessor",
    ],
})
