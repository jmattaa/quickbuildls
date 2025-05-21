const std = @import("std");
const lsp = @import("lsp/lsp.zig");

const Test = struct {jsonrpc: []const u8, method: []const u8, id: u32};

pub fn main() !void {
    const allocator = std.heap.c_allocator;

    const msg = try lsp.rpc.encode(allocator, Test{
        .jsonrpc = "2.0",
        .method = "initialize",
        .id = 1,
    });
    std.debug.print("{s}\n", .{msg});

    const decoded = try lsp.rpc.decode(allocator, msg);
    defer decoded.deinit();
    std.debug.print("{s}\n", .{decoded.value.jsonrpc});
    std.debug.print("{s}\n", .{decoded.value.method});
    std.debug.print("{d}\n", .{decoded.value.id});
}
