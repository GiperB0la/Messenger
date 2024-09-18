#pragma once
#include <iostream>
#include <winSock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <mutex>
#include <string>
#include <unordered_map>
#include <condition_variable>

#include <QObject>
#include <QThread>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable: 4996)


class Messenger;
class Client : public QThread
{
	friend Messenger;
	Q_OBJECT
public:
	Client(QObject* parent = nullptr);
	~Client();

protected:
	void run() override;

private:
	void init();
	void receiveMessage();

public:
	void sendMessage(const std::string& message);

private:
	WSADATA wsa;
	sockaddr_in info;
	SOCKET clientSocket;

private:
	int port;
	std::mutex mx;
	std::string ipAddress;
	std::condition_variable cv;

private:
	bool runFlag;
	std::string name;
	char buffer[512];
	std::string message;

private:
	std::string whom;
	std::unordered_map<std::string, std::string> otherClients;

signals:
	void updateChat(QString message);
};