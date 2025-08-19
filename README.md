# C++ HTTP High Performance Web Server

A high-performance, multi-threaded HTTP web server built from scratch in C++, demonstrating core concepts of low-level networking and concurrent programming.

## 📜 Overview

This project is a deep dive into the fundamentals of web infrastructure. It's a fully functional HTTP server built without relying on external networking libraries like Boost.Asio or Poco. The primary goal was to engage directly with the POSIX socket API to understand how network connections are managed at the operating system level and to explore different concurrency models for handling client requests efficiently.

The server is capable of serving static files (HTML, CSS, etc.) and is designed to handle multiple client connections simultaneously.

## ✨ Features

* **Low-Level Socket Programming:** Directly uses the POSIX socket API (`socket`, `bind`, `listen`, `accept`) for all TCP communication.
* **HTTP/1.1 GET Request Parsing:** A basic parser to handle incoming `GET` requests and identify the requested file path.
* **Static File Serving:** Serves files from the local directory to the client.
* **HTTP Response Crafting:** Generates valid `200 OK` and `404 Not Found` responses with appropriate headers.
* **Multi-threaded Architecture:** Employs a "one-thread-per-client" model to handle concurrent connections, preventing a single request from blocking the entire server.

## 🛠️ Tech Stack & Dependencies

* **Language:** C++ (using C++11 features for threading)
* **Core APIs:**
    * **POSIX Socket API:** For all network-related system calls.
    * **C++ Standard Library (`<thread>`):** For managing concurrent threads.
* **Compiler:** A C++ compiler like `g++` or `clang++`.
* **Operating System:** Developed and tested on a POSIX-compliant OS (e.g., Linux, macOS).

## 🚀 How to Compile and Run

1.  **Clone the repository:**
    ```bash
    git clone <your-repository-url>
    cd <your-repository-directory>
    ```

2.  **Compile the server:**
    Use a C++ compiler. Make sure to link the pthread library for multi-threading support.
    ```bash
    g++ server.cpp -o server -std=c++11 -pthread
    ```

3.  **Create a sample file:**
    The server looks for an `index.html` file by default. Create one in the same directory.
    ```bash
    echo "<h1>Hello from my C++ Server!</h1>" > index.html
    ```

4.  **Run the server:**
    ```bash
    ./server
    ```
    You should see the message: `🚀 Server is listening on port 8080...`

5.  **Test in your browser:**
    Open your web browser and navigate to `http://localhost:8080`.

## 📊 How to Test Performance

This server was built with performance in mind. You can use a load testing tool like `wrk` to benchmark its performance.

1.  **Install `wrk`:**
    * **Ubuntu/Debian:** `sudo apt-get install wrk`
    * **macOS (Homebrew):** `brew install wrk`

2.  **Run a benchmark:**
    While the server is running, open a second terminal and run the following command to simulate 50 concurrent users over 4 threads for 30 seconds.
    ```bash
    wrk -t4 -c50 -d30s http://localhost:8080/index.html
    ```

## 📈 Project Evolution & Learnings

This project was developed iteratively, with performance testing guiding the architecture.

1.  **Initial Single-Threaded Design:** The first version was a simple, single-threaded server. While functional for one user, load testing with `wrk` revealed that it was incapable of handling concurrent connections, leading to a complete failure under minimal load.

2.  **Transition to Multi-threading:** To address the concurrency issue, the architecture was refactored to a multi-threaded model. The main thread's sole responsibility became accepting new connections and spawning a new worker thread for each one. This significantly changed the server's design to handle concurrent workloads.

3.  **Uncovering a Deeper Bug:** After implementing multi-threading, further testing showed that while the server no longer blocked, it still produced a **100% read error rate** under load. This demonstrated that the root cause was not just the concurrency model but a more fundamental bug in the request-handling logic—specifically, the `read()` call was not robust enough to handle partial network data, causing the parser to fail. This was a critical learning experience in network programming and debugging.

## 🔮 Future Improvements

* **Fix the Request Parsing Bug:** Implement a more robust reading loop to ensure the entire HTTP request is received from the socket before attempting to parse it.
* **Implement a Thread Pool:** The current "one-thread-per-client" model is inefficient as creating and destroying threads is expensive. A thread pool would manage a fixed set of worker threads to improve performance and reduce overhead.
* **Support More HTTP Methods:** Extend the parser to handle `POST`, `PUT`, `DELETE`, etc.
* **Enhance Header Parsing:** Implement a more comprehensive parser for various HTTP headers.
* **Add Logging:** Integrate a logging mechanism to record requests, errors, and server status.