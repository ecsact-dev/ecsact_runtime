copts = select({
    "@bazel_tools//tools/cpp:msvc": ["/std:c++latest", "/Zc:preprocessor", "/permissive-"],
    "//conditions:default": ["-std=c++20"],
})
