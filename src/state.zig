const StateType = struct {
    document: []const u8,
    version: u32,
};

/// The state of the document, there is only going to be one document we have
/// to keep track of. Because we don't need any cross file completions and so on
 pub var State: StateType = .{
    .document = "",
    .version = 0,
};
