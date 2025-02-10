#include <thread>
#include <chrono>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

class Socket
{
public:
    Socket();
    ~Socket();
protected:
    struct SocketStructure
    {
        int iSocket;
        std::string ipAddress;
        int port;
        sockaddr_in serverAddress;
        int iAccSocket;
    };

    enum class ConnectionStatus : int
    {
        Error = -1,
        NotConnected,
        Connected
    };

    static constexpr int BUFFER_SIZE = 4096;
};

class SocketServer : public Socket
{
public:
    SocketServer(const std::string& cIpAddress, int iPort);
    ~SocketServer();
    int ServerStart();
    void ServerEnd();
    int Write(const std::string& message);
    bool IsServerRunning() { return m_bThreadRunning; }
private:
    SocketStructure serverStructure;
    void ReadThread();
    virtual int CallbackOnMessage(const std::string& message) { return 1; };
    virtual int CallbackOnSendError(const std::string& message) { return 1; };
    bool m_bThreadRunning = true;
    std::vector<std::thread> m_vServerThreadPool;
};

class SocketClient : public Socket
{
public:
    SocketClient(const std::string& cIpAddress, int iPort);
    ~SocketClient();
    int ClientThread();
    int ClientStart();
    void ClientEnd();
    int Write(const std::string& message);
    bool IsClientRunning() { return m_bThreadRunning; }
    bool IsReconnectEnabled() { return m_bEnableReconnect; }
private:
    SocketStructure clientStructure;
    void Read();
    virtual int CallbackOnMessage(const std::string& message) { return 1; };
    virtual int CallbackOnSendError(const std::string& message) { return 1; };
    bool m_bThreadRunning = true;
    bool m_bEnableReconnect = true;
    std::vector<std::thread> m_vClientThreadPool;
};