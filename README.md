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
