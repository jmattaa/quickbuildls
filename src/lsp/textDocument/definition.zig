const std = @import("std");

const State = @import("../../state.zig").State;
const utils = @import("../../utils.zig");

pub const request = struct {
    jsonrpc: []const u8,
    method: []const u8,
    id: u32,
    params: struct {
        textDocument: struct {
            uri: []const u8,
        },
        position: struct {
            line: u32,
            character: u32,
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
                line: u32,
                character: u32,
            },
            end: struct {
                line: u32,
                character: u32,
            },
        },
    } = null,
};

pub fn respond(_: std.mem.Allocator, req: request, state: State) !response {
    const document = state.document orelse return .{
        // we don have nothing to give
        .jsonrpc = "2.0",
        .id = req.id,
    };

    var tol: u32 = 0;
    var toc: u32 = 0;

    get_definition(
        state,
        document,
        req.params.position.line,
        req.params.position.character,
        &tol,
        &toc,
    ) catch |e| switch (e) {
        error.NotFound => return .{
            .jsonrpc = "2.0",
            .id = req.id,
            .result = null,
        },
        else => unreachable,
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

// this is too c style kinda, we could return the tol, toc in some sort of
// struct but yeh
fn get_definition(
    state: State,
    src: []const u8,
    l: u32,
    c: u32,
    tol: *u32,
    toc: *u32,
) error{NotFound}!void {
    const boffset: usize = utils.line_char_to_offset(src, l, c);
    if (boffset >= src.len)
        return;

    var start = boffset;
    while (start > 0 and utils.is_alphabetic(src[start - 1]))
        start -= 1;
    var end = boffset;
    while (end < src.len and utils.is_alphabetic(src[end]))
        end += 1;

    const ident = if (start < end)
        src[start..end]
    else
        return error.NotFound;

    if (utils.is_keyword(ident))
        return error.NotFound;

    if (state.cstate) |s| {
        for (0..s.nfields) |i| {
            const f = s.fields[i];
            const fname: []const u8 = std.mem.span(f.name);
            if (std.mem.eql(u8, fname, ident)) {
                tol.*, toc.* = utils.offset_to_line_char(
                    src,
                    @intCast(f.offset),
                );
                toc.* -= @intCast(fname.len);
                return;
            }
        }

        for (0..s.ntasks) |i| {
            const t = s.tasks[i];
            const tname: []const u8 = std.mem.span(t.name);
            if (std.mem.eql(u8, tname, ident)) {
                tol.*, toc.* = utils.offset_to_line_char(
                    src,
                    @intCast(t.offset),
                );
                toc.* -= @intCast(tname.len);
                return;
            }

            // TODO: i'm unsure if this actually is needed though
            for (0..t.nfields) |j| {
                const f = t.fields[j];
                const fname: []const u8 = std.mem.span(f.name);
                if (std.mem.eql(u8, fname, ident)) {
                    tol.*, toc.* = utils.offset_to_line_char(
                        src,
                        @intCast(f.offset),
                    );
                    toc.* -= @intCast(fname.len);
                    return;
                }
            }
        }
    }

    return error.NotFound;
}
