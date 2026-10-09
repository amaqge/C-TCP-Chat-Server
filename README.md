```markdown
# Multi-Threaded C++ Chat Server & Client

A robust, low-level multi-threaded chat application built from scratch in C++ using raw sockets (POSIX sockets), multi-threading (`std::thread`), and explicit thread synchronization (`std::mutex`).

---

## Architecture & Communication Flow

```text
+-----------------------------------------------------------------+
|                         SERVER ARCHITECTURE                     |
|                                                                 |
|  [ Main Thread: accept() ] ---> Listens on Port 8080            |
|         |                                                       |
|         v                                                       |
|  Spawns dedicated thread per client via SendMessage()           |
|  Manages shared global state: vector<clientinfo> clients        |
|  Thread-safety enforced via: std::mutex mtx                     |
+-----------------------------------------------------------------+
       ^                                                 ^
       | (TCP Sockets)                                   | (TCP Sockets)
       v                                                 v
+------------------+                             +------------------+
|     Client 1     |                             |     Client 2     |
|  - Main Thread:  |                             |  - Main Thread:  |
|    read loop     |                             |    read loop     |
|  - Detached      |                             |  - Detached      |
|    write loop    |                             |    write loop    |
+------------------+                             +------------------+

```

---

## Detailed Component Breakdown

### 1. Client-Side Lifecycle (`client.cpp`)

* **Connection Phase**: Establishes a TCP connection to the server via `connect()` on `127.0.0.1:8080`.
* **Concurrent Model**: Spawns two independent execution paths:
* **Writer Thread (`sendMessage`)**: Reads standard input (`std::cin`), appends the delimiter (`\n`), and streams data using a safe loop (`while (byte > 0)`).
* **Reader Thread (`readyMessage`)**: Runs asynchronously in the background, buffering incoming TCP streams, parsing lines by `\n`, and printing payloads to the console.



### 2. Server-Side Lifecycle (`server.cpp`)

* **Listener Loop**: The main thread initializes a TCP socket (`socket`, `bind`, `listen`), accepts incoming connections, assigns dynamic identifiers (`Client 1`, `Client 2`), and delegates each connection to an isolated thread running `SendMessage()`.
* **Message Handling Pipeline (`SendMessage`)**:
* **TCP Stream Buffering**: Appends incoming bytes (`recv`) to a local string buffer (`col`) and splits messages strictly at newline delimiters (`\n`) to prevent data fragmentation issues.
* **Command Interception**: Parses inputs starting with specific control characters:
* `/nick <new_name>`: Updates the client's handle in the global vector (with validation against empty spaces) and sends an acknowledgment.
* `/msg <target_name> <text>`: Routes a targeted private payload exclusively to the recipient and echoes a copy to the sender.


* **Broadcast Engine**: Iterates safely through the active `clients` vector under mutex lock protection, excluding the sender, and delivers messages using a resilient partial-send mechanism (`while (0 < byte)`).


* **Disconnection Management**: Detects dropped connections (`bytesReceived <= 0`), safely removes the socket from the global vector, and broadcasts a server notification (`[Server]: <name> left from the chat\n`) to all remaining peers.

---

## How to Build and Run

To compile and run the project locally using `g++` (Linux / macOS), open your terminal and follow these steps:

### 1. Compile the Server

```bash
g++ server.cpp -o server -std=c++11 -pthread

```

### 2. Compile the Client

```bash
g++ client.cpp -o client -std=c++11 -pthread

```

### 3. Run the Application

1. Start the server first:
```bash
./server

```


2. Open one or more new terminal windows and run the client(s):
```bash
./client

```



```

```
