pub const didOpenNotif = struct {
    jsonrpc: []const u8,
    method: []const u8,
    params: struct {
        textDocument: struct {
            uri: []const u8,
            languageId: []const u8,
            version: u32,
            text: []const u8,
        },
    },
};

pub const didChangeNotif = struct {
    jsonrpc: []const u8,
    method: []const u8,
    params: struct {
        textDocument: struct {
            uri: []const u8,
            version: u32,
        },
        contentChanges: []const struct {
            text: []const u8, // we ain't doin incremental change shi so we get
            // the whole document
        },
    },
};
