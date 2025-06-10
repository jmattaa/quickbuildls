const std = @import("std");

const State = @import("../../state.zig").State;
const computils = @import("computils.zig");

pub const request = struct {
    jsonrpc: []const u8,
    method: []const u8,
    id: u32,
    params: struct {
        textDocument: struct {
            uri: []const u8,
        },
        position: struct {
            line: u8,
            character: u8,
        },
    },
};

pub const response = struct {
    jsonrpc: []const u8,
    id: u32,
    result: ?struct {
        isIncomplete: bool,
        items: ?[]computils.completionitem = null,
    } = null,
};

pub fn respond(allocator: std.mem.Allocator, req: request, state: State) !response {
    const document = state.document orelse return .{
        // we don have nothing to give
        .jsonrpc = "2.0",
        .id = req.id,
    };

    var nitems: u8 = 0;
    const items = try computils.getCompletions(
        allocator,
        document,
        req.params.position.line,
        req.params.position.character,
        &nitems,
    );

    return .{
        .jsonrpc = "2.0",
        .id = req.id,
        .result = .{
            .isIncomplete = false,
            .items = items,
        },
    };
}

pub fn deinit(r: response, allocator: std.mem.Allocator) void {
    if (r.result) |res| {
        if (res.items) |items| {
            for (items) |i| {
                allocator.free(i.label);
                if (i.detail) |d|
                    allocator.free(d);
                if (i.documentation) |doc| {
                    if (doc.kind) |k|
                        allocator.free(k);
                    if (doc.value) |v|
                        allocator.free(v);
                }
            }
            allocator.free(items);
        }
    }
}
