const std = @import("std");
const lsp = @import("lsp/lsp.zig");
const Logger = @import("logger.zig").Logger;

const Test = struct { jsonrpc: []const u8, method: []const u8, id: u32 };

pub fn main() !void {
    const allocator = std.heap.c_allocator;

    const logger = try Logger.init(
        "/Users/jonathan/dev/quickbuildls/quickbuildls.log",
    );
    defer logger.deinit();

    const msg = try lsp.rpc.encode(allocator, Test{
        .jsonrpc = "2.0",
        .method = "initialize",
        .id = 1,
    });
    try logger.write(msg);

    const decoded = try lsp.rpc.decode(allocator, msg);
    defer decoded.deinit();
    try logger.write(decoded.value.jsonrpc);
    try logger.write(decoded.value.method);
}
