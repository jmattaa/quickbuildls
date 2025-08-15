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
    _ = state;

    var edits = std.ArrayList(lsputils.textEdit).init(allocator);
    defer edits.deinit();

    try edits.append(.{
        .range = .{
            .start = .{ .line = 0, .character = 0 },
            .end = .{ .line = 0, .character = 0 },
        },
        .newText = "\n",
    });

    return .{
        .jsonrpc = "2.0",
        .id = req.id,
        .result = try edits.toOwnedSlice(),
    };
}

pub fn deinit(r: response, allocator: std.mem.Allocator) void {
    if (r.result) |result| {
        allocator.free(result);
    }
}
