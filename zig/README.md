# Chatbot - Zig Edition

This is a Zig port of the chatbot C implementation. The chatbot uses keyword matching to provide responses based on user input.

## Requirements

- Zig 0.15.2 or later (stable release)

## Building

Build the executable:

```bash
zig build-exe src/main.zig -femit-bin=zig-out/bin/chat
```

Or use the build system:

```bash
zig build
```

This will create the executable in `zig-out/bin/chat`.

## Running

```bash
./zig-out/bin/chat
```

Or via the build system:

```bash
zig build run
```

## Testing

Run all tests:

```bash
zig test src/chatbot.zig
```

Or via the build system:

```bash
zig build test
```

## Architecture

- **chatbot.zig**: Hash table implementation for keyword-response mapping
- **main.zig**: Interactive chatbot CLI

The hash table uses chaining for collision resolution and supports inserting, updating, and retrieving key-value pairs.
