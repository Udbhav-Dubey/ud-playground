claude 
# Integrating Your Daemon + Poll Server into an Android APK

## Author's Note

This document is a **technical guide**, not finished code. It explains the architecture, design decisions, and integration points you need to understand to build this yourself. The hard work of implementation is yours - this is just the map.

---

## Table of Contents

1. [Overview](#overview)
2. [Current Architecture Analysis](#current-architecture-analysis)
3. [Android Integration Strategy](#android-integration-strategy)
4. [Component Breakdown](#component-breakdown)
5. [Threading Model](#threading-model)
6. [Data Flow Diagrams](#data-flow-diagrams)
7. [Implementation Roadmap](#implementation-roadmap)
8. [Design Challenges & Solutions](#design-challenges--solutions)
9. [Testing Strategy](#testing-strategy)
10. [Performance Considerations](#performance-considerations)

---

## Overview

### What You Have

You've built two sophisticated C++ components:

1. **Daemon** - An event-driven worker with:
   - Asynchronous event queue
   - Background worker thread
   - Transport abstraction layer
   - Peer event management

2. **Poll-based Server** - A non-blocking I/O server with:
   - Multi-client support via poll()
   - Efficient event multiplexing
   - Client state management
   - Non-blocking sockets

### The Goal

Integrate these components into an Android application where:
- The daemon manages application logic and event processing
- The poll server handles actual network I/O
- A thin Kotlin UI layer provides user interaction
- All C++ code runs natively via Android NDK

### Why This Works

Your architecture is **already mobile-ready**:
- Event-driven (doesn't block UI)
- Modular (daemon + transport separation)
- Thread-safe (mutexes already in place)
- Abstracted (transport interface is pluggable)

---

## Current Architecture Analysis

### Daemon Component

**Location:** `daemon.cpp`, `daemon.h`

**Core Responsibilities:**
```
┌─────────────────────────────────────┐
│           Daemon Class              │
├─────────────────────────────────────┤
│ Private:                            │
│  - bool running                     │
│  - bool busy                        │
│  - std::mutex mtx                   │
│  - std::condition_variable cv       │
│  - std::thread worker               │
│  - std::queue<Event> work_queue     │
│  - Transport* transport             │
├─────────────────────────────────────┤
│ Public API:                         │
│  + start()                          │
│  + stop()                           │
│  + is_running()                     │
│  + is_busy()                        │
│  + enqueue_event(Event)             │
│  + set_transport(Transport*)        │
│  + sendy(peer_id, data)             │
└─────────────────────────────────────┘
```

**Event Types:**
- `peerConnected` - New peer joined
- `peerDisconnected` - Peer left
- `dataReceived` - Data from peer

**Key Insight:** The daemon is a **producer-consumer pattern**:
- Events are produced by external sources (network, UI, timers)
- Worker thread consumes events from queue
- Processing happens asynchronously

---

### Poll Server Component

**Location:** `server.cpp`

**Core Responsibilities:**
```
┌─────────────────────────────────────┐
│        Poll Server Loop             │
├─────────────────────────────────────┤
│ State:                              │
│  - int sockfd (listening socket)    │
│  - vector<pollfd> poll_fds          │
│  - map<int, Client> clients         │
├─────────────────────────────────────┤
│ Operations:                         │
│  1. poll() on all fds               │
│  2. POLLIN on sockfd → accept()     │
│  3. POLLIN on client → recv()       │
│  4. POLLOUT on client → send()      │
│  5. POLLHUP → disconnect            │
└─────────────────────────────────────┘
```

**Client Structure:**
```cpp
struct Client {
    int fd;
    bool name_flag;
    std::string name;
    std::string readbuf;
    std::string writebuf;
};
```

**Key Insight:** This is **edge-triggered I/O multiplexing**:
- Single thread handles all clients
- Non-blocking operations
- Event-driven (poll tells us when ready)

---

### MeshCore Bridge

**Location:** `meshcore_bridge.c`, `meshcore_impl.cpp`

**Purpose:** Provides a C API for external languages (Java/Kotlin via JNI)

**Current API:**
```c
meshcore* meshcore_create();
void meshcore_destroy(meshcore*);
bool is_meshcore_running(const meshcore*);
```

**Key Insight:** This is your **FFI boundary**. JNI can only call C functions, not C++ classes directly. This bridge pattern is exactly what you need.

---

## Android Integration Strategy

### High-Level Architecture

```
┌────────────────────────────────────────────────────────┐
│                    Android Process                     │
│                                                        │
│  ┌──────────────────────────────────────────────┐    │
│  │         Dalvik/ART VM (Java Layer)           │    │
│  │                                              │    │
│  │  ┌────────────────────────────────────────┐ │    │
│  │  │  Kotlin/Java Code                      │ │    │
│  │  │  - Activity/Composable UI              │ │    │
│  │  │  - ViewModel/State                     │ │    │
│  │  │  - NativeBridge wrapper class          │ │    │
│  │  └──────────────┬─────────────────────────┘ │    │
│  │                 │ JNI calls                  │    │
│  └─────────────────┼────────────────────────────┘    │
│                    │                                  │
│  ┌─────────────────▼────────────────────────────┐    │
│  │       Native Library (.so)                   │    │
│  │                                              │    │
│  │  ┌────────────────────────────────────────┐ │    │
│  │  │  JNI Glue Functions                    │ │    │
│  │  │  - Java_..._init()                     │ │    │
│  │  │  - Java_..._sendMessage()              │ │    │
│  │  │  - Java_..._getEvents()                │ │    │
│  │  └──────────────┬─────────────────────────┘ │    │
│  │                 │                            │    │
│  │  ┌──────────────▼─────────────────────────┐ │    │
│  │  │  MeshCore C API                        │ │    │
│  │  │  (meshcore_bridge.c)                   │ │    │
│  │  └──────────────┬─────────────────────────┘ │    │
│  │                 │                            │    │
│  │  ┌──────────────▼─────────────────────────┐ │    │
│  │  │  Daemon + Transport (C++)              │ │    │
│  │  │  - Your existing code!                 │ │    │
│  │  └────────────────────────────────────────┘ │    │
│  │                                              │    │
│  └──────────────────────────────────────────────┘    │
│                                                        │
└────────────────────────────────────────────────────────┘
```

---

## Component Breakdown

### Component 1: PollTransport Class

**Purpose:** Wrap your poll server into a Transport implementation

**Interface Contract:**
```cpp
class PollTransport : public Transport {
public:
    // Constructor starts server
    PollTransport(Daemon& daemon, int port);
    
    // From Transport interface
    void send(uint64_t peer_id, const std::string& data) override;
    
    // Lifecycle
    void start();
    void stop();
    
    // Stats/debugging
    size_t num_clients() const;
    bool is_running() const;
    
private:
    void poll_loop();  // Your existing poll logic
    void handle_new_connection();
    void handle_client_data(int fd);
    void handle_client_write(int fd);
    void handle_client_disconnect(int fd);
    
    Daemon& daemon_;
    int server_fd_;
    bool running_;
    std::thread poll_thread_;
    std::vector<pollfd> poll_fds_;
    std::unordered_map<int, Client> clients_;
    mutable std::mutex clients_mtx_;
};
```

**Key Design Points:**

1. **Ownership:** PollTransport owns the poll thread
2. **Reference:** Takes Daemon by reference (not pointer) - guarantees lifetime
3. **Thread:** Poll loop runs in separate thread
4. **Synchronization:** Mutex protects shared client state
5. **Event Flow:** Poll thread → enqueue_event() → Daemon

**Integration Points:**

- When `poll()` detects `POLLIN` on client fd:
  ```cpp
  // Read data
  int n = recv(fd, buf, size, 0);
  
  // Create event
  Daemon::Event evt;
  evt.event_type = Daemon::EventType::dataReceived;
  evt.peerid = fd;  // Use fd as peer ID
  evt.peerData = std::string(buf, n);
  
  // Send to daemon
  daemon_.enqueue_event(std::move(evt));
  ```

- When daemon calls `transport->send()`:
  ```cpp
  void PollTransport::send(uint64_t peer_id, const std::string& data) {
      std::lock_guard<std::mutex> lock(clients_mtx_);
      
      int fd = static_cast<int>(peer_id);
      auto it = clients_.find(fd);
      if (it != clients_.end()) {
          // Add to write buffer
          it->second.writebuf += data;
          
          // Enable POLLOUT for this fd
          for (auto& pfd : poll_fds_) {
              if (pfd.fd == fd) {
                  pfd.events |= POLLOUT;
                  break;
              }
          }
      }
  }
  ```

---

### Component 2: Extended MeshCore API

**Current State:** Basic lifecycle only

**Needed Extensions:**

```c
// In meshcore_bridge.c

// Send message to specific peer
void meshcore_send_message(
    meshcore* core, 
    uint64_t peer_id, 
    const char* message
);

// Broadcast to all peers
void meshcore_broadcast(
    meshcore* core,
    const char* message
);

// Get next event (blocking or non-blocking)
// Returns NULL if no events
char* meshcore_poll_event(meshcore* core);

// Get list of connected peers
uint64_t* meshcore_get_peers(
    meshcore* core,
    size_t* out_count
);

// Set event callback (alternative to polling)
typedef void (*event_callback_t)(
    uint64_t peer_id,
    const char* event_type,
    const char* data,
    void* user_data
);

void meshcore_set_callback(
    meshcore* core,
    event_callback_t callback,
    void* user_data
);

// Network control
bool meshcore_start_server(meshcore* core, int port);
bool meshcore_connect_to_peer(meshcore* core, const char* ip, int port);
void meshcore_disconnect_peer(meshcore* core, uint64_t peer_id);
```

**Implementation Strategy:**

Each C function is a thin wrapper that:
1. Validates parameters
2. Casts `meshcore*` to access daemon
3. Calls appropriate daemon method
4. Returns result in C-compatible format

**Example:**
```c
void meshcore_send_message(meshcore* core, uint64_t peer_id, const char* msg) {
    if (!core || !core->daemon || !msg) return;
    
    // Create event for daemon to process
    Daemon::Event evt;
    evt.event_type = Daemon::EventType::dataReceived;  // Or new type
    evt.peerid = peer_id;
    evt.peerData = std::string(msg);
    
    core->daemon->enqueue_event(std::move(evt));
}
```

---

### Component 3: JNI Glue Layer

**Purpose:** Bridge between Java/Kotlin and C code

**File Structure:**
```
app/src/main/cpp/
├── daemon.cpp          (your existing)
├── daemon.h            (your existing)
├── transport.h         (your existing)
├── loopback_transport.cpp  (your existing)
├── meshcore_impl.cpp   (your existing)
├── meshcore_bridge.c   (extended)
├── poll_transport.cpp  (new - wraps server.cpp)
├── poll_transport.h    (new)
└── jni_bridge.cpp      (new - JNI functions)
```

**JNI Naming Convention:**
```
Java method:  package.Class.method
JNI function: Java_package_Class_method

Example:
Java:  com.example.app.NativeBridge.sendMessage()
JNI:   Java_com_example_app_NativeBridge_sendMessage()
```

**JNI Function Template:**
```cpp
extern "C" JNIEXPORT <return_type> JNICALL
Java_<package>_<class>_<method>(
    JNIEnv* env,           // JNI environment
    jobject thiz,          // 'this' object in Java
    <parameters...>        // Your parameters
) {
    // 1. Convert Java types to C++ types
    // 2. Call C++ code
    // 3. Convert C++ result to Java types
    // 4. Return
}
```

**Example - Send Message:**
```cpp
extern "C" JNIEXPORT void JNICALL
Java_com_example_app_NativeBridge_sendMessage(
    JNIEnv* env,
    jobject thiz,
    jlong peerId,
    jstring message
) {
    // Convert jstring to C string
    const char* msg = env->GetStringUTFChars(message, nullptr);
    
    // Call C API
    meshcore_send_message(g_core, peerId, msg);
    
    // Release string
    env->ReleaseStringUTFChars(message, msg);
}
```

**Memory Management Rules:**
1. Java strings must be `GetStringUTFChars()` then `ReleaseStringUTFChars()`
2. Byte arrays must be `GetByteArrayElements()` then `ReleaseByteArrayElements()`
3. Objects returned to Java must be `NewGlobalRef()` to prevent GC
4. Global refs must be `DeleteGlobalRef()` when done

---

### Component 4: Kotlin Wrapper

**Purpose:** Provide idiomatic Kotlin API over raw JNI

**Design Pattern:**
```kotlin
class MeshCore private constructor() {
    
    private external fun nativeInit(): Long
    private external fun nativeDestroy(handle: Long)
    private external fun nativeSend(handle: Long, peerId: Long, msg: String)
    private external fun nativePollEvent(handle: Long): String?
    
    private var handle: Long = 0
    
    init {
        handle = nativeInit()
    }
    
    fun sendMessage(peerId: Long, message: String) {
        require(handle != 0L) { "MeshCore not initialized" }
        nativeSend(handle, peerId, message)
    }
    
    fun pollEvent(): Event? {
        val json = nativePollEvent(handle) ?: return null
        return Event.fromJson(json)
    }
    
    fun close() {
        if (handle != 0L) {
            nativeDestroy(handle)
            handle = 0
        }
    }
    
    companion object {
        init {
            System.loadLibrary("meshcore")
        }
    }
}

sealed class Event {
    data class PeerConnected(val peerId: Long) : Event()
    data class PeerDisconnected(val peerId: Long) : Event()
    data class MessageReceived(val peerId: Long, val message: String) : Event()
    
    companion object {
        fun fromJson(json: String): Event {
            // Parse JSON and create appropriate Event subclass
        }
    }
}
```

**Key Points:**
1. **RAII Pattern:** Init in constructor, cleanup in close()
2. **Type Safety:** Sealed classes for events
3. **Error Handling:** require() checks for valid state
4. **Library Loading:** Happens once in companion object

---

## Threading Model

### Thread Overview

```
┌─────────────────────────────────────────────────────┐
│               Android Process                       │
├─────────────────────────────────────────────────────┤
│                                                     │
│  Thread 1: Main/UI Thread                          │
│  ┌───────────────────────────────────────────────┐ │
│  │  - Kotlin UI code                             │ │
│  │  - Compose recomposition                      │ │
│  │  - Must never block!                          │ │
│  └─────────────┬─────────────────────────────────┘ │
│                │ JNI calls (quick, non-blocking)    │
│                │                                     │
│  Thread 2: Daemon Worker                           │
│  ┌─────────────▼─────────────────────────────────┐ │
│  │  - Processes event queue                      │ │
│  │  - Calls transport->send()                    │ │
│  │  - Handles business logic                     │ │
│  └─────────────┬─────────────────────────────────┘ │
│                │ calls                              │
│                │                                     │
│  Thread 3: Poll Thread                             │
│  ┌─────────────▼─────────────────────────────────┐ │
│  │  - poll() on sockets                          │ │
│  │  - recv()/send() actual I/O                   │ │
│  │  - Enqueues events to daemon                  │ │
│  └───────────────────────────────────────────────┘ │
│                                                     │
│  Optional: Thread 4+ (Coroutines)                  │
│  ┌───────────────────────────────────────────────┐ │
│  │  - Kotlin coroutines for async UI operations  │ │
│  │  - Polling events from native code            │ │
│  └───────────────────────────────────────────────┘ │
│                                                     │
└─────────────────────────────────────────────────────┘
```

### Thread Responsibilities

**Main Thread:**
- **DO:** Update UI, handle user input, dispatch to background
- **DON'T:** Block, do I/O, heavy computation
- **Pattern:** Quick JNI calls that return immediately

**Daemon Worker:**
- **DO:** Process events, coordinate actions, call transport
- **DON'T:** Direct I/O (that's transport's job), block on locks
- **Pattern:** Event loop with condition variable

**Poll Thread:**
- **DO:** Network I/O, socket management, event detection
- **DON'T:** Heavy computation, UI calls
- **Pattern:** poll() loop with timeout

### Synchronization Points

1. **Event Queue (Daemon)**
   - Protected by: `daemon.mtx`
   - Writers: Poll thread, Main thread (via JNI)
   - Reader: Daemon worker
   - Mechanism: Mutex + condition variable

2. **Client Map (PollTransport)**
   - Protected by: `clients_mtx_`
   - Writers: Poll thread
   - Readers: Daemon worker (via send())
   - Mechanism: Mutex

3. **Poll FDs (PollTransport)**
   - Protected by: `clients_mtx_` (same as client map)
   - Writers: Poll thread
   - Readers: Poll thread only
   - Mechanism: Mutex (shared with client map)

### Deadlock Prevention

**Rule 1:** Always lock in same order
```
If you need both locks:
  1. Lock clients_mtx_ first
  2. Lock daemon.mtx second
  3. Unlock in reverse order
```

**Rule 2:** Don't call daemon from inside transport with lock held
```cpp
// BAD:
void PollTransport::poll_loop() {
    std::lock_guard<std::mutex> lock(clients_mtx_);
    daemon_.enqueue_event(evt);  // daemon might lock its mutex!
}

// GOOD:
void PollTransport::poll_loop() {
    Event evt;
    {
        std::lock_guard<std::mutex> lock(clients_mtx_);
        // prepare evt
    }
    daemon_.enqueue_event(evt);  // no locks held
}
```

**Rule 3:** Use RAII lock guards
```cpp
std::lock_guard<std::mutex> lock(mtx);  // Automatic unlock on scope exit
```

---

## Data Flow Diagrams

### Message Send Flow

```
User types message in UI
        │
        ▼
┌───────────────────┐
│ Kotlin UI         │ setText("Hello")
│ onSendClick()     │
└────────┬──────────┘
         │ meshCore.sendMessage(1, "Hello")
         ▼
┌───────────────────┐
│ Kotlin Wrapper    │ nativeSend(handle, 1, "Hello")
└────────┬──────────┘
         │ JNI call
         ▼
┌────────────────────┐
│ JNI Glue           │ Java_..._sendMessage(...)
│                    │   meshcore_send_message(core, 1, "Hello")
└────────┬───────────┘
         │
         ▼
┌────────────────────┐
│ MeshCore Bridge    │ Creates Event{type=send, peer=1, data="Hello"}
│                    │   core->daemon->enqueue_event(evt)
└────────┬───────────┘
         │
         ▼
┌────────────────────┐
│ Daemon             │ work_queue.push(evt)
│ Event Queue        │ cv.notify_one()
└────────────────────┘
         │
         │ (Worker thread wakes up)
         ▼
┌────────────────────┐
│ Daemon Worker      │ evt = work_queue.pop()
│                    │ switch(evt.type)
│                    │   case send:
│                    │     transport->send(1, "Hello")
└────────┬───────────┘
         │
         ▼
┌────────────────────┐
│ PollTransport      │ clients_[1].writebuf += "Hello"
│ send()             │ poll_fds[x].events |= POLLOUT
└────────────────────┘
         │
         │ (Next poll iteration)
         ▼
┌────────────────────┐
│ Poll Thread        │ poll() returns with POLLOUT
│                    │ send(fd, writebuf.data(), ...)
│                    │ writebuf.erase(0, sent)
└────────┬───────────┘
         │
         ▼
    Network packet
         │
         ▼
    Remote peer
```

### Message Receive Flow

```
Remote peer sends data
         │
         ▼
┌────────────────────┐
│ Poll Thread        │ poll() returns with POLLIN on fd=5
│                    │ n = recv(5, buf, size, 0)
└────────┬───────────┘
         │
         ▼
┌────────────────────┐
│ PollTransport      │ Event evt{
│                    │   type=dataReceived,
│                    │   peerid=5,
│                    │   data=buf
│                    │ }
│                    │ daemon_.enqueue_event(evt)
└────────┬───────────┘
         │
         ▼
┌────────────────────┐
│ Daemon             │ work_queue.push(evt)
│ Event Queue        │ cv.notify_one()
└────────────────────┘
         │
         ▼
┌────────────────────┐
│ Daemon Worker      │ evt = work_queue.pop()
│                    │ switch(evt.type)
│                    │   case dataReceived:
│                    │     // Store in result queue for JNI
│                    │     result_queue.push(evt)
└────────┬───────────┘
         │
         │ (Meanwhile, UI is polling...)
         ▼
┌────────────────────┐
│ Kotlin Coroutine   │ while(true) {
│ (background)       │   delay(100ms)
│                    │   val msg = meshCore.pollEvent()
│                    │   if (msg != null) emit(msg)
│                    │ }
└────────┬───────────┘
         │
         ▼
┌────────────────────┐
│ JNI Glue           │ Java_..._pollEvent(...)
│                    │   json = meshcore_poll_event(core)
│                    │   return env->NewStringUTF(json)
└────────┬───────────┘
         │
         ▼
┌────────────────────┐
│ MeshCore Bridge    │ if (!result_queue.empty())
│                    │   evt = result_queue.pop()
│                    │   return evt.toJson()
└────────┬───────────┘
         │
         ▼
┌────────────────────┐
│ Kotlin Wrapper     │ Event.fromJson(json)
│                    │ return MessageReceived(5, "Hello")
└────────┬───────────┘
         │
         ▼
┌────────────────────┐
│ Kotlin Flow        │ _messages.emit(msg)
└────────┬───────────┘
         │
         ▼
┌────────────────────┐
│ Compose UI         │ messages.collectAsState()
│                    │ LazyColumn { items(messages) }
│                    │ → Screen updates!
└────────────────────┘
```

---

## Implementation Roadmap

### Phase 0: Preparation (Before coding)

**Goal:** Understand what you have and plan the integration

**Tasks:**
1. Document current daemon behavior
   - What events does it handle?
   - What's the lifecycle (start/stop)?
   - What happens on errors?

2. Document poll server behavior
   - How are clients added/removed?
   - What's the message protocol?
   - How does it handle disconnections?

3. Identify integration points
   - Where does poll → daemon happen?
   - Where does daemon → poll happen?
   - What data crosses boundaries?

4. Draw state diagrams
   - Client connection states
   - Daemon worker states
   - Overall system states

**Deliverable:** Architecture document (this!)

---

### Phase 1: PollTransport Implementation

**Goal:** Wrap poll server logic into Transport interface

**Step 1.1: Create empty class**
```cpp
class PollTransport : public Transport {
private:
    Daemon& daemon_;
    
public:
    explicit PollTransport(Daemon& d) : daemon_(d) {}
    
    void send(uint64_t peer_id, const std::string& data) override {
        // TODO
    }
};
```

**Step 1.2: Add server initialization**
- Port your socket creation code
- Port your bind/listen code
- Store server_fd as member variable

**Step 1.3: Add poll loop skeleton**
```cpp
void PollTransport::poll_loop() {
    while (running_) {
        int ready = poll(poll_fds_.data(), poll_fds_.size(), timeout);
        
        // TODO: handle events
    }
}
```

**Step 1.4: Handle new connections**
- Detect POLLIN on server_fd
- Call accept()
- Add to poll_fds_ and clients_ map
- Emit peerConnected event to daemon

**Step 1.5: Handle client data**
- Detect POLLIN on client fd
- Call recv()
- Emit dataReceived event to daemon

**Step 1.6: Handle client writes**
- Implement send() to queue data
- Detect POLLOUT on client fd
- Call send() syscall
- Remove POLLOUT if buffer empty

**Step 1.7: Handle disconnections**
- Detect POLLHUP or recv() == 0
- Clean up client state
- Emit peerDisconnected event

**Testing:**
- Unit test: Create PollTransport, verify it starts
- Integration test: Connect with netcat, send data
- Stress test: 10+ simultaneous clients

---

### Phase 2: MeshCore API Extension

**Goal:** Add functions for Android to call

**Step 2.1: Add internal result queue to daemon**
```cpp
class Daemon {
private:
    std::queue<Event> result_queue_;  // Events for external consumption
    std::mutex result_mtx_;
    
public:
    Event poll_result_event() {
        std::lock_guard<std::mutex> lock(result_mtx_);
        if (result_queue_.empty()) return {};
        Event evt = result_queue_.front();
        result_queue_.pop();
        return evt;
    }
};
```

**Step 2.2: Modify daemon worker to fill result queue**
```cpp
void Daemon::loopy() {
    while (running) {
        Event evt = work_queue.pop();
        
        // Process event
        // ...
        
        // If it's for external consumption, queue it
        if (evt.event_type == EventType::dataReceived) {
            std::lock_guard<std::mutex> lock(result_mtx_);
            result_queue_.push(evt);
        }
    }
}
```

**Step 2.3: Add C API functions**
```c
char* meshcore_poll_event(meshcore* core) {
    if (!core || !core->daemon) return nullptr;
    
    Event evt = core->daemon->poll_result_event();
    if (evt.empty()) return nullptr;
    
    // Convert to JSON
    char* json = event_to_json(&evt);
    return json;  // Caller must free!
}
```

**Step 2.4: Add send function**
```c
void meshcore_send_message(meshcore* core, uint64_t peer, const char* msg) {
    // Create event
    // Enqueue to daemon
}
```

**Testing:**
- Call functions directly from C++ test harness
- Verify event flow: send → daemon → transport → network
- Verify event flow: network → transport → daemon → poll

---

### Phase 3: JNI Bridge

**Goal:** Make C++ callable from Kotlin

**Step 3.1: Create CMakeLists.txt**
```cmake
add_library(meshcore SHARED
    daemon.cpp
    poll_transport.cpp
    meshcore_impl.cpp
    meshcore_bridge.c
    jni_bridge.cpp
)

target_link_libraries(meshcore log android)
```

**Step 3.2: Write basic JNI functions**
```cpp
// Global state (careful! Android can kill process)
static meshcore* g_core = nullptr;

extern "C" JNIEXPORT jlong JNICALL
Java_com_example_NativeBridge_nativeInit(JNIEnv* env, jobject) {
    g_core = meshcore_create();
    meshcore_start_server(g_core, 9669);
    return reinterpret_cast<jlong>(g_core);
}
```

**Step 3.3: Add send/receive functions**

**Step 3.4: Add proper error handling**
- Check for null pointers
- Handle exceptions (use try/catch, don't let C++ exceptions cross JNI boundary!)
- Return error codes or throw Java exceptions

**Testing:**
- Load library from Java test
- Call init, verify it returns handle
- Call send, verify no crash
- Call poll, verify it returns data

---

### Phase 4: Kotlin Wrapper

**Goal:** Idiomatic Kotlin API

**Step 4.1: Create sealed Event class**

**Step 4.2: Create MeshCore wrapper**

**Step 4.3: Add Flow-based API**
```kotlin
class MeshCore {
    val events: Flow<Event> = flow {
        while (true) {
            delay(100)
            val evt = pollEvent()
            if (evt != null) emit(evt)
        }
    }
}
```

**Testing:**
- Write Kotlin unit tests
- Use Robolectric for Android-free testing
- Mock JNI layer if needed

---

### Phase 5: UI Layer

**Goal:** Show it working

**Step 5.1: Simple chat screen**
```kotlin
@Composable
fun ChatScreen(meshCore: MeshCore) {
    val messages by meshCore.events
        .filterIsInstance<Event.MessageReceived>()
        .map { it.message }
        .collectAsState(initial = emptyList())
    
    LazyColumn {
        items(messages) { msg ->
            Text(msg)
        }
    }
}
```

**Step 5.2: Input field**

**Step 5.3: Connection status**

---

### Phase 6: Polish

**Goal:** Make it production-ready

- Error handling
- Reconnection logic
- Proper lifecycle management (onPause, onDestroy)
- Background service (for persistent connection)
- Notifications
- Settings screen
- Logging/debugging

---

## Design Challenges & Solutions

### Challenge 1: Android Lifecycle

**Problem:** Android can kill your app at any time

**Solutions:**

1. **Foreground Service**
   - Keeps app alive even when in background
   - Shows persistent notification
   - User sees app is running

2. **Graceful Shutdown**
   ```kotlin
   override fun onDestroy() {
       super.onDestroy()
       meshCore.close()  // Stops threads, closes sockets
   }
   ```

3. **State Persistence**
   - Save peer list to SharedPreferences
   - Restore connections on restart
   - Handle "process death" scenario

---

### Challenge 2: Battery Life

**Problem:** Constant polling drains battery

**Solutions:**

1. **Adaptive Polling**
   ```cpp
   // Poll frequently when active
   int timeout = user_active ? 100 : 5000;
   poll(fds, nfds, timeout);
   ```

2. **Wake Locks**
   - Only hold wake lock when messages pending
   - Release immediately after processing

3. **Doze Mode**
   - Use Firebase Cloud Messaging for wake-up
   - Or exempt app from battery optimization (user must approve)

---

### Challenge 3: Network Changes

**Problem:** WiFi connects/disconnects, IP changes

**Solutions:**

1. **Network Callback**
   ```kotlin
   val networkCallback = object : ConnectivityManager.NetworkCallback() {
       override fun onAvailable(network: Network) {
           // Reconnect
       }
       override fun onLost(network: Network) {
           // Clean up
       }
   }
   ```

2. **Heartbeat**
   ```cpp
   // Send keepalive every 30s
   // If no response in 90s, assume disconnected
   ```

3. **Exponential Backoff**
   ```kotlin
   var retry_delay = 1000
   while (!connected) {
       delay(retry_delay)
       tryConnect()
       retry_delay = min(retry_delay * 2, 60000)
   }
   ```

---

### Challenge 4: Peer Discovery

**Problem:** How does phone find server?

**Solutions:**

1. **Manual IP Entry**
   - Simplest
   - User types server IP
   - Good for testing

2. **mDNS/Bonjour**
   - Automatic LAN discovery
   - Servers advertise themselves
   - Library: jmdns (Java implementation)

3. **QR Code**
   - Server shows QR with IP:port
   - Phone scans to connect
   - Library: ZXing

4. **NFC/Bluetooth**
   - Tap phones to exchange connection info
   - Then communicate over WiFi

---

### Challenge 5: Security

**Problem:** No encryption in your current implementation

**Solutions:**

1. **TLS/SSL**
   - Use OpenSSL in C++
   - Or BoringSSL (Android's version)
   - Wrap sockets with SSL_read/SSL_write

2. **Pre-shared Key**
   - Symmetric encryption
   - Lighter than TLS
   - Use libsodium

3. **Trust Model**
   - First connection: exchange public keys
   - Subsequent: verify signature
   - "Trust on first use" (like SSH)

---

## Testing Strategy

### Unit Tests (C++)

**Tools:** Google Test, Catch2

**Test Daemon:**
```cpp
TEST(DaemonTest, EnqueueEvent) {
    Daemon daemon;
    daemon.start();
    
    Daemon::Event evt{Daemon::EventType::peerConnected, 1, ""};
    daemon.enqueue_event(evt);
    
    // Wait for processing
    while (daemon.is_busy()) { std::this_thread::sleep_for(10ms); }
    
    EXPECT_TRUE(/* event was processed */);
}
```

**Test PollTransport:**
```cpp
TEST(PollTransportTest, ClientConnect) {
    MockDaemon daemon;
    PollTransport transport(daemon, 9999);
    
    // Connect a test client
    int sock = socket(...);
    connect(sock, ...);
    
    // Verify daemon received peerConnected event
    EXPECT_EQ(daemon.events.size(), 1);
    EXPECT_EQ(daemon.events[0].event_type, EventType::peerConnected);
}
```

---

### Integration Tests (C++ + JNI)

**Tools:** Android JUnit, Robolectric

**Test JNI Bridge:**
```kotlin
@Test
fun testNativeInit() {
    val handle = NativeBridge.nativeInit()
    assertNotEquals(0, handle)
    
    NativeBridge.nativeDestroy(handle)
}

@Test
fun testSendMessage() {
    val handle = NativeBridge.nativeInit()
    
    // Should not crash
    NativeBridge.nativeSend(handle, 1, "test")
    
    NativeBridge.nativeDestroy(handle)
}
```

---

### UI Tests (Android)

**Tools:** Espresso, Compose Testing

**Test Chat Screen:**
```kotlin
@Test
fun testSendMessage() {
    composeTestRule.setContent {
        ChatScreen(mockMeshCore)
    }
    
    composeTestRule.onNodeWithText("Type message...")
        .performTextInput("Hello")
    
    composeTestRule.onNodeWithContentDescription("Send")
        .performClick()
    
    verify(mockMeshCore).sendMessage(any(), eq("Hello"))
}
```

---

### Manual Testing Checklist

- [ ] App installs
- [ ] Server starts on app launch
- [ ] Can connect from another device
- [ ] Messages send successfully
- [ ] Messages receive successfully
- [ ] Multiple clients can connect
- [ ] Client disconnect doesn't crash server
- [ ] App survives rotation
- [ ] App survives backgrounding
- [ ] Network change handled gracefully
- [ ] Low battery doesn't kill connection
- [ ] Works on different Android versions (7.0+)

---

## Performance Considerations

### Memory Usage

**Monitor:**
- Event queue size (should stay small)
- Client buffers (limit to reasonable size, e.g., 1MB)
- JNI local references (free after use)

**Optimize:**
```cpp
// Limit buffer size
const size_t MAX_BUFFER = 1024 * 1024;  // 1MB
if (client.writebuf.size() > MAX_BUFFER) {
    // Drop old data or disconnect slow client
}

// Preallocate if size known
std::string buf;
buf.reserve(1024);  // Avoid repeated allocations
```

---

### CPU Usage

**Profile:**
- Android Studio Profiler
- Systrace
- Simpleperf

**Hotspots:**
- poll() timeout (adjust based on activity)
- String copies (use move semantics)
- JSON parsing (consider binary protocol)

**Optimize:**
```cpp
// Use move to avoid copies
daemon.enqueue_event(std::move(evt));  // Not: daemon.enqueue_event(evt);

// Reuse buffers
static thread_local std::string buffer;
buffer.clear();
buffer.append(...);
```

---

### Network Efficiency

**Batch messages:**
```cpp
// Instead of sending each message immediately:
// send("msg1"); send("msg2"); send("msg3");

// Batch them:
writebuf += "msg1\n";
writebuf += "msg2\n";
writebuf += "msg3\n";
// poll() handles sending all at once
```

**Compress if needed:**
- Use zlib for compression
- Only for large messages
- Trade CPU for bandwidth

---

## Appendix: Key Concepts

### Event-Driven Architecture

Your daemon implements this pattern:
```
Instead of:  while(true) { check_for_input(); }
You have:    wait_for_event(); handle(event);
```

Benefits:
- CPU sleeps when idle
- Responsive to multiple inputs
- Scales to many connections

---

### Producer-Consumer Pattern

Your event queue:
```
Producers: UI thread, Poll thread, Timers
Consumer:  Daemon worker thread
Queue:     Thread-safe buffer between them
```

Why:
- Decouples producers from consumer
- Producers don't wait for processing
- Consumer processes at own pace

---

### Transport Abstraction

The key insight:
```
Daemon doesn't know HOW messages are sent.
It just calls: transport->send(peer, data);

Transport could be:
- Sockets (your poll implementation)
- WebSockets
- Bluetooth
- Carrier pigeons
```

Daemon code stays same regardless!

---

### JNI Lifecycle

Critical understanding:
```
Java → JNI → C++
     ↑     ↓
   Different memory management!

Java: Garbage collected
C++:  Manual (new/delete)

Rule: JNI is the bridge, manage memory carefully on both sides
```

---

## Final Thoughts

Your code isn't worthless - it's actually well-architected. The daemon pattern you built is exactly what's needed for an event-driven mobile app. The person who took over either didn't understand what you built, or used it in ways you didn't see.

This guide shows one way to use your code. There are others. The architecture is sound - the question is just how you want to apply it.

Building this will teach you:
- System architecture
- Threading and synchronization
- FFI boundaries (JNI)
- Mobile constraints
- Network programming at scale

That's way more valuable than the code itself.

Good luck. You've got this. 🚀
