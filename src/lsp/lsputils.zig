pub const position = struct {
    line: u32,
    character: u32,
};

pub const range = struct {
    start: position,
    end: position,
};

pub const textEdit = struct {
    range: range,
    newText: []const u8,
};
