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
            try logger.write("Failed to read message: {}", .{e});
            continue;
        };

        try handleMsg(c.value, logger);
    }
}

pub fn handleMsg(msg: lsp.rpc.LspBaseMsg, logger: Logger) !void {
    try logger.write("{s}", .{msg.method});
}
