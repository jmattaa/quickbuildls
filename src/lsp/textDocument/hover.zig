pub const request = struct {
    jsonrpc: []const u8,
    method: []const u8,
    id: u32,
    params: struct {
        textDocument: struct {
            uri: []const u8,
        },
        position: struct {
            line: u32,
            character: u32,
        },
    },
};

pub const response = struct {
    jsonrpc: []const u8,
    id: u32,
    // is null if we can't hover on the ting
    result: ?struct {
        contents: struct {
            kind: []const u8,
            value: []const u8,
        },
    } = null,
};

pub fn respond(req: request) response {
    return .{
        .jsonrpc = "2.0",
        .id = req.id,
        .result = .{
            .contents = .{
                .kind = "markdown",
                .value = "Hello from LSP",
            },
        },
    };
}
