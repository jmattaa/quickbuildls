const std = @import("std");

const utils = @import("../utils.zig");

pub const LspBaseMsg = struct {
    // idk what we do with it honestly
    // jsonrpc: []const u8, // should always be "2.0"
    method: []const u8,

    // we'll expect only integers from neovim but other editors?? idk
    // it's optional cuz notifications don't have an id
    id: ?u32 = null,
};

const contentlenheader = "Content-Length: ";
const headersep = "\r\n\r\n";

pub fn encode(allocator: std.mem.Allocator, msg: anytype) ![]const u8 {
    var buf = std.ArrayList(u8).init(allocator);
    defer buf.deinit();

    try std.json.stringify(msg, .{}, buf.writer());

    const len_str = try std.fmt.allocPrint(allocator, "{}", .{buf.items.len});
    defer allocator.free(len_str);

    const res = try allocator.alloc(u8, contentlenheader.len + len_str.len +
        headersep.len + buf.items.len);

    var stream = std.io.fixedBufferStream(res);
    var writer = stream.writer();
    try writer.writeAll(contentlenheader);
    try writer.writeAll(len_str);
    try writer.writeAll(headersep);
    try writer.writeAll(buf.items);

    return res;
}

pub fn decode(
    allocator: std.mem.Allocator,
    msg: []const u8,
) !std.json.Parsed(LspBaseMsg) {
    const cut = utils.cut(msg, headersep) catch |e| switch (e) {
        utils.CutError.NotFound => {
            return error.WaitForMoreData;
        },
        else => unreachable,
    };

    const header = cut[0];
    const content = cut[1];

    var it = std.mem.tokenizeSequence(u8, header, "\r\n");
    var contentlen: ?u32 = null;

    while (it.next()) |line| {
        if (std.mem.startsWith(u8, line, contentlenheader)) {
            const len_str = line[contentlenheader.len..];
            contentlen = try std.fmt.parseInt(u32, len_str, 10);
            break;
        }
    }

    if (contentlen == null) return error.MissingContentLength;

    if (content.len < contentlen.?)
        return error.WaitForMoreData;

    return std.json.parseFromSlice(
        LspBaseMsg,
        allocator,
        content[0..contentlen.?],
        .{ .ignore_unknown_fields = true },
    );
}

pub fn readMessage(
    allocator: std.mem.Allocator,
    reader: anytype,
) !struct {
    parsed: std.json.Parsed(LspBaseMsg),
    buf: std.ArrayList(u8),
} {
    var buf = std.ArrayList(u8).init(allocator);
    while (true) {
        var temp: [1024]u8 = undefined;
        const n = try reader.read(&temp);
        if (n == 0) continue; // wait for more data

        try buf.appendSlice(temp[0..n]);

        return .{ .parsed = decode(allocator, buf.items) catch |e| switch (e) {
            error.WaitForMoreData => continue,
            else => return e,
        }, .buf = buf };
    }
}
