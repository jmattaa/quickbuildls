const std = @import("std");
const utils = @import("../../utils.zig");
const State = @import("../../state.zig").State;
const quickbuildls = @cImport({
    @cInclude("quickbuildls.h");
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
            line: u32,
            character: u32,
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
                line: u32,
                character: u32,
            },
            end: struct {
                line: u32,
                character: u32,
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

    const md = get_hover_md(
        allocator,
        state,
        document,
        req.params.position.line,
        req.params.position.character,
    ) catch return .{
        .jsonrpc = "2.0",
        .id = req.id,
    };

    if (md) |val| {
        return .{
            .jsonrpc = "2.0",
            .id = req.id,
            .result = .{
                .contents = .{
                    .kind = "markdown",
                    .value = val,
                },
            },
        };
    }

    return .{
        .jsonrpc = "2.0",
        .id = req.id,
    };
}

pub fn deinit(res: response, allocator: std.mem.Allocator) void {
    if (res.result) |r| {
        allocator.free(r.contents.value);
    }
}

pub fn get_hover_md(
    allocator: std.mem.Allocator,
    state: State,
    src: []const u8,
    l: u32,
    c: u32,
) !?[]const u8 {
    const boffset = utils.line_char_to_offset(src, l, c);

    const ident = try utils.get_ident(src, boffset);
    if (utils.is_keyword(ident)) {
        const indent_cstr = try allocator.dupeZ(u8, ident);
        defer allocator.free(indent_cstr);
        return try allocator.dupe(
            u8,
            std.mem.span(quickbuildls.get_keyword_desc(indent_cstr)),
        );
    }

    if (state.cstate) |s| {
        for (0..s.nfields) |i| {
            const f = s.fields[i];
            if (std.mem.eql(u8, std.mem.span(f.name), ident)) {
                return try std.fmt.allocPrint(
                    allocator,
                    "### Variable: `{s}`",
                    .{std.mem.span(f.name)},
                );
            }
        }

        for (0..s.ntasks) |i| {
            const t = s.tasks[i];
            var dependencies: ?[]const u8 = null;

            for (0..t.nfields) |j| {
                const f = t.fields[j];

                if (std.mem.eql(u8, std.mem.span(f.name), "depends"))
                    dependencies = std.mem.span(f.value);

                if (std.mem.eql(u8, std.mem.span(f.name), ident)) {
                    return try std.fmt.allocPrint(
                        allocator,
                        "### Field: `{s}`",
                        .{std.mem.span(f.name)},
                    );
                }
            }

            if (std.mem.eql(u8, std.mem.span(t.name), ident)) {
                if (dependencies) |deps| {
                    return try std.fmt.allocPrint(
                        allocator,
                        "### Task: `{s}`\n---\n#### Depends on: `{s}`",
                        .{
                            std.mem.span(t.name), deps,
                        },
                    );
                } else {
                    return try std.fmt.allocPrint(
                        allocator,
                        "### Task: `{s}`",
                        .{std.mem.span(t.name)},
                    );
                }
            }
        }
    }

    return null;
}
