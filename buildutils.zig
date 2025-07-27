const std = @import("std");

pub fn addSourceRecursive(
    allocator: std.mem.Allocator,
    exe: *std.Build.Step.Compile,
    b: *std.Build,
    dirname: []const u8,
    cflags: []const []const u8,
    cppflags: []const []const u8,
) !void {
    var dir = std.fs.cwd().openDir(dirname, .{ .iterate = true }) catch
        return error.DirNotFound;
    defer dir.close();

    exe.addIncludePath(.{ .src_path = .{ .owner = b, .sub_path = dirname } });

    var dirIterator = dir.iterate();
    while (try dirIterator.next()) |entry| {
        switch (entry.kind) {
            .directory => {
                const subdir = try std.fs.path.join(allocator, &.{
                    dirname,
                    entry.name,
                });
                defer allocator.free(subdir);
                try addSourceRecursive(
                    allocator,
                    exe,
                    b,
                    subdir,
                    cflags,
                    cppflags,
                );
                continue;
            },
            .file => {
                const is_cpp = std.mem.endsWith(u8, entry.name, ".cpp");
                const is_c = std.mem.endsWith(u8, entry.name, ".c");

                // js skip any files that ain't sourcefiles
                if (!is_cpp and !is_c) continue;
                const flags = if (is_cpp) cppflags else cflags;
                const path = try std.fs.path.join(allocator, &.{
                    dirname,
                    entry.name,
                });
                defer allocator.free(path);
                exe.addCSourceFile(.{
                    .flags = flags,
                    .language = if (is_cpp) .cpp else .c,
                    .file = .{
                        .src_path = .{
                            .owner = b,
                            .sub_path = path,
                        },
                    },
                });
            },
            else => continue,
        }
    }
}
