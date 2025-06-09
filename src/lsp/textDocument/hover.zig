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
    _: std.mem.Allocator,
    req: request,
    state: State,
) !response {
    const document = state.document orelse return .{
        // we don have nothing to give
        .jsonrpc = "2.0",
        .id = req.id,
    };

    const md = get_hover_md(
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

fn get_hover_md(state: State, src: []const u8, l: u8, c: u8) ?[]const u8 {
    const boffset = utils.line_char_to_offset(src, l, c);

    if (state.cstate) |s| {
        for (0..s.nfields, s.fields) |_, f|
            if (utils.in_range(boffset, f.offset, f.name))
                return "### Variable: `" ++ f.name ++ "`";

        for (0..s.ntasks, s.tasks) |_, t| {
            for (0..t.nfields, t.fields) |_, f| {
                var dependencies: ?[*c]u8 = null;
                if (std.mem.eql(u8, f.name, "depends")) {
                    dependencies = f.value;

                    if (utils.in_range(boffset, f.offset, f.name))
                        return "### Task: `" ++ t.name ++
                            ("` depends on:\n---`" ++ dependencies) ++
                            ("`\n the dependency list is the list of tasks that should be run before this task");
                } else if (std.mem.eql(u8, f.name, "run"))
                    if (utils.in_range(boffset, f.offset, f.name))
                        return "### Run \n---\n The command this task will run";

                if (utils.in_range(boffset, f.offset, f.name)) {
                    return "### Field: `" ++ f.name ++ "`";
                }
            }

            if (utils.in_range(boffset, t.offset, t.name))
                return "### Task: `" ++ t.name ++ "`" ++
                    (if (t.description.len > 0) "\n---\n" ++ t.description else "");
        }
    }

    return null;
}
