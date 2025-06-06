const std = @import("std");

const cdefinition = @cImport({
    @cInclude("definition.h");
});

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
    result: ?struct {
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
    } = null,
};

pub fn respond(allocator: std.mem.Allocator, req: request, state: State) !response {
    const document = state.document orelse return .{
        // we don have nothing to give
        .jsonrpc = "2.0",
        .id = req.id,
    };

    const c_src = try allocator.dupeZ(u8, document); // null-terminated
    defer allocator.free(c_src);

    var tol: c_int = -1;
    var toc: c_int = -1;

    cdefinition.get_definition(
        c_src,
        req.params.position.line,
        req.params.position.character,
        &tol,
        &toc,
    );

    if (tol == -1 or toc == -1)
        return .{
            .jsonrpc = "2.0",
            .id = req.id,
            .result = null,
        };

    return .{
        .jsonrpc = "2.0",
        .id = req.id,
        .result = .{
            .uri = req.params.textDocument.uri,
            .range = .{
                .start = .{
                    .line = tol,
                    .character = toc,
                },
                .end = .{
                    .line = tol,
                    .character = toc,
                },
            },
        },
    };
}

// not necessary for now but just to make the lsp/lsp.zig respond funciton happy
pub fn deinit(_: response, _: std.mem.Allocator) void {
    return;
}
