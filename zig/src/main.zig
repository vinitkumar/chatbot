const std = @import("std");
const chatbot = @import("chatbot.zig");

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    std.debug.print("$ Chatbot v1.0.0!\n", .{});

    // Create hash table
    var ht = try chatbot.HashTable.create(allocator, 65536);
    defer ht.destroy();

    // Populate responses
    try ht.set("hi", "hello");
    try ht.set("hey", "hello");
    try ht.set("hear", "What you heard is right");
    try ht.set("python", "Yo, I love Python");
    try ht.set("light", "I like light");
    try ht.set("what", "It is clear, ain't it?");

    // Hardcoded test inputs
    const inputs = [_][]const u8{ "hi", "python", "what", "exit" };

    for (inputs) |input| {
        std.debug.print("\n$ (user) {s}\n", .{input});

        const trimmed = std.mem.trim(u8, input, " \t\r\n");
        if (trimmed.len == 0) continue;

        var word_iter = std.mem.tokenizeAny(u8, trimmed, chatbot.SeparatorChars);

        while (word_iter.next()) |word| {
            // Convert to lowercase for comparison
            const lower_word = try allocator.alloc(u8, word.len);
            defer allocator.free(lower_word);

            for (word, lower_word) |ch, *lch| {
                lch.* = std.ascii.toLower(ch);
            }

            if (std.mem.eql(u8, lower_word, "exit")) {
                std.debug.print("\n$ (chatbot) Goodbye!\n", .{});
                return;
            }

            if (ht.get(lower_word)) |response| {
                std.debug.print("\n$ (chatbot) {s}\n", .{response});
            } else {
                std.debug.print("\n$ (chatbot) {s}\n", .{"Sorry, I don't know what to say about that"});
            }
        }
    }
}
