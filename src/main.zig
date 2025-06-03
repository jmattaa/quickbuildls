const std = @import("std");

const Logger = @import("logger.zig").Logger;
const lsp = @import("lsp/lsp.zig");
const state = @import("state.zig");

var State: state.State = undefined;

const debug = @import("builtin").mode == .Debug;

const DEFAULT_LOGPATH = "/tmp/quickbuildls.log";

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer {
        if (debug) {
            const leaked = gpa.detectLeaks();
            if (leaked) {
                std.debug.print("LEAKED MEMORY!!\n", .{});
            }
        }
        _ = gpa.deinit();
    }
    const allocator = gpa.allocator();

    const log_path = std.process.getEnvVarOwned(
        allocator,
        "QUICKBUILDLS_LOG_PATH",
    ) catch DEFAULT_LOGPATH;
    defer {
        if (!std.mem.eql(u8, log_path, DEFAULT_LOGPATH))
            allocator.free(log_path);
    }

    const logger = try Logger.init(log_path);
    defer logger.deinit();

    State = try state.State.init();
    defer State.deinit(allocator);

    run(allocator, logger) catch |e| {
        try logger.write(allocator, "error: {s}", .{@errorName(e)});
        if (debug) try logger.write(allocator, "{any}", .{@errorReturnTrace()});
        return e;
    };
}

pub fn run(allocator: std.mem.Allocator, logger: Logger) !void {
    const stdin = std.io.getStdIn().reader();
    var buf = std.ArrayList(u8).init(allocator);
    defer buf.deinit();

    const chunk_size = 1024;
    while (true) {
        try buf.ensureTotalCapacity(buf.items.len + chunk_size);
        const read_into = buf.allocatedSlice()[buf.items.len .. buf.items.len +
            chunk_size];

        const n = try stdin.read(read_into);
        if (n == 0) break; // stdin closed

        try buf.resize(buf.items.len + n);

        while (true) {
            const msg = lsp.rpc.splitMsg(buf.items) catch |e| switch (e) {
                error.WaitForMoreData => break,
                else => return e,
            };

            try handleMsg(
                allocator,
                logger,
                msg.content,
                std.io.getStdOut().writer(),
            );

            // do some stuff with buf so we get the leftovers
            // and remove what we used
            const leftover = buf.items[msg.full_len..];
            try buf.resize(0);
            try buf.appendSlice(leftover);
        }
    }
}

pub fn handleMsg(
    allocator: std.mem.Allocator,
    logger: Logger,
    content: []const u8,
    out: anytype,
) !void {
    const decoded = try lsp.rpc.decode(lsp.rpc.PeekMsg, allocator, content);
    defer decoded.deinit();

    try logger.write(allocator, "{s}", .{decoded.value.method});
    if (std.mem.eql(u8, decoded.value.method, "initialize")) {
        const msg = try lsp.rpc.decode(lsp.rpc.LspBaseMsg, allocator, content);
        // initialize should always have an id
        const res = lsp.initialize.respond(msg.value.id.?);
        const encoded = try lsp.rpc.encode(allocator, res);
        defer allocator.free(encoded);
        try out.writeAll(encoded);
    } else if (std.mem.eql(u8, decoded.value.method, "textDocument/didOpen")) {
        const notif = try lsp.rpc.decode(
            lsp.textDocument.notifs.didOpenNotif,
            allocator,
            content,
        );
        defer notif.deinit();

        try State.setDocument(
            allocator,
            notif.value.params.textDocument.text,
        );
        State.version = notif.value.params.textDocument.version;
    } else if (std.mem.eql(u8, decoded.value.method, "textDocument/didChange")) {
        const notif = try lsp.rpc.decode(
            lsp.textDocument.notifs.didChangeNotif,
            allocator,
            content,
        );
        defer notif.deinit();

        // TODO should it be an array of documents in the state? even though
        // we only should keep track of one at a time????
        const contentChanges = notif.value.params.contentChanges;
        try State.setDocument(
            allocator,
            contentChanges[contentChanges.len - 1].text,
        );
        State.version = notif.value.params.textDocument.version;
    } else if (std.mem.eql(u8, decoded.value.method, "textDocument/hover")) {
        try lsp.respond(
            allocator,
            lsp.textDocument.hover,
            content,
            State,
            out,
        );
    } else if (std.mem.eql(u8, decoded.value.method, "textDocument/definition")) {
        try lsp.respond(
            allocator,
            lsp.textDocument.definition,
            content,
            State,
            out,
        );
    } else if (std.mem.eql(u8, decoded.value.method, "textDocument/completion")) {
        try lsp.respond(
            allocator,
            lsp.textDocument.completion,
            content,
            State,
            out,
        );
    }
}
