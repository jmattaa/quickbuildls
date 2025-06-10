const std = @import("std");
const utils = @import("../../utils.zig");
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
            line: u8,
            character: u8,
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
                line: u8,
                character: u8,
            },
            end: struct {
                line: u8,
                character: u8,
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

    const md = try get_hover_md(
        allocator,
        state,
        document,
        req.params.position.line,
        req.params.position.character,
    );

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

pub fn deinit(_: response, _: std.mem.Allocator) void {
    return;
}

pub fn get_hover_md(
    allocator: std.mem.Allocator,
    state: State,
    src: []const u8,
    l: u8,
    c: u8,
) !?[]const u8 {
    const boffset = utils.line_char_to_offset(src, l, c);

    if (state.cstate) |s| {
        for (0..s.nfields) |i| {
            const f = s.fields[i];
            if (utils.in_range(
                boffset,
                f.offset,
                std.mem.span(f.name),
            )) {
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

                if (std.mem.eql(u8, std.mem.span(f.name), "depends")) {
                    dependencies = std.mem.span(f.value);

                    if (utils.in_range(
                        boffset,
                        f.offset,
                        std.mem.span(f.name),
                    )) {
                        return try std.fmt.allocPrint(
                            allocator,
                            "### Task: `{s}`\n---\n`{s}`\nThe dependency list is the list of tasks that should be run before this task",
                            .{ std.mem.span(t.name), dependencies.? },
                        );
                    }
                } else if (std.mem.eql(u8, std.mem.span(f.name), "run")) {
                    if (utils.in_range(
                        boffset,
                        f.offset,
                        std.mem.span(f.name),
                    )) {
                        return "### Run \n---\n The command this task will run";
                    }
                }

                if (utils.in_range(
                    boffset,
                    f.offset,
                    std.mem.span(f.name),
                )) {
                    return try std.fmt.allocPrint(
                        allocator,
                        "### Field: `{s}`",
                        .{std.mem.span(f.name)},
                    );
                }
            }

            if (utils.in_range(
                boffset,
                t.offset,
                std.mem.span(t.name),
            )) {
                if (dependencies) |deps| {
                    return try std.fmt.allocPrint(
                        allocator,
                        "### Task: `{s}`\n#### Depends on:\n---\n{s}",
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
