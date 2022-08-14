copts = select({
    "@bazel_tools//tools/cpp:msvc": [],
    "//conditions:default": [],
})
