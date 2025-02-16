#include <atomic>
#include <functional>
#include <thread>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#include "osc_parser.h"

class OSCReceiver {
public:
    OSCReceiver(int port);

    void SetMessageCallback(std::function<void(const OSCParser::ParsedMessage&)> callback);

    void Start();

    void Stop();

private:
    int port_;
#ifdef _WIN32
    SOCKET sockfd_;
#else
    int sockfd_;
#endif

    std::function<void(const OSCParser::ParsedMessage&)> messageCallback_;

    std::atomic<bool> running_;
    std::thread listenerThread_;

    void Listen();
};