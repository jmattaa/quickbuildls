const std = @import("std");
pub const rpc = @import("rpc.zig");
pub const initialize = @import("initialize.zig");
pub const textDocument = @import("textDocument/textDocument.zig");

pub const StateType = @import("../state.zig").State;

/// ```zig
/// fn respond(
///     allocator: std.mem.Allocator,
///     req: Req, 
///     State: StateType,
/// ) !Res
/// the Res.respond function should have the signature above
///
/// `T` should look at least have the following signature as base
/// ```zig
/// const T = struct {
///   request: struct {...},
///   response: struct {...},
///   respond: fn () // the above function
/// };
pub fn respond(
    allocator: std.mem.Allocator,
    comptime T: type,
    content: []const u8,
    State: StateType,
    out: anytype,
) !void {
    const req = try rpc.decode(T.request, allocator, content);
    defer req.deinit();

    const res = try T.respond(allocator, req.value, State);
    const encoded = try rpc.encode(allocator, res);
    defer allocator.free(encoded);
    try out.writeAll(encoded);
}
