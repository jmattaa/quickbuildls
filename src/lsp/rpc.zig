const std = @import("std");
const utils = @import("../utils.zig");

pub const LspBaseMsg = struct {
    jsonrpc: []const u8, // should always be "2.0"
    method: []const u8,
    id: u32, // we'll expect only integers from neovim but other editors?? idk
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

pub fn decode(allocator: std.mem.Allocator, msg: []const u8) !std.json.Parsed(LspBaseMsg) {
    const cut = utils.cut(msg, headersep) catch |e| switch (e) {
        utils.CutError.NotFound => {
            std.debug.print("logger should take care of this later, but the msg is wrongly formatted\n", .{});
            return error.InvalidMessage;
        },
        else => unreachable,
    };

    const header = cut[0];
    const content = cut[1];

    const contentlenstr = header[contentlenheader.len..];
    const contentlen = try std.fmt.parseInt(u32, contentlenstr, 10);

    return std.json.parseFromSlice(LspBaseMsg, allocator, content[0..contentlen], .{.ignore_unknown_fields = true});
}
