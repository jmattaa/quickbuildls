const utils = @import("../utils.zig");

pub const initializeresponse = struct {
    jsonrpc: []const u8,
    id: u32,
    result: struct {
        capabilities: struct {
            textDocumentSync: u8,
        },
        serverInfo: struct {
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
                .textDocumentSync = 1, // full
            },
            .serverInfo = .{
                .name = "quickbuildls",
                .version = utils.VERSION,
            },
        },
    };
}
