

pub const position = struct {
    line: u32,
    character: u32,
};

pub const range = struct {
    start: position,
    end: position,
};
