"""Project-wide wrappers around rules_cc: same warnings everywhere, gtest linked into tests."""

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

WARNINGS = [
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Wshadow",
    "-Werror",
]

def mm_cc_library(name, copts = [], **kwargs):
    cc_library(name = name, copts = WARNINGS + copts, **kwargs)

def mm_cc_binary(name, copts = [], **kwargs):
    cc_binary(name = name, copts = WARNINGS + copts, **kwargs)

def mm_cc_test(name, copts = [], deps = [], size = "small", **kwargs):
    cc_test(
        name = name,
        size = size,
        copts = WARNINGS + copts,
        deps = deps + ["@googletest//:gtest_main"],
        **kwargs
    )
