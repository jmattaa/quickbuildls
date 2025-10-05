const std = @import("std");

const State = @import("../../state.zig").State;
const lsputils = @import("../lsputils.zig");

pub const request = struct {
    jsonrpc: []const u8,
    method: []const u8,
    id: u32,
    params: struct {
        textDocument: struct {
            uri: []const u8,
        },
        options: struct {
            tabSize: u32,
            insertSpaces: bool,
        },
    },
};

pub const response = struct {
    jsonrpc: []const u8,
    id: u32,
    result: ?[]lsputils.textEdit = null,
};

pub fn respond(
    allocator: std.mem.Allocator,
    req: request,
    state: State,
) !response {
    var edits = std.array_list.Managed(lsputils.textEdit).init(allocator);
    defer edits.deinit();

    var lines = std.mem.splitAny(u8, state.document.?, "\n");
    var lineslen: u32 = 0;
    const formatted = try formatFile(allocator, &lines, &lineslen);
    try edits.append(.{
        .range = .{
            .start = .{ .line = 0, .character = 0 },
            .end = .{ .line = lineslen - 1, .character = 0 },
        },
        .newText = formatted,
    });

    return .{
        .jsonrpc = "2.0",
        .id = req.id,
        .result = try edits.toOwnedSlice(),
    };
}

pub fn deinit(r: response, allocator: std.mem.Allocator) void {
    if (r.result) |edits| {
        for (edits) |edit| {
            allocator.free(edit.newText);
        }
        allocator.free(edits);
    }
}

fn formatFile(
    allocator: std.mem.Allocator,
    lines: *std.mem.SplitIterator(u8, .any),
    len: *u32,
) ![]const u8 {
    var out = std.array_list.Managed(u8).init(allocator);
    defer out.deinit();

    var indent_level: usize = 0;
    var first = true;

    while (lines.next()) |line| {
        const trimmed = std.mem.trim(u8, line, " \t");

        if (std.mem.startsWith(u8, trimmed, "}")) {
            if (indent_level > 0) indent_level -= 1;
        }

        if (!first) try out.appendSlice("\n");
        first = false;

        for (0..indent_level) |_| try out.appendSlice("    ");
        try out.appendSlice(trimmed);

        if (std.mem.endsWith(u8, trimmed, "{")) {
            indent_level += 1;
        }

        len.* += 1;
    }

    return try out.toOwnedSlice();
}
