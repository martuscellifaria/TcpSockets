#include "Socket.h"

Socket::Socket()
{   
}

Socket::~Socket()
{
}

void Socket::SetTimeout(SocketStructure& socketStructure)
{
    struct timeval tv;
    tv.tv_sec = SOCKETS_TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(socketStructure.iSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
}

SocketServer::SocketServer(const std::string& cIpAddress, int iPort, int iMaxNumberOfClients)
{
    serverStructure.ipAddress = cIpAddress;
    serverStructure.port = iPort;
    serverStructure.iMaxNumberOfClientConnections = iMaxNumberOfClients;
    serverStructure.iSocket = socket(AF_INET, SOCK_STREAM, 0);

    SetTimeout(serverStructure);

    serverStructure.serverAddress.sin_family = AF_INET;
    serverStructure.serverAddress.sin_port = htons(iPort);
    serverStructure.serverAddress.sin_addr.s_addr = INADDR_ANY;

    for (int i = 0; i < iMaxNumberOfClients; i++)
    {
        m_vThreadRunning.push_back(false);
        serverStructure.iAccSocket.push_back((int)ConnectionStatus::Error);
    }

    bind
    (
        serverStructure.iSocket,
        (struct sockaddr*)&serverStructure.serverAddress,
        sizeof(serverStructure.serverAddress)
    );

    int connStatus = listen(serverStructure.iSocket, serverStructure.iMaxNumberOfClientConnections);
}

SocketServer::~SocketServer()
{
    m_bEnableReconnect = false;
    for (int i = 0; i < serverStructure.iMaxNumberOfClientConnections; i++)
    {
        m_vThreadRunning[i] = false;
        ServerEnd(i);
    }
    for (auto& thr : m_vServerThreadPool)
    {
        thr.join();
    }
}

void SocketServer::ServerThread()
{
    for (int i = 0; i < serverStructure.iMaxNumberOfClientConnections; i++)
    {
        std::thread mainServerThread(&SocketServer::ServerThreadsManager, this, i);
        m_vServerThreadPool.push_back(move(mainServerThread));
    }
}

int SocketServer::ServerThreadsManager(int iThreadIndex)
{
    int connStatus;

    while (IsReconnectEnabled())
    {
        connStatus = ServerStart(iThreadIndex);
        if (connStatus != (int)ConnectionStatus::Error)
        {
            Read(iThreadIndex);
            if (!m_vThreadRunning[iThreadIndex])
            {
                ServerEnd(iThreadIndex);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    return connStatus;
}

int SocketServer::ServerStart(int iThreadIndex)
{
    int connStatus = accept(serverStructure.iSocket, nullptr, nullptr);
    serverStructure.iAccSocket[iThreadIndex] = connStatus;
    m_vThreadRunning[iThreadIndex] = true;
    return connStatus;
}

void SocketServer::ServerEnd(int iThreadIndex)
{
    close(serverStructure.iAccSocket[iThreadIndex]);
    serverStructure.iAccSocket.erase(serverStructure.iAccSocket.begin() + iThreadIndex);
}

void SocketServer::Read(int iThreadIndex)
{
    while (m_vThreadRunning[iThreadIndex])
    {
        char buffer[BUFFER_SIZE] = { 0 };
        int recvStatus;
        recvStatus = recv(serverStructure.iAccSocket[iThreadIndex], &buffer, sizeof(buffer), 0);
        switch (recvStatus)
        {
            case (int)MessageStatus::NotConnected:
            {
                m_vThreadRunning[iThreadIndex] = false;
                break;
            }
            case (int)MessageStatus::Timeout:
            {
                break;
            }
            default:
            {
                CallbackOnMessage(buffer);
                break;
            }
        }
    }
}

int SocketServer::Write(const std::string& message, int iThreadIndex)
{
    int sendStatus = send(serverStructure.iAccSocket[iThreadIndex], message.c_str(), strlen(message.c_str()), 0);
    if (sendStatus == (int)ConnectionStatus::Error)
    {
        CallbackOnSendError(message);
    }
    return sendStatus;
}

SocketClient::SocketClient(const std::string& cIpAddress, int iPort)
{
    clientStructure.ipAddress = cIpAddress;
    clientStructure.port = iPort;
}
SocketClient::~SocketClient()
{
}

int SocketClient::ClientThread()
{
    int connStatus;
    while (IsReconnectEnabled())
    {
        connStatus = ClientStart();
        if (connStatus != (int)ConnectionStatus::Error)
        {
            Read();
            if (!m_bThreadRunning)
            {
                ClientEnd();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    return 1;
}

int SocketClient::ClientStart()
{
    int connStatus;

    clientStructure.iSocket = socket(AF_INET, SOCK_STREAM, 0);
    clientStructure.serverAddress.sin_family = AF_INET;
    clientStructure.serverAddress.sin_port = htons(clientStructure.port);
    clientStructure.serverAddress.sin_addr.s_addr = INADDR_ANY;

    connStatus = connect
    (
        clientStructure.iSocket,
        (struct sockaddr*)&clientStructure.serverAddress,
        sizeof(clientStructure.serverAddress)
    );

    if (connStatus != (int)ConnectionStatus::Error)
    {
        m_bThreadRunning = true;
    }

    return connStatus;
}

void SocketClient::ClientEnd()
{ 
    close(clientStructure.iSocket);
}

void SocketClient::Read()
{
    while (m_bThreadRunning)
    {
        char buffer[BUFFER_SIZE] = { 0 };
        int recvStatus;
        recvStatus = recv(clientStructure.iSocket, &buffer, sizeof(buffer), 0);
        if (recvStatus > (int)ConnectionStatus::NotConnected)
        {
            CallbackOnMessage(buffer);
        }
        else
        {
            m_bThreadRunning = false;
        }
    }
}

int SocketClient::Write(const std::string& message)
{
    int sendStatus = (int)send(clientStructure.iSocket, message.c_str(), strlen(message.c_str()), 0);
    if (sendStatus == (int)ConnectionStatus::NotConnected)
    {
        CallbackOnSendError(message);
    }
    return sendStatus;
}