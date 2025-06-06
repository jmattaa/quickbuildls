const std = @import("std");

const State = @import("../../state.zig").State;

const ccompletion = @cImport({
    @cInclude("completion.h");
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
    result: ?struct {
        isIncomplete: bool,
        items: ?[]completionitem = null,
    } = null,
};

// gotta perfectly match ccompletion.quickbuildls_completion
const completionitem = struct {
    label: []const u8,
    kind: ?ccompletion.completion_kind = null,
    detail: ?[]const u8 = null,
    documentation: ?struct {
        kind: ?[]const u8 = null,
        value: ?[]const u8 = null,
    } = null,
};

pub fn respond(allocator: std.mem.Allocator, req: request, state: State) !response {
    const document = state.document orelse return .{
        // we don have nothing to give
        .jsonrpc = "2.0",
        .id = req.id,
    };

    const c_str = try allocator.dupeZ(u8, document);
    defer allocator.free(c_str);

    var comp_count: usize = 0;
    const citems: [*c]ccompletion.quickbuildls_completion =
        ccompletion.get_completions(
            c_str,
            req.params.position.line,
            req.params.position.character,
            &comp_count,
        );

    if (comp_count == 0 or citems == null) {
        if (citems != null)
            ccompletion.free_completion(citems, comp_count);

        return .{
            .jsonrpc = "2.0",
            .id = req.id,
        };
    }

    var items = try allocator.alloc(completionitem, comp_count);
    errdefer allocator.free(items);

    for (0..comp_count) |i| {
        const citem = citems[i];

        const label = try allocator.dupe(u8, std.mem.span(citem.label));
        const kind =
            if (citem.kind >= 1 and citem.kind <= ccompletion.qls_compkind_TypeParameter)
                citem.kind
            else
                null;
        const detail = if (citem.detail != null)
            try allocator.dupe(
                u8,
                std.mem.span(citem.detail),
            )
        else
            null;

        const documentation: @TypeOf(items[i].documentation) =
            if (citem.documentation != null)
                .{
                    .kind = if (citem.documentation.*.kind != null)
                        try allocator.dupe(
                            u8,
                            std.mem.span(citem.documentation.*.kind),
                        )
                    else
                        null,
                    .value = if (citem.documentation.*.value != null)
                        try allocator.dupe(
                            u8,
                            std.mem.span(citem.documentation.*.value),
                        )
                    else
                        null,
                }
            else
                null;

        items[i] = .{
            .label = label,
            .kind = kind,
            .detail = detail,
            .documentation = documentation,
        };
    }
    // free the c stuff cuz we've copied it
    ccompletion.free_completion(citems, comp_count);

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
