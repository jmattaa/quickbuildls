const std = @import("std");

const State = @import("../../state.zig").State;
const utils = @import("../../utils.zig");
const lsputils = @import("../lsputils.zig");
const rpc = @import("../rpc.zig");

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
    if (state.document == null) return;
    if (state.cstate) |cs| {
        var d = std.ArrayList(diagnostics).init(allocator);

        if (cs.err) |e|
            if (e.*.msg) |msg| {
                const lchar = utils.offsetToLineChar(
                    state.document.?,
                    e.*.offset,
                );
                const lcharend = utils.offsetToLineChar(
                    state.document.?,
                    e.*.endoffset,
                );

                try d.append(.{
                    .range = lsputils.range{
                        .start = lsputils.position{
                            .line = lchar[0],
                            .character = lchar[1],
                        },
                        .end = lsputils.position{
                            .line = lcharend[0],
                            .character = lcharend[1],
                        },
                    },
                    .severity = DIAGNOSTIC_Error,
                    .source = "quickbuildls",
                    .message = std.mem.span(msg),
                });
            };

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
