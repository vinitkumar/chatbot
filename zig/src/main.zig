const std = @import("std");
const chatbot = @import("chatbot.zig");
const c = @cImport({
    @cInclude("stdio.h");
    @cInclude("string.h");
    @cInclude("ctype.h");
});

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    _ = c.printf("$ Chatbot v1.0.0!\n", );

    // Create hash table
    var ht = try chatbot.HashTable.create(allocator, 65536);
    defer ht.destroy();

    // Populate responses
    try ht.set("hi", "hello");
    try ht.set("hey", "hello");
    try ht.set("hear", "What you heard is right");
    try ht.set("python", "Yo, I love Python");
    try ht.set("light", "I like light");
    try ht.set("What", "It is clear, ain't it?");

    var buf: [chatbot.LineLength]u8 = undefined;

    while (true) {
        _ = c.printf("\n$ (user) ", );

        const result = c.fgets(&buf, chatbot.LineLength, c.stdin());
        if (result == null) break;

        if (@as(usize, c.strlen(&buf)) <= 1) break;

        const line = buf[0..c.strlen(&buf)];
        const trimmed = std.mem.trim(u8, line, " \t\r\n");
        if (trimmed.len == 0) break;

        var word_iter = std.mem.tokenizeAny(u8, trimmed, chatbot.SeparatorChars);

        while (word_iter.next()) |word| {
            const lower_word = try allocator.alloc(u8, word.len);
            defer allocator.free(lower_word);

            for (word, lower_word) |ch, *lch| {
                lch.* = std.ascii.toLower(ch);
            }

            if (std.mem.eql(u8, lower_word, "exit")) {
                return;
            }

            if (ht.get(lower_word)) |response| {
                _ = c.printf("\n$ (chatbot) %s\n", response.ptr);
            } else {
                _ = c.printf("\n$ (chatbot) %s\n", "Sorry, I don't know what to say about that");
            }
        }
    }
}
