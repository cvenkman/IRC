#include "Server.hpp"

void Server::init() {
	this->createSocket();
	this->getSocketFd();
	this->Socket();
	this->Listen();
}

void Server::createServerInfo() {
	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int status = getaddrinfo("localhost", port.c_str(), &hints, &serverInfo);
	if (status != 0) {
		perror("getaddrinfo");
		throw ServerStandartFunctionsException("getaddrinfo (port: " + port + ")");
	}
	freeaddrinfo(serverInfo); // освободить связанный список, не знаю пока, нужно ли это

}

void Server::createSocket() {
	socketFd = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	if (socketFd == -1) {
		perror("socket");
		throw ServerStandartFunctionsException("socket");
	}
}

void Server::Socket() {
	/* залипание порта: нужно перед вызовом bind выставить на
		будущем слушающем сокете опцию SO_REUSEADDR */
	int opt = 1;
	if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1) {
		perror("setsockopt");
		throw ServerStandartFunctionsException("setsockopt");
	}
	if (bind(socketFd, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1) {
		perror("bind");
		throw ServerStandartFunctionsException("bind");
	}
}

void Server::Listen() {
	/*
	* нужно дождаться входящих подключений и затем их как-то обрабатывать.
	* Это двухшаговый процесс: сначала слушаем - listen(), затем принимаем accept()
	*/
	if (listen(socketFd, 0) == -1) {
		perror("listen");
		throw ServerStandartFunctionsException("listen (port: " + port + ")");
	}
}

int Server::Accept(int fd) {
	int newFd;
	struct sockaddr_in clientAddr; // для accept()
	socklen_t clientAddrLen = sizeof(clientAddr);
	newFd = accept(fd, (struct sockaddr *)&clientAddr, &clientAddrLen); // украла эту строку из экзамена
	if (newFd == -1) {
		perror("accept");
		throw ServerStandartFunctionsException("accept (port: " + port + ")");
	}
	/*
	* теперь у нас есть socketFd и newFd:
	* на socketFd - слушать запросы на соединение
	* на newFd - обслуживать соединение (новое подключение) конкретного клиента
	*/
	std::string msg("SERVER: new client with fd: " + std::to_string(newFd) + "\n");
	
	std::cout << msg;

	for (int i = 1; i < this->userFd.size(); i++) {
		int sentBytes = send(userFd[i].fd, msg.c_str(), msg.length(), 0);
		if (sentBytes == -1) {
			perror("send");
			throw ServerStandartFunctionsException("send");
		}
	}

	pollFd.fd = newFd;
	pollFd.events = POLLIN;
	pollFd.revents = 0;
	userFd.push_back(pollFd);

	User *newUser = new User();
	users.push_back(newUser);


	/* новый сокет newFd добавили в множество активных сокетов activeSet (установить в 1 newFd в activeSet) */
	// FD_SET(newFd, &activeSet);
	return newFd;
}

void Server::readDataFromClient(int fd) {
}

int Server::mainLoop() {
	pollFd.fd = socketFd;
	pollFd.events = POLLIN;
	pollFd.revents = 0;
	userFd.push_back(pollFd);
	// int numSet = 1;
	int newFd;

	while (1) {
		int ret = poll(userFd.data(), userFd.size(), -1);
		if (ret == -1) {
			perror("poll");
			throw ServerStandartFunctionsException("poll (port: " + port + ")");
		}

		if (ret > 0) {
			for (int i = 0; i < userFd.size(); i++) {

				if (userFd[i].revents & POLLIN) {
					userFd[i].revents = 0;

					if (userFd[i].fd == socketFd) {
						// создание нового соединения
						newFd = Accept(userFd[i].fd);
						// регистрация нового сокета


					}

					else {
						char buf[100];
						bzero(&buf, 100);
						int readBytes = recv(userFd[i].fd, &buf, 100, 0);
						if (readBytes == -1) {
							perror("recv");
							close(userFd[i].fd);
							userFd.erase(userFd.begin() + i);
						}
						else if (!strncmp(buf, "stop", 4)) {
							std::cout << "SERVER: stop client with fd: " << userFd[i].fd << "\n";
							close(userFd[i].fd);
							userFd.erase(userFd.begin() + i);
						}
						// else if (!strncmp(buf, "PASS 123", 8)) {
						// 	send(userFd[i].fd, hiMsg.c_str(), hiMsg.length(), 0);
						// }


						else {
							std::string msg = "CLIENTS FD: " + std::to_string(userFd[i].fd) + " : ";
							msg += buf;
							for (int i = 1; i < this->userFd.size(); i++) {
								int sentBytes = send(userFd[i].fd, msg.c_str(), msg.length(), 0);
								if (sentBytes == -1) {
									perror("send");
									throw ServerStandartFunctionsException("send");
								}
							}
						}
					}
				}
			}
		}
	}
	close(socketFd);
	return 0;
}
