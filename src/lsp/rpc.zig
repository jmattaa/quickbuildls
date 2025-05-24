const std = @import("std");

const utils = @import("../utils.zig");

pub const LspBaseMsg = struct {
    // idk what we do with it honestly
    jsonrpc: []const u8, // should always be "2.0"
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

/// decodes ONLY the contents of an LSP message
pub fn decode(
    comptime T: type,
    allocator: std.mem.Allocator,
    content: []const u8,
) !std.json.Parsed(T) {
    return std.json.parseFromSlice(
        T,
        allocator,
        content,
        .{ .ignore_unknown_fields = true },
    );
}

pub const SplitMsgRet = struct {
    header: []const u8,
    content: []const u8,
    full_len: usize,
};

/// splits the message to return only the contents and not headers.
/// returns `error.WaitForMoreData` if the message is not complete
pub fn splitMsg(buf: []const u8) !SplitMsgRet {
    const cut = utils.cut(buf, headersep) catch |e| switch (e) {
        utils.CutError.NotFound => return error.WaitForMoreData,
        else => unreachable,
    };

    const header = cut[0];
    const content = cut[1];

    var content_len: ?usize = null;
    var it = std.mem.tokenizeSequence(u8, header, "\r\n");
    while (it.next()) |line| {
        if (std.mem.startsWith(u8, line, contentlenheader)) {
            const len_str = line[contentlenheader.len..];
            content_len = try std.fmt.parseInt(usize, len_str, 10);
            break;
        }
    }

    if (content_len == null or content.len < content_len.?)
        return error.WaitForMoreData;

    return .{
        .header = header,
        .content = content,
        .full_len = header.len + headersep.len + content_len.?,
    };
}
