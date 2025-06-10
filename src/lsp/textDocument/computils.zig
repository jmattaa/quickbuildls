const std = @import("std");

pub const completion_kind = enum(u8) {
    Text = 1,
    Method = 2,
    Function = 3,
    Constructor = 4,
    Field = 5,
    Variable = 6,
    Class = 7,
    Interface = 8,
    Module = 9,
    Property = 10,
    Unit = 11,
    Value = 12,
    Enum = 13,
    Keyword = 14,
    Snippet = 15,
    Color = 16,
    File = 17,
    Reference = 18,
    Folder = 19,
    EnumMember = 20,
    Constant = 21,
    Struct = 22,
    Event = 23,
    Operator = 24,
    TypeParameter = 25,
};

pub const completionitem = struct {
    label: []const u8,
    kind: completion_kind = completion_kind.Text,
    detail: ?[]const u8 = null,
    documentation: ?struct {
        kind: ?[]const u8 = null,
        value: ?[]const u8 = null,
    } = null,
};

pub fn getCompletions(
    allocator: std.mem.Allocator,
    src: []const u8,
    l: u8,
    c: u8,
    count: *u8,
) ![]completionitem {
    _ = src;
    _ = l;
    _ = c;

    var items = std.ArrayList(completionitem).init(allocator);
    defer items.deinit();

    try items.append(
        .{
            .label = "test",
            .detail = "this is a test and this is the label for the test",
            .kind = completion_kind.Variable,
            .documentation = .{
                .kind = "markdown",
                .value = "## this is a markdown test\n--\ntesting markdown",
            },
        },
    );

    count.* = 1;

    return items.toOwnedSlice();
}

pub fn deinit(allocator: std.mem.Allocator, items: []completionitem) void {
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
