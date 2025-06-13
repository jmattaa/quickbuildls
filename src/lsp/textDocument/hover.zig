const std = @import("std");

const State = @import("../../state.zig").State;
const utils = @import("../../utils.zig");
const lsputils = @import("../lsputils.zig");

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
        position: lsputils.position,
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
        range: ?lsputils.range = null,
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
                const doc = try get_doc_cmt(allocator, src, @intCast(f.offset));
                defer if (doc) |d| allocator.free(d);

                var res = std.ArrayList(u8).init(allocator);
                defer res.deinit();

                if (doc) |d| {
                    try res.appendSlice(d);
                    try res.appendSlice("\n---\n");
                }

                try res.appendSlice("## Field: `");
                try res.appendSlice(std.mem.span(f.name));
                try res.appendSlice("`");

                return try res.toOwnedSlice();
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
                        "## Field: `{s}`",
                        .{std.mem.span(f.name)},
                    );
                }
            }

            if (std.mem.eql(u8, std.mem.span(t.name), ident)) {
                const doc = try get_doc_cmt(allocator, src, @intCast(t.offset));
                defer if (doc) |d| allocator.free(d);

                var res = std.ArrayList(u8).init(allocator);
                defer res.deinit();

                if (doc) |d| {
                    try res.appendSlice(d);
                    try res.appendSlice("\n---\n");
                }

                try res.appendSlice("## Task: `");
                try res.appendSlice(std.mem.span(t.name));
                try res.appendSlice("`");

                if (dependencies) |deps| {
                    try res.appendSlice("\n### Depends on: `");
                    try res.appendSlice(deps);
                    try res.appendSlice("`");
                }

                return try res.toOwnedSlice();
            }
        }
    }

    return null;
}

fn get_doc_cmt(
    allocator: std.mem.Allocator,
    src: []const u8,
    off: usize,
) !?[]const u8 {
    var cmts = std.ArrayList([]const u8).init(allocator);
    defer cmts.deinit();

    // go to the line above
    var line_start = off;
    while (line_start > 0 and src[line_start - 1] != '\n') : (line_start -= 1) {}
    if (line_start == 0) return null;

    var i = line_start - 1;

    while (i > 0) : (i -= 1) {
        const lend = i;
        var lstart = i;
        while (lstart > 0 and src[lstart - 1] != '\n') : (lstart -= 1) {}
        const l = src[lstart..lend];

        // skip whitespace
        var j: usize = 0;

        while (j < l.len and (l[j] == ' ' or l[j] == '\t')) : (j += 1) {}
        if (j >= l.len or l[j] != '#') break;
        j += 1; // skip the '#'
        // skip whitespace
        while (j < l.len and (l[j] == ' ' or l[j] == '\t')) : (j += 1) {}
        if (j >= l.len) break;

        try cmts.insert(0, l[j..]);

        if (lstart == 0) break;
        i = lstart;
    }

    if (cmts.items.len == 0) return null;
    return try std.mem.join(allocator, "\n", cmts.items);
}
