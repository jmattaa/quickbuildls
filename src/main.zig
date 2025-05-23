const std = @import("std");

const Logger = @import("logger.zig").Logger;
const lsp = @import("lsp/lsp.zig");

pub fn main() !void {
    const allocator = std.heap.c_allocator;

    const logger = try Logger.init(
        "/Users/jonathan/dev/quickbuildls/quickbuildls.log",
    );
    defer logger.deinit();

    const stdin = std.io.getStdIn().reader();

    while (true) {
        const c = lsp.rpc.readMessage(allocator, stdin) catch |e| {
            try logger.write(allocator, "Failed to read message: {any}", .{e});
            continue;
        };
        defer c.buf.deinit();

        try handleMsg(allocator, c.parsed.value, std.io.getStdOut().writer());
        try logger.write(allocator, "{s}", .{c.parsed.value.method}); // only for debugging
    }
}

pub fn handleMsg(
    allocator: std.mem.Allocator,
    msg: lsp.rpc.LspBaseMsg,
    out: anytype,
) !void {
    if (std.mem.eql(u8, msg.method, "initialize")) {
        // initialize should always have an id
        const res = lsp.initialize.respond(msg.id.?);
        const encoded = try lsp.rpc.encode(allocator, res);
        try out.writeAll(encoded);
    }
}
