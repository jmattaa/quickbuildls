const std = @import("std");

const State = @import("../../state.zig").State;

pub const request = struct {
    jsonrpc: []const u8,
    method: []const u8,
    id: u32,
    params: struct {
        textDocument: struct {
            uri: []const u8,
        },
        position: struct {
            line: c_int,
            character: c_int,
        },
    },
};

pub const response = struct {
    jsonrpc: []const u8,
    id: u32,
    result: ?[]const struct {
        uri: []const u8,
        range: struct {
            start: struct {
                line: c_int,
                character: c_int,
            },
            end: struct {
                line: c_int,
                character: c_int,
            },
        },
    },
};

pub fn respond(allocator: std.mem.Allocator, req: request, state: State) !response {
    _ = state;
    _ = allocator;
    return .{
        .jsonrpc = "2.0",
        .id = req.id,
        .result = null,
    };
}

