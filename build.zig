const std = @import("std");

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

    // C++
    // -----------------------------------------------------------------------
    var dir = try std.fs.cwd().openDir("src/cpp", .{ .iterate = true });
    defer dir.close();

    exe.addIncludePath(.{ .src_path = .{ .owner = b, .sub_path = "src/cpp" } });
    exe.linkLibCpp();

    const cppflags = [_][]const u8{"--std=c++17"};

    var dirIterator = dir.iterate();
    while (try dirIterator.next()) |entry| {
        const path = try std.fs.path.join(
            allocator,
            &.{ "src/cpp", entry.name },
        );
        defer allocator.free(path);
        if (std.mem.endsWith(u8, entry.name, ".cpp")) {
            exe.addCSourceFile(
                .{
                    .flags = &cppflags,
                    .language = .cpp,
                    .file = .{
                        .src_path = .{
                            .owner = b,
                            .sub_path = path,
                        },
                    },
                },
            );
        }
    }
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
