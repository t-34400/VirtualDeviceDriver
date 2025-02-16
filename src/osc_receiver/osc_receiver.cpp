#include "osc_receiver.h"

#include <thread>
#include <atomic>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSESOCKET(s) closesocket(s)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define CLOSESOCKET(s) close(s)
#endif

#include "openvr_driver.h"
#include "driverlog.h"

#include "osc_parser.h"
#include <functional>

OSCReceiver::OSCReceiver(int port) : port_(port), running_(false) {}

void OSCReceiver::SetMessageCallback(std::function<void(const OSCParser::ParsedMessage&)> callback) {
    messageCallback_ = std::move(callback);
}

void OSCReceiver::Start() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        DriverLog("WSAStartup failed");
        return;
    }
#endif
    
    if (running_) return;

    running_ = true;
    listenerThread_ = std::thread(&OSCReceiver::Listen, this);

    DriverLog("OSC Receiver started on port %d", port_);
}

void OSCReceiver::Stop() {
    if (!running_) return;

    running_ = false;

    if (sockfd_ != -1) {
#ifdef _WIN32
        shutdown(sockfd_, SD_BOTH);
#else
        shutdown(sockfd_, SHUT_RDWR);
#endif
        CLOSESOCKET(sockfd_);
        sockfd_ = -1;
    }

    running_ = false;
    if (listenerThread_.joinable()) {
        listenerThread_.join();
    }
    DriverLog("OSC Receiver stopped.");

#ifdef _WIN32
    WSACleanup();
#endif
}

void OSCReceiver::Listen() {
#ifdef _WIN32
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
#else
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
#endif

    if (sockfd_ < 0) {
        DriverLog("Failed to create socket");
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port_);

    if (bind(sockfd_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        DriverLog("Failed to bind socket");
        CLOSESOCKET(sockfd_);
        return;
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500000;

#ifdef _WIN32
    setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#else
    setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, (const void*)&timeout, sizeof(timeout));
#endif

    DriverLog("Listening for OSC messages on port %d...", port_);

    char buffer[1024];
    while (running_) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int receivedBytes = recvfrom(sockfd_, buffer, sizeof(buffer) - 1, 0,
                                      (struct sockaddr*)&clientAddr, &clientLen);
        if (receivedBytes > 0) {
            OSCParser::ParsedMessage message = OSCParser::Parse(buffer, receivedBytes);
            if (messageCallback_) {
                messageCallback_(message);
            }
        } else if (receivedBytes < 0) {
#ifdef _WIN32
            int errCode = WSAGetLastError();
            if (errCode != WSAEWOULDBLOCK && errCode != WSAETIMEDOUT) {
                DriverLog("recvfrom() failed with error: %d", errCode);
            }
#else
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                DriverLog("recvfrom() failed with error: %d", errno);
            }
#endif
        }
    }
    
    running_ = false;
    CLOSESOCKET(sockfd_);
    DriverLog("Stopped listening for OSC messages.");
}