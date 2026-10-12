# A Lightweight In-Memory Key-Value Store

This is a custom-built, single-threaded, event-driven in-memory key-value store written in modern C++. It implements a subset of the **Redis Serialization Protocol (RESP)**, supports raw TCP socket networking using I/O multiplexing (`select`), and provides an extensible architecture for executing atomic commands with $O(1)$ data lookups.

---

## Architecture & Technical Overview


### 1. TCP Design Overview
This program uses standard POSIX socket APIs to manage reliable, connection-oriented TCP communication between multiple clients and a server:

- **Address Resolution**: Uses `getaddrinfo()` to dynamically resolve hostnames and IP addresses, ensuring protocol-agnostic support for both IPv4 and IPv6.

- **Socket Lifecycle**: Follows the classic socket connection pattern: 
`socket()` creation $\rightarrow$ `bind()` to a local port $\rightarrow$ `listen()` for incoming connections with `SOMAXCONN` backlog management $\rightarrow$ `accept()` to spawn dedicated client connections.


### 2. Socket Class (`Socket.hpp` / `Socket.cpp`)
- **Design Choices**: Instead of scattering socket initialization boilerplate, `getaddrinfo` configuration, and stream operations across the application, the `Socket` class encapsulates state setup and data transfer helpers (`readRaw`, `writeRaw`).

- **Justification**: Separating network boilerplate from command execution keeps the main function clean and testable. 


### 3. Tokenization (`resp.cpp`)
- **How it works**: The `tokenize()` function parses a raw CLI command string character-by-character into a vector of strings. It features built-in state tracking (`inQuotes`, `quoteChar`) to handle single and double quotes.

- **Why it's needed**: Standard whitespace-based splitting would break user inputs containing spaces inside strings (e.g., `SET user "John Doe"`). Tokenization correctly groups quoted text into a single cohesive argument.


### 4. Serialization (`resp.cpp`)
- **How it works**: Serialization takes internal server responses and formats them into raw text strings packed with special prefix characters and `\r\n` (Carriage Return + Line Feed) endings so clients can easily read them.

- **Why it's needed**: Network protocols require a framing mechanism. Serialization transforms internal C++ data structures into byte streams that client applications can parse. It tells the receiver what kind of message, length of that message, and other information required for parsing.

- **Example 1 (Simple String)**: A success message like "OK" gets converted into +OK\r\n (where + indicates a simple string).

- **Example 2 (Bulk String)**: A data value like "bob" gets converted into $3\r\nbob\r\n (where $ indicates a bulk string, and 3 is the exact character length of the word).


### 5. RESP Class (`resp.h` / `resp.cpp`)
The `Resp` class serves as the core protocol translation:

- **Types Supported**: Allows for Simple Strings, Errors, Integers, Bulk Strings, and Arrays using the enum class `RespType`.

- **Recursive Parsing (`parser`)**: Handles streaming TCP data by inspecting buffers. Because TCP is a continuous byte stream, messages can arrive fragmented or concatenated. The parser inspects length headers (e.g., bulk string byte counts or array sizes), checks if the full payload has arrived, consumes the parsed frame from the buffer, and returns a fully realized command object.


### 6. Handler (`handler.h` / `handler.cpp`)
- **How it works**: The handler takes a parsed `Resp` object and a reference to the `KVStore` (the instance it will modify), validates argument counts (returning RESP errors if arg length is less than required), and calls the associated function. For example, if the command is parsed to be delete, it would update the `KVStore` object (the unordered map/the in-memory store), and relay feedback in the form of a parsed `Resp` object .

- **Why it's needed**: Decouples protocol parsing from command handling. The network loop only cares about bytes and buffers, while the handler focuses on command routing.


### 7. KVStore Class (`KVStore.h` / `KVStore.cpp`)
- **Underlying Data Structure**: Uses `std::unordered_map<string, string>` for the key-value store, guaranteeing average $O(1)$ time complexity for insertions (`set`), lookups (`get`), and deletions (`del`).

- **Case-Insensitive Command Lookup**: Command names are automatically converted to uppercase before lookup so users can type `get`, `Get`, or `GET`


### 8. Non-Blocking & Multiplexing I/O (`server.cpp`)
- **How it works**: Instead of locking up while waiting for a client to send data, the server uses `select()` to poll multiple connections (sockets) at once. When activity is detected, `FD_ISSET` identifies which specific socket is ready to read. Incoming data is read in chunks (of 12 bytes to ensure this works) and appended to a per-socket `std::string buffer` (`in_buffer[i]`), allowing the server to handle partial network reads without stalling

- **Buffer Isolation**: Every incoming client file descriptor has its own dedicated buffer (`std::unordered_map<int, std::string> in_buffer`) indexed by file descriptor. This prevents partial reads or interleaved data from corrupting concurrent client sessions when packets arrive incrementally.

---

## Commands Implemented

| Command | Arguments | Description | Return Type |
| :--- | :--- | :--- | :--- |
| **PING** | None | Tests connection liveness. | Simple String (`PONG`) |
| **SET** | `key value` | Stores a string value associated with a key. | Simple String (`OK`) |
| **GET** | `key` | Retrieves the string value stored at a given key. | Bulk String (or Null if missing) |
| **DEL** | `key` | Deletes a key from the store. | Integer (`1` if deleted, `0` otherwise) |

### Planned Future Implementations
1. **EXPIRE / TTL**: Adding time-to-live metadata tracking to keys for automated cache eviction.
2. **INCR / DECR**: Atomic numeric increment/decrement operations for counter use cases.
3. **Lists (`LPUSH`, `LRANGE`)**: Expanding beyond simple strings to support ordered list data structures.

---

## Getting Started

### Prerequisites
* A modern C++ compiler supporting C++17 or later (e.g., `g++`).
* A Linux/Unix-based environment (Ubuntu recommended).

### Compilation & Execution

1. **Compile the Server:**
   ```bash
   g++ -Wall -Wextra -Og -g server.cpp Socket.cpp resp.cpp KVStore.cpp handler.cpp -o server
   ```

2. **Compile the Client:**
   ```bash
   g++ -Wall -Wextra -Og -g client.cpp Socket.cpp resp.cpp -o client
   ```

3. **Run the Server:**
   ```bash
   ./server
   ```

4. **Connect via Client (in a separate terminal):**
   ```bash
   ./client
   ```


# TODO
- [Shows list of common commands](https://redis.io/tutorials/howtos/quick-start/cheat-sheet/)