const std = @import("std");
const buildutils = @import("buildutils.zig");

pub fn build(b: *std.Build) !void {
    const allocator = std.heap.page_allocator;
    const target = b.standardTargetOptions(.{});

    const optimize = b.standardOptimizeOption(.{});

    const exe_mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });

    const exe = b.addExecutable(.{
        .name = "quickbuildls",
        .root_module = exe_mod,
    });

    // C++ & C
    // -----------------------------------------------------------------------
    exe.linkLibCpp();
    exe.linkLibC();

    const cflags = [_][]const u8{};
    const cppflags = [_][]const u8{"--std=c++20"};

    try buildutils.addSourceRecursive(
        allocator,
        exe,
        b,
        "src/cpp",
        &cflags,
        &cppflags,
    );

    // -----------------------------------------------------------------------

    b.installArtifact(exe);

    const exe_check = b.addExecutable(.{
        .name = "check",
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });
    const check = b.step("check", "Check if it compiles");
    check.dependOn(&exe_check.step);

    const run_cmd = b.addRunArtifact(exe);

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    const exe_unit_tests = b.addTest(.{
        .root_module = exe_mod,
    });

    const run_exe_unit_tests = b.addRunArtifact(exe_unit_tests);

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&run_exe_unit_tests.step);
}
