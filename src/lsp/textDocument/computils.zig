const std = @import("std");
const State = @import("../../state.zig").State;

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
    _ = src;
    _ = l;
    _ = c;
    _ = state;

    var items = std.ArrayList(completionitem).init(allocator);
    defer items.deinit();

    try items.append(
        .{
            .label = "test",
            .kind = COMPLETION_Variable,
            .detail = "this is a test and this is the label for the test",
            .documentation = .{
                .kind = "markdown",
                .value = "## this is a markdown test\n--\ntesting markdown",
            },
        },
    );

    return try items.toOwnedSlice();
}

pub fn deinit(
    allocator: std.mem.Allocator,
    items: []completionitem,
) void {
    allocator.free(items);
}
