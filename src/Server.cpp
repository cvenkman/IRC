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
		throw std::runtime_error("SERVER ERROR: getaddrinfo (port: " + port + ")");
	}
	freeaddrinfo(serverInfo); // освободить связанный список, не знаю пока, нужно ли это
}

void Server::createSocket() {
	socketFd = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	if (socketFd == -1) {
		perror("socket");
		throw std::runtime_error("SERVER ERROR: socket");
	}
}

void Server::Socket() {
	/* залипание порта: нужно перед вызовом bind выставить на
		будущем слушающем сокете опцию SO_REUSEADDR */
	int opt = 1;
	if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1) {
		perror("setsockopt");
		throw std::runtime_error("SERVER ERROR: setsockopt");
	}
	if (bind(socketFd, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1) {
		perror("bind");
		throw std::runtime_error("SERVER ERROR: bind");
	}
}

void Server::Listen() {
	/*
	* нужно дождаться входящих подключений и затем их как-то обрабатывать.
	* Это двухшаговый процесс: сначала слушаем - listen(), затем принимаем accept()
	*/
	if (listen(socketFd, 0) == -1) {
		perror("listen");
		throw std::runtime_error("SERVER ERROR: listen (port: " + port + ")");
	}
	fcntl(socketFd, F_SETFL, O_NONBLOCK); // FIXME
}

int Server::Accept() {
	int newFd;
	struct sockaddr_in clientAddr; // для accept()
	socklen_t clientAddrLen = sizeof(clientAddr);
	newFd = accept(socketFd, (struct sockaddr *)&clientAddr, &clientAddrLen); // украла эту строку из экзамена
	if (newFd == -1) {
		perror("accept");
		throw std::runtime_error("SERVER ERROR: accept (port: " + port + ")");
	}
	/*
	* теперь у нас есть socketFd и newFd:
	* на socketFd - слушать запросы на соединение
	* на newFd - обслуживать соединение (новое подключение) конкретного клиента
	*/
	std::string msg("SERVER: new client with fd: " + std::to_string(newFd) + "\n");
	
	std::cout << msg;

	sendToAllUsers(msg, newFd);
	Send(newFd, "SERVER: enter PASS, NICK, USER\n", 31, 0);

	pollFd.fd = newFd;
	pollFd.events = POLLIN;
	pollFd.revents = 0;
	userFd.push_back(pollFd);

	User *newUser = new User(newFd);
	users.insert(std::pair<int, User *>(newFd, newUser));

	
	return newFd;
}

void Server::readDataFromClient(int fd) {
}

// bool	work = true;

// void	sigHandler(int signum)
// {
// 	(void)signum;
// 	work = false;
// }

int Server::mainLoop() {
	pollFd.fd = socketFd;
	pollFd.events = POLLIN;
	pollFd.revents = 0;
	userFd.push_back(pollFd);
	// int numSet = 1;
	int newFd;

	// signal(SIGINT, sigHandler);
	while (true) {
		int ret = poll(userFd.data(), userFd.size(), -1);
		if (ret == -1) {
			perror("poll");
			throw std::runtime_error("SERVER ERROR: poll (port: " + port + ")");
		}

		if (ret > 0) {
			for (int i = 0; i < userFd.size(); i++) {

				if (userFd[i].revents & POLLIN) {
					userFd[i].revents = 0;

					if (userFd[i].fd == socketFd) {
						// создание нового соединения
						newFd = Accept();
						// регистрация нового сокета
					}

					else {



	/// ПЕРЕПИСАТЬ buf[100] В ЦИКЛ С buf[99]




						char buf[100];
						bzero(&buf, 100);
						int readBytes = recv(userFd[i].fd, &buf, 100, 0);
						if (readBytes == -1) {
							close(userFd[i].fd);
							userFd.erase(userFd.begin() + i);
						}
						buf[readBytes - 1] = '\0';
						std::cout << '|' << buf << "|" << "\n";
						if (!strcmp(buf, "stop")) {
							std::cout << "SERVER: stop client with fd: " << userFd[i].fd << "\n";
							close(userFd[i].fd);
							userFd.erase(userFd.begin() + i);
						}
						// else if (!strncmp(buf, "PASS 123", 8)) {
						// 	send(userFd[i].fd, hiMsg.c_str(), hiMsg.length(), 0);
						// }


						else {
							std::string passwordStr("PASS " + password);
							// std::cout << passwordStr << " " << passwordStr.length() << "\n";
							// std::cout << buf << "\n";
							if (!strcmp(buf, password.c_str())) {
								send(userFd[i].fd, hiMsg.c_str(), hiMsg.length(), 0);
								users.find(userFd[i].fd)->second->setUserConnection(true);
							}
							else if (users.find(userFd[i].fd)->second->getUserConnection()) {
								std::string msg = "CLIENTS FD: " + std::to_string(userFd[i].fd) + " : ";
								msg += buf;
								sendToAllUsers(msg, userFd[i].fd);
							}
							else {
								Send(userFd[i].fd, "SERVER: Invalid password\n", 25, 0);
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

void Server::sendToAllUsers(std::string &msg, int currentUserFd) {
	for (int i = 1; i < this->userFd.size(); i++) {
		if (userFd[i].fd != currentUserFd &&
				users.find(userFd[i].fd)->second->getUserConnection()) {
			Send(userFd[i].fd, msg.c_str(), msg.length(), 0);
		}
	}
}

int Server::Send(int fd, const void *msg, size_t msgSize, int flags) {
	int sentBytes = send(fd, msg, msgSize, 0);
	if (sentBytes == -1) {
		throw std::runtime_error("SERVER ERROR: send");
	}
	return sentBytes;
}