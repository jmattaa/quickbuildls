const utils = @import("../utils.zig");

pub const initializeresponse = struct {
    jsonrpc: []const u8,
    id: u32,
    result: struct {
        capabilities: struct {
            textdocumentsync: u8,
        },
        serverinfo: struct {
            name: []const u8,
            version: []const u8,
        },
    },
};

pub fn respond(id: u32) initializeresponse {
    return initializeresponse{
        .jsonrpc = "2.0",
        .id = id,
        .result = .{
            .capabilities = .{
                .textdocumentsync = 1, // full
            },
            .serverinfo = .{
                .name = "quickbuildls",
                .version = utils.VERSION,
            },
        },
    };
}

