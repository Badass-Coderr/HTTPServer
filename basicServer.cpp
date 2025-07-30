#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>

// Forward declaration
class RequestHandler;

/**
 * @class Server
 * @brief Manages the network socket setup and the main server loop.
 */
class Server {
private:
    int port;
    int server_fd;

    void setup_socket() {
        // Create socket
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            throw std::runtime_error("Socket creation failed");
        }

        // Reuse address to avoid "Address already in use" errors
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            throw std::runtime_error("setsockopt failed");
        }

        // Bind socket
        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            throw std::runtime_error("Bind failed");
        }

        // Listen on the socket
        if (listen(server_fd, 10) < 0) {
            throw std::runtime_error("Listen failed");
        }
    }

public:
    Server(int port) : port(port), server_fd(-1) {
        setup_socket();
    }

    ~Server() {
        if (server_fd != -1) {
            close(server_fd);
        }
    }

    void run();
};

/**
 * @class RequestHandler
 * @brief Handles a single client connection.
 */
class RequestHandler {
private:
    int client_socket;

    // Parses the first line of an HTTP request to get the file path
    std::string parse_request() {
        char buffer[30000] = {0};
        read(client_socket, buffer, 30000);
        
        std::string request(buffer);
        size_t method_end = request.find(' ');
        size_t path_end = request.find(' ', method_end + 1);

        if (method_end == std::string::npos || path_end == std::string::npos) {
            return ""; // Invalid request format
        }

        std::string filepath = request.substr(method_end + 1, path_end - (method_end + 1));
        return (filepath == "/") ? "/index.html" : filepath;
    }

    // Sends a formatted HTTP response to the client
    void send_response(const std::string& response) {
        send(client_socket, response.c_str(), response.length(), 0);
    }
    
    // Creates an HTTP response string
    std::string create_http_response(const std::string& status, const std::string& content_type, const std::string& body) {
        std::stringstream response;
        response << "HTTP/1.1 " << status << "\r\n";
        response << "Content-Type: " << content_type << "\r\n";
        response << "Content-Length: " << body.length() << "\r\n";
        response << "\r\n"; // Blank line separates headers from body
        response << body;
        return response.str();
    }

public:
    RequestHandler(int socket) : client_socket(socket) {}

    ~RequestHandler() {
        close(client_socket);
    }

    // Main method to process the client request
    void process_request() {
        std::string filepath = parse_request();
        if (filepath.empty()) {
            std::cout << "❌ Received an invalid request." << std::endl;
            return;
        }

        std::ifstream file("." + filepath);
        if (file.good()) {
            // 200 OK
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string body = buffer.str();
            std::string response = create_http_response("200 OK", "text/html", body);
            send_response(response);
            std::cout << "➡️  Sent response for " << filepath << std::endl;
        } else {
            // 404 Not Found
            std::string body = "<h1>404 Not Found</h1>";
            std::string response = create_http_response("404 Not Found", "text/html", body);
            send_response(response);
            std::cout << "❌ File not found: " << filepath << std::endl;
        }
    }
};

// Implementation of Server::run() which needs RequestHandler
void Server::run() {
    std::cout << "🚀 Server is listening on port " << port << "..." << std::endl;
    while (true) {
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }
        std::cout << "✅ Connection accepted..." << std::endl;
        RequestHandler handler(client_socket);
        handler.process_request();
    }
}

int main() {
    try {
        Server my_server(8080);
        my_server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}