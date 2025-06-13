const std = @import("std");
const lsputils = @import("../lsputils.zig");
const rpc = @import("../rpc.zig");
const State = @import("../../state.zig").State;

// once again this could be a c enum cuz zig enums idk they be annoying
pub const DIAGNOSTIC_Error = 1;
pub const DIAGNOSTIC_Warning = 2;
pub const DIAGNOSTIC_Info = 3;
pub const DIAGNOSTIC_Hint = 4;

pub const notif = struct {
    jsonrpc: []const u8,
    method: []const u8,
    params: struct {
        uri: []const u8,
        version: ?i64 = null,
        diagnostics: []diagnostics,
    },
};

pub const diagnostics = struct {
    range: lsputils.range,
    severity: ?u8 = null,
    source: ?[]const u8 = null,
    message: []const u8,
};

pub fn trypush(
    allocator: std.mem.Allocator,
    state: State,
    uri: []const u8,
    version: ?i64,
    out: anytype,
) !void {
    // TODO get error from the State
    if (state.cstate) |_| {
        var d = std.ArrayList(diagnostics).init(allocator);
        try d.append(.{
            .range = lsputils.range{
                .start = lsputils.position{
                    .line = 0,
                    .character = 0,
                },
                .end = lsputils.position{
                    .line = 0,
                    .character = 0,
                },
            },
            .severity = DIAGNOSTIC_Error,
            .source = "quickbuildls",
            .message = "Error",
        });

        const dnotif = notif{
            .jsonrpc = "2.0",
            .method = "textDocument/publishDiagnostics",
            .params = .{
                .uri = uri,
                .version = version,
                .diagnostics = try d.toOwnedSlice(),
            },
        };
        defer allocator.free(dnotif.params.diagnostics);

        const encoded = try rpc.encode(allocator, dnotif);
        defer allocator.free(encoded);
        try out.writeAll(encoded);
    }
}
