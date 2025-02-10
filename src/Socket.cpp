#include "Socket.h"

Socket::Socket()
{   
}

Socket::~Socket()
{
}

SocketServer::SocketServer(const std::string& cIpAddress, int iPort)
{
    serverStructure.ipAddress = cIpAddress;
    serverStructure.port = iPort;
    serverStructure.iSocket = socket(AF_INET, SOCK_STREAM, 0);
    serverStructure.serverAddress.sin_family = AF_INET;
    serverStructure.serverAddress.sin_port = htons(iPort);
    serverStructure.serverAddress.sin_addr.s_addr = INADDR_ANY;
}

SocketServer::~SocketServer()
{
}

int SocketServer::ServerStart()
{
    int connStatus;
    connStatus = bind
    (
        serverStructure.iSocket,
        (struct sockaddr*)&serverStructure.serverAddress,
        sizeof(serverStructure.serverAddress)
    );

    connStatus = listen(serverStructure.iSocket, 1);

    serverStructure.iAccSocket = accept(serverStructure.iSocket, nullptr, nullptr);

    m_bThreadRunning = true;

    std::thread serverThread(&SocketServer::ReadThread, this);
    m_vServerThreadPool.emplace_back(std::move(serverThread));

    return connStatus;
}

void SocketServer::ServerEnd()
{
    for (auto& thr : m_vServerThreadPool)
    {
        thr.detach();
    }
    close(serverStructure.iSocket);

}

void SocketServer::ReadThread()
{
    while (m_bThreadRunning)
    {
        char buffer[BUFFER_SIZE] = { 0 };
        int recvStatus;
        recvStatus = recv(serverStructure.iAccSocket, &buffer, sizeof(buffer), 0);
        if (recvStatus > (int)ConnectionStatus::NotConnected)
        {
            CallbackOnMessage(buffer);
            std::cout << "Message from client: " << buffer<< '\n';
        }
        else
        {
            m_bThreadRunning = false;
        }
    }
    ServerEnd();
}

int SocketServer::Write(const std::string& message)
{
    int sendStatus = send(serverStructure.iAccSocket, message.c_str(), strlen(message.c_str()), 0);
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
            ReadThread();
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
            std::cout << "Message from server: " << buffer << '\n';
        }
        else
        {
            std::cout << "Server down" << buffer << '\n';
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