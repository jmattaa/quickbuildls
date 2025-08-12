const std = @import("std");

const State = @import("../../state.zig").State;
const cstate_s = @cImport({
    @cInclude("state.h");
});

const utils = @import("../../utils.zig");
const lsputils = @import("../lsputils.zig");

const quickbuildls = @cImport({
    @cInclude("quickbuildls.h");
});

const MAX_FIELD_HOVER_VALUE_LEN: usize = 40;

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

    const boffset = utils.lineCharToOffset(
        document,
        req.params.position.line,
        req.params.position.character,
    );
    const ident = utils.getIdent(document, boffset) catch return .{
        .jsonrpc = "2.0",
        .id = req.id,
    };

    const md = get_hover_md(
        allocator,
        state,
        ident,
        boffset,
        document,
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
                .range = .{
                    .start = .{
                        .line = req.params.position.line,
                        .character = req.params.position.character,
                    },
                    .end = .{
                        .line = req.params.position.line,
                        .character = req.params.position.character,
                    },
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
    ident: []const u8,
    boffset: usize,
    src: []const u8,
) !?[]const u8 {
    if (state.cstate) |s| {
        for (0..s.nfields) |i| {
            const f = s.fields[i];
            if (std.mem.eql(u8, std.mem.span(f.name), ident))
                return try get_field_hover(allocator, f, ident, src);
        }

        for (0..s.ntasks) |i| {
            const t = s.tasks[i];
            var dependencies: ?[]const u8 = null;

            for (0..t.nfields) |j| {
                const f = t.fields[j];
                const fname = std.mem.span(f.name);
                if (std.mem.eql(u8, fname, "depends"))
                    dependencies = std.mem.span(f.value);

                if (std.mem.eql(u8, fname, ident)
                // the fields inside a task shouldn't show hover info
                // unless the cursor is on them, this will make it so that
                // we keep the fields in a task inside of that scope 🔥
                and utils.isInRange(boffset, f.offset, fname))
                    return try get_field_hover(allocator, f, ident, src);
            }

            if (std.mem.eql(u8, std.mem.span(t.name), ident)) {
                const doc = try utils.getDocCmt(
                    allocator,
                    src,
                    @intCast(t.offset),
                );
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

fn get_field_hover(
    allocator: std.mem.Allocator,
    f: cstate_s.qls_obj,
    ident: []const u8,
    src: []const u8,
) !?[]const u8 {
    if (utils.isKeyword(ident)) {
        const indent_cstr = try allocator.dupeZ(u8, ident);
        defer allocator.free(indent_cstr);
        return try allocator.dupe(
            u8,
            std.mem.span(quickbuildls.get_keyword_desc(indent_cstr)),
        );
    }

    const doc = try utils.getDocCmt(
        allocator,
        src,
        @intCast(f.offset),
    );
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

    if (f.value) |val| {
        const v = std.mem.span(val);

        try res.appendSlice("\n---\n**Value:** `");

        if (v.len > MAX_FIELD_HOVER_VALUE_LEN) {
            try res.appendSlice(v[0..MAX_FIELD_HOVER_VALUE_LEN]);
            try res.appendSlice("...");
        } else {
            try res.appendSlice(v);
        }

        try res.appendSlice("`");
    }

    return try res.toOwnedSlice();
}
