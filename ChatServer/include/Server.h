#pragma once
#include <iostream>
#include <winSock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <string>
#include <chrono>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable: 4996)


class Server
{
public:
	Server();
	~Server();

public:
	void start();

private:
	void init();
	void listening();
	void handleClient(SOCKET clientSocket);
	void clientDisconnected(SOCKET clientSocket);
	void broadcastClientNames(SOCKET clientSocket);

private:
	WSADATA wsa;
	sockaddr_in info;
	SOCKET serverSocket;

private:
	int port;
	std::mutex mx;
	std::string ipAddress;
	char buffer[512];
	std::string message;

private:
	std::string clientsNames;
	std::unordered_map<SOCKET, std::string> clients;
};