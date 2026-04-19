#include "VitaWebSocket.hpp"
#include <iostream>
#include <cstring>
#include <random>
#include <sstream>

extern "C" {
    #include "easyencryptor.hpp"
}
#include "log.hpp"

// We use OpenSSL for Base64 (from easyencryptor)
static std::string base64_encode(const std::string& in) {
    // A quick implementation using BIO could be used, or just simple mapping since it's only for WebSocket key.
    // Let's implement a simple base64 encoder for the WS key if not using external.
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

VitaWebSocket::VitaWebSocket() : sockfd(-1), ctx(nullptr), ssl(nullptr), connected(false) {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

VitaWebSocket::~VitaWebSocket() {
    disconnect();
}

std::string VitaWebSocket::generateWebSocketKey() {
    std::string key = "1234567890123456"; // 16 bytes random
    for(int i=0; i<16; i++){
        key[i] = rand() % 256;
    }
    return base64_encode(key);
}

bool VitaWebSocket::sendAll(const char* data, size_t len) {
    size_t sent = 0;
    while(sent < len) {
        int r = SSL_write(ssl, data + sent, len - sent);
        if (r <= 0) return false;
        sent += r;
    }
    return true;
}

bool VitaWebSocket::recvAll(char* buf, size_t len) {
    size_t recvd = 0;
    while(recvd < len) {
        int r = SSL_read(ssl, buf + recvd, len - recvd);
        if (r <= 0) return false;
        recvd += r;
    }
    return true;
}

bool VitaWebSocket::connect(const std::string& host, int port, const std::string& path) {
    disconnect();

    struct hostent *server = gethostbyname(host.c_str());
    if (server == nullptr) {
        return false;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return false;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    if (::connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd);
        sockfd = -1;
        return false;
    }

    const SSL_METHOD *method = TLS_client_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        close(sockfd);
        sockfd = -1;
        return false;
    }

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    SSL_set_tlsext_host_name(ssl, host.c_str());

    if (SSL_connect(ssl) <= 0) {
        disconnect();
        return false;
    }

    // Perform WebSocket Handshake
    std::string key = generateWebSocketKey();
    std::stringstream request;
    request << "GET " << path << " HTTP/1.1\r\n"
            << "Host: " << host << "\r\n"
            << "Upgrade: websocket\r\n"
            << "Connection: Upgrade\r\n"
            << "Sec-WebSocket-Key: " << key << "\r\n"
            << "Sec-WebSocket-Version: 13\r\n"
            << "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36\r\n"
            << "Origin: https://discord.com\r\n\r\n";

    if (!sendAll(request.str().c_str(), request.str().length())) {
        disconnect();
        return false;
    }

    // Read response header
    std::string response;
    char c;
    while (true) {
        int r = SSL_read(ssl, &c, 1);
        if (r <= 0) {
            disconnect();
            return false;
        }
        response += c;
        if (response.size() >= 4 && response.substr(response.size() - 4) == "\r\n\r\n") {
            break;
        }
    }

    // Check if 101 Switching Protocols
    if (response.find("101 Switching Protocols") == std::string::npos &&
        response.find("101") == std::string::npos) {
        disconnect();
        return false;
    }

    connected = true;
    return true;
}

void VitaWebSocket::disconnect() {
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        ssl = nullptr;
    }
    if (ctx) {
        SSL_CTX_free(ctx);
        ctx = nullptr;
    }
    if (sockfd >= 0) {
        close(sockfd);
        sockfd = -1;
    }
    connected = false;
}

bool VitaWebSocket::isConnected() const {
    return connected;
}

bool VitaWebSocket::send(const std::string& data) {
    if (!connected) return false;

    std::vector<uint8_t> frame;
    frame.push_back(0x81); // FIN + Text opcode

    size_t len = data.length();
    if (len <= 125) {
        frame.push_back((uint8_t)(len | 0x80)); // Mask bit
    } else if (len <= 65535) {
        frame.push_back(126 | 0x80);
        frame.push_back((len >> 8) & 0xFF);
        frame.push_back(len & 0xFF);
    } else {
        frame.push_back(127 | 0x80);
        for (int i = 7; i >= 0; i--) {
            frame.push_back((len >> (i * 8)) & 0xFF);
        }
    }

    // Masking key
    uint8_t mask[4] = { (uint8_t)(rand() % 256), (uint8_t)(rand() % 256), (uint8_t)(rand() % 256), (uint8_t)(rand() % 256) };
    frame.insert(frame.end(), mask, mask + 4);

    // Payload
    for (size_t i = 0; i < len; i++) {
        frame.push_back(data[i] ^ mask[i % 4]);
    }

    return sendAll((const char*)frame.data(), frame.size());
}

bool VitaWebSocket::recv(std::string& out_data) {
    if (!connected) return false;

    out_data.clear();
    char header[2];
    if (!recvAll(header, 2)) return false;

    uint8_t opcode = header[0] & 0x0F;
    bool is_masked = (header[1] & 0x80) != 0;
    uint64_t payload_len = header[1] & 0x7F;

    if (payload_len == 126) {
        char ext[2];
        if (!recvAll(ext, 2)) return false;
        payload_len = ((uint8_t)ext[0] << 8) | (uint8_t)ext[1];
    } else if (payload_len == 127) {
        char ext[8];
        if (!recvAll(ext, 8)) return false;
        payload_len = 0;
        for (int i = 0; i < 8; i++) {
            payload_len = (payload_len << 8) | (uint8_t)ext[i];
        }
    }

    uint8_t mask[4];
    if (is_masked) {
        if (!recvAll((char*)mask, 4)) return false;
    }

    std::vector<char> payload(payload_len);
    if (payload_len > 0) {
        if (!recvAll(payload.data(), payload_len)) return false;
    }

    if (is_masked) {
        for (size_t i = 0; i < payload_len; i++) {
            payload[i] ^= mask[i % 4];
        }
    }

    if (opcode == 0x1) {
        // Text
        out_data = std::string(payload.data(), payload_len);
        return true;
    } else if (opcode == 0x8) {
        // Close
        disconnect();
        return false;
    }

    // Ignore other opcodes (ping/pong) and continue receiving
    return recv(out_data);
}
