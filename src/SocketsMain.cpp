#include <iostream>
#include <memory>
#include "../include/Socket.h"

int main()
{
    //std::shared_ptr<SocketServer> sockServer(new SocketServer("127.0.0.1", 8080));
    std::shared_ptr<SocketClient> sockClient(new SocketClient("127.0.0.1", 8080));

    //std::thread mainServerThread(&SocketServer::ServerStart, sockServer);
    std::thread mainClientThread(&SocketClient::ClientThread, sockClient);
    // sleep(2);
    // sockClient->Write("Hey Server!");
    // sockServer->Write("Hey Client!");

    // sleep(2);

    // sockClient->ClientEnd();
    // std::cout << "Client end successful" << '\n';
    // sockServer->ServerEnd();

    // mainClientThread.join();
    // mainServerThread.join();

    // while (sockServer->IsServerRunning())
    // {

    // }
    // mainServerThread.join();

    while (true)
    {

    }
    //mainClientThread.join();
    return 1;

}