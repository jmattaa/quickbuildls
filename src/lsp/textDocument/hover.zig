const std = @import("std");

const State = @import("../../state.zig").State;

const chover = @cImport({
    @cInclude("hover.h");
});

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
    // is null if we can't hover on the ting
    result: ?struct {
        contents: struct {
            kind: []const u8,
            value: []const u8,
        },
        range: ?struct {
            start: struct {
                line: c_int,
                character: c_int,
            },
            end: struct {
                line: c_int,
                character: c_int,
            },
        } = null,
    } = null,
};

pub fn respond(
    allocator: std.mem.Allocator,
    req: request,
    state: State,
) !response {
    const document = state.document orelse return .{
        // we don have nothing to give
        .jsonrpc = "2.0",
        .id = req.id,
    };

    const c_src = try allocator.dupeZ(u8, document); // null-terminated
    defer allocator.free(c_src);

    const md: [*c]const u8 = chover.get_hover_md(
        c_src,
        req.params.position.line,
        req.params.position.character,
    );

    if (md != null) {
        return .{
            .jsonrpc = "2.0",
            .id = req.id,
            .result = .{
                .contents = .{
                    .kind = "markdown",
                    .value = std.mem.span(md), // cstr to zig str
                },
            },
        };
    }

    return .{
        .jsonrpc = "2.0",
        .id = req.id,
    };
}

pub fn deinit(r: response, _: std.mem.Allocator) void {
    if (r.result == null) return;
    chover.hover_md_free(@ptrCast(r.result.?.contents.value));
}
