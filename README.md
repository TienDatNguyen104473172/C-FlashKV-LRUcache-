# FlashKV

> **High-Performance, Multi-threaded In-Memory Key-Value Store with Async Persistence.**

**FlashKV** is a robust, lightweight Key-Value database engine built from scratch in C++. It demonstrates advanced systems programming concepts including **Multi-threading**, **Thread Safety**, **LRU Caching**, and **Asynchronous I/O**.

Designed as a high-performance backend component, FlashKV handles concurrent client connections efficiently and ensures data durability without blocking the main request processing loop.

---

## Key Features

-   **Multi-threaded Server**: Implements a **Thread-per-Client** architecture to handle thousands of concurrent connections simultaneously.
-   **Thread Safety**: Ensures data integrity across multiple threads using fine-grained locking (`std::mutex`, `std::lock_guard`) to prevent Race Conditions.
-   **LRU Cache**: Built-in **Least Recently Used (LRU)** cache eviction policy to optimize read performance for frequently accessed data.
-   **Async Logging (WAL)**: Decouples Disk I/O from the critical path using a **Producer-Consumer** model with `std::condition_variable` and a background worker thread. This ensures `SET` operations are extremely fast and not bottlenecked by disk writes.
-   **Crash Recovery**: Automatically restores data from the disk log upon startup.
-   **Custom Protocol**: Supports a text-based protocol capable of handling keys and values with spaces.

## Architecture

### Core Components
1.  **Server (`server.cpp`)**:
    -   Uses `Winsock2` for networking.
    -   Spawns a detached `std::thread` for each incoming connection.
2.  **KV Store (`kv_store.cpp`)**:
    -   The central engine managing data storage.
    -   Combines an in-memory `std::unordered_map` with an `LRUCache`.
    -   Manages the **Async Logger** thread.
3.  **LRU Cache (`lru_cache.cpp`)**:
    -   Hybrid data structure using `std::list` (doubly linked list) and `std::unordered_map` for O(1) access and eviction.
4.  **Async Logger**:
    -   A dedicated background thread that sleeps when idle and wakes up only when there are logs to write, maximizing CPU efficiency.

## 📂 Folder Structure

```
FlashKV/
├── include/
│   ├── kv_store.h       # Key-Value Store & Async Logger definitions
│   └── lru_cache.h      # LRU Cache definitions
├── src/
│   ├── kv_store.cpp     # Core logic implementation
│   ├── lru_cache.cpp    # LRU Cache implementation
│   ├── main.cpp         # CLI Entry point (optional)
│   └── server.cpp       # Multi-threaded Server entry point
├── build/               # Build artifacts
├── CMakeLists.txt       # CMake build configuration
├── stress_test.py       # Python stress test script
└── README.md            # Project documentation
```

## Build & Run

### Prerequisites
-   C++20 compatible compiler (MSVC, MinGW, or Clang)
-   CMake (3.10+)
-   Windows OS (due to Winsock dependency)

### Build Steps
```bash
# 1. Clone the repository
git clone https://github.com/TienDatNguyen104473172/C-FlashKV-LRUcache-.git
cd FlashKV

# 2. Create build directory
mkdir build
cd build

# 3. Configure and Build
cmake ..
cmake --build .
```

### Running the Server
```bash
# Inside the build directory
.\FlashServer.exe
```
*You should see: `[Multi-Threaded Server] Listening on port 8888...`*

## Usage

You can connect to FlashKV using `telnet`, `netcat`, or the provided Python stress test tool.

### Using Telnet
```bash
telnet localhost 8888
```

### Supported Commands
| Command | Syntax | Description |
| :--- | :--- | :--- |
| **SET** | `SET <key> <value>` | Stores a key-value pair. Value can contain spaces. |
| **GET** | `GET <key>` | Retrieves the value for a key. |
| **DEL** | `DEL <key>` | Deletes a key. |

**Example Session:**
```text
SET user:1 "Nguyen Van A"
OK
GET user:1
Nguyen Van A
DEL user:1
(integer) 1
```

## Performance Testing

FlashKV includes a Python script to stress-test the server and verify thread safety under load.

```bash
# Run the stress test (Simulates 100 concurrent users)
python stress_test.py
```

**Expected Output:**
```text
---  STARTING STRESS TEST ON FLASHKV ---
Target: localhost:8888
Virtual Users: 100
Total Requests: 40000 (SET + GET)
----------------------------------------
...
---  REPORT ---
Time Taken: 2.45 seconds
Successful Operations: 20000 cycles (SET+GET)
Failed Operations: 0
THROUGHPUT (RPS): 16326.53 requests/second
STATUS: PASSED - Server is Rock Solid!
```

## Author

**Tien Dat Nguyen**
-   **Role**: C++/Backend Developer
-   **Focus**: High-performance Systems, Concurrency, Data Structures.

---
*This project was built for educational purposes to demonstrate advanced C++ system design patterns.*

## License

```
(License text to be added)
```
