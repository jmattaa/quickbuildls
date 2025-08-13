const std = @import("std");

const State = @import("../../state.zig").State;
const lsputils = @import("../lsputils.zig");
const lspdiagnostics = @import("./diagnostics.zig");

pub const request = struct {
    jsonrpc: []const u8,
    method: []const u8,
    id: u32,
    params: struct {
        textDocument: struct {
            uri: []const u8,
        },
        range: lsputils.range,
        context: struct {
            diagnostics: []lspdiagnostics.diagnostic,
        },
    },
};

pub const textEdit = struct {
    range: lsputils.range,
    newText: []const u8,

    pub fn jsonStringify(self: textEdit, jw: anytype) !void {
        try jw.beginObject();
        try jw.objectField("range");
        try jw.write(self.range);
        try jw.objectField("newText");
        try jw.write(self.newText);
        try jw.endObject();
    }
};

pub const actionResult = struct {
    title: []const u8,
    diagnostics: ?[]lspdiagnostics.diagnostic = null,
    edit: ?struct {
        changes: ?std.StringHashMap([]textEdit) = null,
    } = null,

    pub fn jsonStringify(self: actionResult, jw: anytype) !void {
        try jw.beginObject();

        try jw.objectField("title");
        try jw.write(self.title);

        if (self.diagnostics) |diags| {
            try jw.objectField("diagnostics");
            try jw.write(diags);
        }

        if (self.edit) |edit| {
            try jw.objectField("edit");
            try jw.beginObject();
            if (edit.changes) |changes| {
                try jw.objectField("changes");
                try jw.beginObject();
                var it = changes.iterator();
                while (it.next()) |kv| {
                    try jw.objectField(kv.key_ptr.*);
                    try jw.write(kv.value_ptr.*);
                }
                try jw.endObject();
            }
            try jw.endObject();
        }

        try jw.endObject();
    }
};

pub const response = struct {
    jsonrpc: []const u8,
    id: u32,
    result: ?[]actionResult = null,

    pub fn jsonStringify(self: response, jw: anytype) !void {
        try jw.beginObject();
        try jw.objectField("jsonrpc");
        try jw.write(self.jsonrpc);
        try jw.objectField("id");
        try jw.write(self.id);
        if (self.result) |res| {
            try jw.objectField("result");
            try jw.write(res);
        }
        try jw.endObject();
    }
};

pub fn respond(
    allocator: std.mem.Allocator,
    req: request,
    _: State,
) !response {
    var results = std.ArrayList(actionResult).init(allocator);
    defer results.deinit();

    for (req.params.context.diagnostics) |d| {
        if (getErrorAction(
            allocator,
            req.params.textDocument.uri,
            d,
        )) |action| {
            try results.append(action);
        }
    }

    return .{
        .jsonrpc = "2.0",
        .id = req.id,
        .result = try results.toOwnedSlice(),
    };
}

pub fn deinit(r: response, allocator: std.mem.Allocator) void {
    if (r.result) |result| {
        for (result) |*action| {
            if (action.edit) |*edit| {
                if (edit.changes) |*changes| {
                    var it = changes.iterator();
                    while (it.next()) |kv|
                        allocator.free(kv.value_ptr.*);
                    changes.deinit();
                }
            }
            if (action.diagnostics) |diags| {
                allocator.free(diags);
            }
        }
        allocator.free(result);
    }
}

fn getErrorAction(
    allocator: std.mem.Allocator,
    uri: []const u8,
    diagnostic: lspdiagnostics.diagnostic,
) ?actionResult {
    if (std.mem.eql(
        u8,
        diagnostic.message,
        "Missing semicolon at end of line",
    )) {
        var map = std.StringHashMap([]textEdit).init(allocator);
        var edits = allocator.alloc(textEdit, 1) catch return null;
        edits[0] = .{
            .range = .{
                .start = diagnostic.range.end,
                .end = diagnostic.range.end,
            },
            .newText = ";",
        };
        map.put(uri, edits) catch return null;

        const diagnostics = allocator.alloc(
            lspdiagnostics.diagnostic,
            1,
        ) catch return null;
        diagnostics[0] = diagnostic;

        return .{
            .title = "Add semicolon",
            .diagnostics = diagnostics,
            .edit = .{ .changes = map },
        };
    }
    return null;
}
