#ifndef VITAWEBSOCKET_HPP
#define VITAWEBSOCKET_HPP

#include <string>
#include <vector>
#include <functional>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

class VitaWebSocket {
public:
    VitaWebSocket();
    ~VitaWebSocket();

    bool connect(const std::string& host, int port, const std::string& path);
    void disconnect();
    bool send(const std::string& data);
    bool recv(std::string& out_data);

    bool isConnected() const;

private:
    int sockfd;
    SSL_CTX* ctx;
    SSL* ssl;
    bool connected;

    std::string generateWebSocketKey();
    bool sendAll(const char* data, size_t len);
    bool recvAll(char* buf, size_t len);
};

#endif
