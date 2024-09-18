#include "../include/Server.h"


Server::Server()
    : wsa{ 0 }, info{ 0 }, serverSocket(INVALID_SOCKET),
    port(55555), ipAddress("127.0.0.1")
{
    int wsaStartupResult = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (wsaStartupResult != 0) {
        std::cerr << std::endl << "[-] WSAStartup failed: " << wsaStartupResult << std::endl;
        exit(1);
    }
}

Server::~Server()
{
    WSACleanup();
}

void Server::start()
{
    init();

    std::thread connectingThread(&Server::listening, this);
    connectingThread.detach();

    while (true) {

    }
}

void Server::init()
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << std::endl << "[-] Error creating socket: " << WSAGetLastError() << std::endl;
        return;
    }

    info.sin_family = AF_INET;
    info.sin_port = htons(port);
    info.sin_addr.s_addr = inet_addr(ipAddress.c_str());

    if (bind(serverSocket, (sockaddr*)&info, sizeof(info)) == SOCKET_ERROR) {
        std::cerr << std::endl << "[-] Error bind socket: " << WSAGetLastError() << std::endl;
        return;
    }

    std::cout << "[+] The server is running." << std::endl;
}

void Server::listening()
{
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << std::endl << "[-] Error listen: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        return;
    }

    while (true) {
        sockaddr_in clientInfo;
        int clientInfoLength = sizeof(clientInfo);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientInfo, &clientInfoLength);

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "[-] Accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        recv(clientSocket, buffer, sizeof(buffer), 0);
        clients[clientSocket] = buffer;
        std::cout << "[+] Client connected from <" << inet_ntoa(clientInfo.sin_addr) << ":" << ntohs(clientInfo.sin_port) << "> under the name <" << buffer << ">." << std::endl;
        std::thread(&Server::handleClient, this, clientSocket).detach();

        broadcastClientNames(clientSocket);
    }
}

void Server::handleClient(SOCKET clientSocket)
{
    memset(buffer, 0, sizeof(buffer));
    while (true) {
        int recvLength = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (recvLength <= 0) {
            clientDisconnected(clientSocket);
            break;
        }

        message.clear();
        message = buffer;
        if (message.empty())
            continue;
        std::string whom;
        if (message.at(0) == '{') {
            int i = 1;
            while (message.at(i) != '}') {
                whom += message.at(i);
                ++i;
            }

            std::string whomR = "{" + whom + "}";
            size_t pos = message.find(whomR);
            if (pos != std::string::npos) {
                message.erase(pos, whomR.length());
            }
        }

        else {
            message = "Select the recipient.";
            send(clientSocket, message.c_str(), message.size(), 0);
            memset(buffer, 0, sizeof(buffer));
            continue;
        }

        message = "{" + clients[clientSocket] + "}" + "> " + message;

        if (whom == "007") {
            std::cout << "Client {" + clients[clientSocket] + ":" + std::to_string(clientSocket) + "} sent to recipient {General} " + message << std::endl;
            for (auto client : clients) {
                if (client.first == clientSocket)
                    continue;
                send(client.first, message.c_str(), message.size(), 0);
            }
        }
        else {
            std::cout << "Client {" + clients[clientSocket] + "} sent to recipient {" + clients[std::stoi(whom)] + "} " + message << std::endl;
            send(std::stoi(whom), message.c_str(), message.size(), 0);
        }

        memset(buffer, 0, sizeof(buffer));
    }
}

void Server::clientDisconnected(SOCKET clientSocket)
{
    //std::lock_guard<std::mutex> lock(mx);

    std::cout << "[+] Client under the name <" << clients[clientSocket] << "> disconnected, socket closed." << std::endl;
    clients.erase(clientSocket);
    closesocket(clientSocket);

    broadcastClientNames(clientSocket);
}

void Server::broadcastClientNames(SOCKET clientSocket)
{
    std::lock_guard<std::mutex> lock(mx);

    std::string clientsNames = "CLIENT_LIST:General,";
    for (const auto& client : clients) {
        clientsNames += "{" + client.second + " : " + std::to_string(client.first) + "},";
    }

    for (const auto& client : clients) {
        send(client.first, clientsNames.c_str(), clientsNames.size(), 0);
    }
}