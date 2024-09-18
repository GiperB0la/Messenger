#include "../include/Client.h"


Client::Client(QObject* parent)
    : wsa{ 0 }, info{ 0 }, clientSocket(INVALID_SOCKET),
    port(55555), ipAddress("127.0.0.1"), runFlag(false), name("Name")
{
    int wsaStartupResult = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (wsaStartupResult != 0) {
        std::cerr << std::endl << "[-] WSAStartup failed: " << wsaStartupResult << std::endl;
        exit(1);
    }
}

Client::~Client()
{
    WSACleanup();
}

void Client::run()
{
    init();

    while (true) {
        if (runFlag) {
            receiveMessage();
        }
        else {
            init();
        }
    }
}

void Client::init()
{
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << std::endl << "[-] Error creating socket: " << WSAGetLastError() << std::endl;
        return;
    }

    info.sin_family = AF_INET;
    info.sin_port = htons(port);

    if (inet_pton(AF_INET, ipAddress.c_str(), &info.sin_addr) <= 0) {
        std::cerr << "[-] Invalid address" << std::endl;
        return;
    }

    if (::connect(clientSocket, (sockaddr*)&info, sizeof(info)) < 0) {
        std::cerr << "[-] Connection failed" << std::endl;
        closesocket(clientSocket);
        return;
    }

    runFlag = true;

    send(clientSocket, name.c_str(), name.size(), 0);
}

void Client::sendMessage(const std::string& message)
{
    if (runFlag) {
        if (!message.empty()) {
            send(clientSocket, message.c_str(), message.size(), 0);
        }
    }
}

void Client::receiveMessage()
{
    while (runFlag) {
        memset(buffer, 0, sizeof(buffer));
        int recvLength = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (recvLength == 0) {
            std::cerr << "[-] Server closed connection." << std::endl;
            runFlag = false;
        }
        else if (recvLength < 0) {
            std::cerr << "[-] Receive failed with error: " << WSAGetLastError() << std::endl;
            runFlag = false;
        }

        std::string receivedMessage(buffer);

        if (receivedMessage.rfind("CLIENT_LIST:", 0) == 0) {
            otherClients.clear();
            std::string clientList = receivedMessage.substr(12);
            clientList.erase(std::remove_if(clientList.begin(), clientList.end(),
                [](char c) { return c == '{' || c == '}' || c == ' ' || c == ':'; }),
                clientList.end());

            std::string nameClient;
            std::string socketClient;
            for (auto c : clientList) { 
                if (c != ',') {
                    if (!std::isdigit(c))
                        nameClient += c;
                    else
                        socketClient += c;
                }
                else {
                    if (nameClient == name) {
                        nameClient.clear();
                        socketClient.clear();
                        continue;
                    }

                    if (nameClient == "General") {
                        socketClient = "007";
                    }

                    otherClients[nameClient] = socketClient;
                    nameClient.clear();
                    socketClient.clear();
                }
            }
        }
        else {
            emit updateChat(QString::fromStdString(receivedMessage));
        }
    }
}