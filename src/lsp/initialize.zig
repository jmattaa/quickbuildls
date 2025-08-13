const utils = @import("../utils.zig");

pub const initializeresponse = struct {
    jsonrpc: []const u8,
    id: u32,
    result: struct {
        capabilities: struct {
            textDocumentSync: u8,
            hoverProvider: bool,
            definitionProvider: bool,
            codeActionProvider: bool,
            completionProvider: struct {},
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
                // honetly i don't think it's a bad idea
                // to send the full thing and not do
                // partial updates cuz it's only one file
                // we're working with
                .textDocumentSync = 1, // full
                .hoverProvider = true,
                .definitionProvider = true,
                .codeActionProvider = true,
                .completionProvider = .{}, // just say we want to do completions
            },
            .serverInfo = .{
                .name = "quickbuildls",
                .version = utils.VERSION,
            },
        },
    };
}
