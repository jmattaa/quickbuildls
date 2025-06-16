const std = @import("std");

const State = @import("../../state.zig").State;
const utils = @import("../../utils.zig");

const quickbuildls = @cImport({
    @cInclude("quickbuildls.h");
});

const cstate_s = @cImport({
    @cInclude("state.h");
});
// ain't an enum cuz they be annoying
// TODO: checkout how zig works with enums or maybe eventually declare it in
// c???
pub const COMPLETION_Text = 1;
pub const COMPLETION_Method = 2;
pub const COMPLETION_Function = 3;
pub const COMPLETION_Constructor = 4;
pub const COMPLETION_Field = 5;
pub const COMPLETION_Variable = 6;
pub const COMPLETION_Class = 7;
pub const COMPLETION_Interface = 8;
pub const COMPLETION_Module = 9;
pub const COMPLETION_Property = 10;
pub const COMPLETION_Unit = 11;
pub const COMPLETION_Value = 12;
pub const COMPLETION_Enum = 13;
pub const COMPLETION_Keyword = 14;
pub const COMPLETION_Snippet = 15;
pub const COMPLETION_Color = 16;
pub const COMPLETION_File = 17;
pub const COMPLETION_Reference = 18;
pub const COMPLETION_Folder = 19;
pub const COMPLETION_EnumMember = 20;
pub const COMPLETION_Constant = 21;
pub const COMPLETION_Struct = 22;
pub const COMPLETION_Event = 23;
pub const COMPLETION_Operator = 24;
pub const COMPLETION_TypeParameter = 25;

pub const INSERTTEXTFORMAT_PlainText = 1;
pub const INSERTTEXTFORMAT_Snippet = 2;

pub const completionitem = struct {
    label: []const u8,
    kind: ?u8 = null,
    detail: ?[]const u8 = null,
    documentation: ?struct {
        kind: ?[]const u8 = null,
        value: ?[]const u8 = null,
    } = null,
    insertTextFormat: ?u8 = null,
    insertText: ?[]const u8 = null,
};

pub fn getCompletions(
    allocator: std.mem.Allocator,
    state: State,
    src: []const u8,
    l: u32,
    c: u32,
) ![]completionitem {
    var items = std.ArrayList(completionitem).init(allocator);
    defer items.deinit();

    if (state.cstate) |s| {
        const off = utils.lineCharToOffset(src, l, c);

        if (is_cursor_in_brackets(src, off)) {
            if (find_enclosing_task(src, off)) |tname| {
                if (utils.getTaskByName(s, tname)) |t| {
                    // iterator
                    if (t.value) |v| {
                        try items.append(.{
                            .label = std.mem.span(v),
                            .kind = COMPLETION_Variable,
                        });
                    }

                    // local fields
                    for (0..t.nfields) |i| {
                        const comp = try get_field_completion(
                            allocator,
                            t.fields[i],
                            src,
                        );
                        if (comp) |cmp| try items.append(cmp);
                    }
                }
            }

            for (0..s.nfields) |i| {
                const comp = try get_field_completion(
                    allocator,
                    s.fields[i],
                    src,
                );
                if (comp) |cmp| try items.append(cmp);
            }
        } else if (is_cursor_in_depends(src, off)) {
            for (0..s.ntasks) |i| {
                const t = s.tasks[i];
                try items.append(.{
                    .label = std.mem.span(if (t.quotedname) |q| q else t.name),
                    .kind = COMPLETION_Function,
                    .documentation = .{
                        .kind = "markdown",
                        .value = try utils.getDocCmt(
                            allocator,
                            src,
                            @intCast(t.offset),
                        ),
                    },
                });
            }
            for (0..s.nfields) |i| {
                const f = s.fields[i];
                try items.append(.{
                    .label = std.mem.span(if (f.quotedname) |q| q else f.name),
                    .kind = COMPLETION_Field,
                    .documentation = .{
                        .kind = "markdown",
                        .value = try utils.getDocCmt(
                            allocator,
                            src,
                            @intCast(f.offset),
                        ),
                    },
                });
            }
        } else if (is_cursor_in_task(src, off)) {
            for (quickbuildls.task_keyword_names) |kw| {
                try items.append(
                    .{
                        .label = std.mem.span(kw),
                        .kind = COMPLETION_Snippet,
                        .insertTextFormat = INSERTTEXTFORMAT_Snippet,
                        .insertText = try std.fmt.allocPrint(
                            allocator,
                            "{s} = $1;",
                            .{kw},
                        ),
                    },
                );
            }
        }
    }

    return try items.toOwnedSlice();
}

pub fn deinit(
    allocator: std.mem.Allocator,
    items: []completionitem,
) void {
    for (items) |i| {
        if (i.documentation) |doc| if (doc.value) |v| allocator.free(v);
        if (i.insertText) |v| allocator.free(v);
    }

    allocator.free(items);
}

fn get_field_completion(
    allocator: std.mem.Allocator,
    f: cstate_s.qls_obj,
    src: []const u8,
) !?completionitem {
    if (utils.isKeyword(std.mem.span(f.name))) return null;
    return .{
        .label = std.mem.span(f.name),
        .kind = COMPLETION_Field,
        .detail = std.mem.span(f.value),
        .documentation = .{
            .kind = "markdown",
            .value = try utils.getDocCmt(
                allocator,
                src,
                @intCast(f.offset),
            ),
        },
    };
}

fn is_cursor_in_brackets(src: []const u8, off: usize) bool {
    if (off == 0 or off > src.len) return false;

    var i: usize = off - 1;
    while (i > 0) {
        if (src[i] == ']' or src[i] == '\n') return false;
        if (src[i] == '[') return true;
        i -= 1;
    }
    return false;
}

// scan backward for "task_name" {
fn find_enclosing_task(src: []const u8, off: usize) ?[]const u8 {
    var i: usize = off;
    while (i > 0) : (i -= 1) {
        if (src[i] == '{') {
            var j: usize = i;

            while (j > 0 and std.ascii.isWhitespace(src[j - 1])) : (j -= 1) {}

            if (src[j] == '"') {
                // it's the `"name" {` pattern
                var k = j - 1;
                while (k > 0 and src[k] != '"') : (k -= 1) {}
                return src[k + 1 .. j - 1];
            }

            // check for the `name as smth` pattern
            const line_start = blk: {
                var k = j;
                while (k > 0 and src[k - 1] != '\n') : (k -= 1) {}
                break :blk k;
            };

            const line = src[line_start..j];
            const as_index = std.mem.indexOf(u8, line, " as ") orelse return null;
            const before_as = std.mem.trimRight(u8, line[0..as_index], " ");

            return if (before_as.len > 0) before_as else null;
        }
    }
    return null;
}

fn is_cursor_in_depends(src: []const u8, off: usize) bool {
    var line_start: usize = off;
    while (line_start > 0 and src[line_start - 1] != '\n') : (line_start -= 1) {}

    const line = src[line_start..@min(src.len, line_start + 64)];
    return std.mem.indexOf(u8, line, "depends") != null and
        std.mem.indexOf(u8, line, "=") != null;
}

fn is_cursor_in_task(src: []const u8, off: usize) bool {
    var i: usize = off - 1;
    while (i > 0) {
        if (src[i] == '}') return false;
        if (src[i] == '{') return true;
        i -= 1;
    }
    return false;
}
