load("@bazel_skylib//lib:selects.bzl", "selects")

copts = selects.with_or({
    ("//conditions:default", "@rules_cc//cc/compiler:clang", "@rules_cc//cc/compiler:gcc", "@rules_cc//cc/compiler:mingw-gcc"): [
        "-std=c++23",
        "-fexperimental-library",
    ],
    ("@rules_cc//cc/compiler:msvc-cl", "@rules_cc//cc/compiler:clang-cl"): [
        "/std:c++latest",
        "/permissive-",
        "/Zc:preprocessor",
    ],
})
