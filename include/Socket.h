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
        std::vector<int> iAccSocket;
        int iMaxNumberOfClientConnections;
    };

    enum class ConnectionStatus : int
    {
        Error = -1,
        NotConnected,
        Connected
    };

    enum class MessageStatus : int
    {
        Timeout = -1,
        NotConnected
    };

    // Sets a Timeout for the blocking recv function, which will
    // return -1 (or MessageStatus::Timeout) case it is reached.
    void SetTimeout(SocketStructure& socketStructure);

    static constexpr int BUFFER_SIZE = 4096;
    static constexpr int SOCKETS_TIMEOUT = 20;
};

class SocketServer : public Socket
{
public:
    // Constructor and destructor
    SocketServer(const std::string& cIpAddress, int iPort, int iMaxNumberOfClients = 1);
    ~SocketServer();

    // Threads Starter
    void ServerThread();

    // Write Method
    int Write(const std::string& message, int iThreadIndex);

private:
    // Multiple connection threads manager
    int ServerThreadsManager(int iThreadIndex);
    int ServerStart(int iThreadIndex);
    void ServerEnd(int iThreadIndex);
    
    // Read Method
    void Read(int iThreadIndex);

    // Callbacks on message and fail to send.
    // These two methods are virtual and require
    // implementation within a child class.
    virtual int CallbackOnMessage(const std::string& message) { return 1; };
    virtual int CallbackOnSendError(const std::string& message) { return 1; };

    // State checkers
    bool IsServerRunning() { return m_bThreadRunning; }
    bool IsReconnectEnabled() { return m_bEnableReconnect; }
    
    // Private variables
    SocketStructure serverStructure;
    bool m_bThreadRunning = true;
    bool m_bEnableReconnect = true;
    std::vector<bool> m_vThreadRunning;
    std::vector<std::thread> m_vServerThreadPool;
};

class SocketClient : public Socket
{
public:
    // Constructor and destructor
    SocketClient(const std::string& cIpAddress, int iPort);
    ~SocketClient();
    // Client thread starter
    int ClientThread();
    int Write(const std::string& message);

private:
    // Client connection manager
    int ClientStart();
    void ClientEnd();

    // Read method
    void Read();

    // Callbacks on message and fail to send.
    // These two methods are virtual and require
    // implementation within a child class.
    virtual int CallbackOnMessage(const std::string& message) { return 1; };
    virtual int CallbackOnSendError(const std::string& message) { return 1; };

    // State checkers
    bool IsClientRunning() { return m_bThreadRunning; }
    bool IsReconnectEnabled() { return m_bEnableReconnect; }
    
    // Private variables
    SocketStructure clientStructure;
    bool m_bThreadRunning = true;
    bool m_bEnableReconnect = true;
    std::vector<std::thread> m_vClientThreadPool;
};