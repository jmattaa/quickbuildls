const std = @import("std");

const State = @import("../../state.zig").State;
const utils = @import("../../utils.zig");

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

pub const completionitem = struct {
    label: []const u8,
    kind: ?u8 = null,
    detail: ?[]const u8 = null,
    documentation: ?struct {
        kind: ?[]const u8 = null,
        value: ?[]const u8 = null,
    } = null,
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
        if (cursor_in_brackets(src, l, c)) {
            for (0..s.nfields) |i| {
                const comp = try get_field_completion(
                    allocator,
                    s.fields[i],
                    src,
                );
                if (comp) |cmp| try items.append(cmp);
            }
        }
    }

    return try items.toOwnedSlice();
}

pub fn deinit(
    allocator: std.mem.Allocator,
    items: []completionitem,
) void {
    for (items) |i| if (i.documentation) |doc|
        if (doc.value) |v| allocator.free(v);

    allocator.free(items);
}

fn get_field_completion(
    allocator: std.mem.Allocator,
    f: cstate_s.qls_obj,
    src: []const u8,
) !?completionitem {
    if (utils.is_keyword(std.mem.span(f.name))) return null;
    return .{
        .label = std.mem.span(f.name),
        .kind = COMPLETION_Field,
        .detail = std.mem.span(f.value),
        .documentation = .{
            .kind = "markdown",
            .value = try utils.get_doc_cmt(
                allocator,
                src,
                @intCast(f.offset),
            ),
        },
    };
}

fn cursor_in_brackets(src: []const u8, l: u32, c: u32) bool {
    const off = utils.line_char_to_offset(src, l, c);
    if (off == 0 or off > src.len) return false;

    var i: usize = off - 1;
    while (i > 0) {
        if (src[i] == ']' or src[i] == '\n') return false;
        if (src[i] == '[') return true;
        i -= 1;
    }
    return false;
}
