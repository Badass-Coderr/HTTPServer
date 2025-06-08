#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

// Function to parse HTTP request and extract method and path
std::pair<std::string, std::string> parse_request(const std::string& request) {
    std::stringstream ss(request);
    std::string method, path, protocol;
    ss >> method >> path >> protocol;
    return {method, path};
}

// Function to generate HTTP response
std::string create_response(const std::string& status, const std::string& content) {
    std::string response = "HTTP/1.1 " + status + "\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + std::to_string(content.length()) + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += content;
    return response;
}

int main() {
    // Step 1: Create a socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return 1;
    }

    // Step 2: Set socket options (allow port reuse)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cerr << "Failed to set socket options: " << strerror(errno) << std::endl;
        close(server_fd);
        return 1;
    }

    // Step 3: Bind socket to address and port
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to all interfaces
    server_addr.sin_port = htons(8080); // Port 8080
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Failed to bind: " << strerror(errno) << std::endl;
        close(server_fd);
        return 1;
    }

    // Step 4: Listen for incoming connections
    if (listen(server_fd, 10) == -1) {
        std::cerr << "Failed to listen: " << strerror(errno) << std::endl;
        close(server_fd);
        return 1;
    }
    std::cout << "Server listening on port 8080..." << std::endl;

    while (true) {
        // Step 5: Accept client connection
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
            continue;
        }

        // Step 6: Read client request
        char buffer[1024] = {0};
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            std::cerr << "Failed to read request: " << strerror(errno) << std::endl;
            close(client_fd);
            continue;
        }

        std::string request(buffer);
        auto [method, path] = parse_request(request);

        // Step 7: Generate response
        std::string response;
        if (method == "GET" && path == "/") {
            std::string content = "<html><body><h1>Hello, World!</h1></body></html>";
            response = create_response("200 OK", content);
        } else {
            std::string content = "<html><body><h1>404 Not Found</h1></body></html>";
            response = create_response("404 Not Found", content);
        }

        // Step 8: Send response
        write(client_fd, response.c_str(), response.length());

        // Step 9: Close client connection
        close(client_fd);
    }

    // Step 10: Clean up server socket (unreachable in this loop, but good practice)
    close(server_fd);
    return 0;
}