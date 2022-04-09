#include "Server.hpp"

#include <sys/poll.h>

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
	std::cout << "Server: new connect; client's fd: " << newFd << "\n";
	/* новый сокет newFd добавили в множество активных сокетов activeSet (установить в 1 newFd в activeSet) */
	// FD_SET(newFd, &activeSet);
	return newFd;
}

void Server::readDataFromClient(int fd) {
}


int Server::mainLoop() {
	pollfd actSet[100];
	actSet[0].fd = socketFd;
	actSet[0].events = POLLIN;
	actSet[0].revents = 0;
	int numSet = 1;
	int newFd;

	while (1) {
		int ret = poll(actSet, numSet, -1);
		if (ret == -1) {
			perror("poll");
			throw ServerStandartFunctionsException("poll (port: " + port + ")");
		}

		if (ret > 0) {
			for (int i = 0; i < numSet; i++) {
				if (actSet[i].revents &POLLIN) {
					std::cout << "get POLLIN at fd=" << actSet[i].fd << "\n";
					actSet[i].revents = 0;
					if (i == 0) {
						// создание нового соединения
						newFd = Accept(actSet[i].fd);
						// регистрация нового сокета
						if (numSet < 100) {
							actSet[numSet].fd = newFd;
							actSet[numSet].events = POLLIN;
							actSet[numSet].revents = 0;
							numSet++;
						}
						else {
							std::cout << "no more sockets for client\n";
							close(newFd);
						}
					}

					else {
						char buf[100];
						bzero(&buf, 100);
						int readBytes = recv(actSet[i].fd, &buf, 100, 0);
						if (readBytes == -1) {
							perror("recv");
							close(actSet[i].fd);
							// if (i < numSet - 1) {
							// 	actSet[i] = actSet[numSet - 1];
							// 	numSet--;
							// 	i--;
							// }
						}
						else if (!strncmp(buf, "stop", 4)) {
							close(actSet[i].fd);
							// if (i < numSet - 1) {
							// 	actSet[i] = actSet[numSet - 1];
							// 	numSet--;
							// 	i--;
							// }
						}
						else {
							std::string msg = "hi i'm a message for a client with fd=" + std::to_string(i);
							msg += ", your message: ";
							msg += buf;
							int sentBytes = send(actSet[i].fd, msg.c_str(), msg.length(), 0);
							if (sentBytes == -1) {
								perror("send");
								return 1;
							}
						}
						std::cout << "From client (fd = " << i << "): " << buf << "\n";
					}
				}
			}
		}
	}
	close(socketFd);
	return 0;
}


// int Server::mainLoop() {
// 	fd_set readSet; // множества дескрипторов для select
// 	FD_ZERO(&activeSet);
// 	/*
// 	* в activeSet устанавливаем 1 по тому номеру, который соотвествует socketFd
// 	* т.е. сейчас в activeSet один сокет socketFd, который принимает запросы на соединение
// 	*/
// 	FD_SET(socketFd, &activeSet); // FIXME

// 	while (1) {
// 		/*
// 		 * FD_SETSIZE - максимальное кол-во каналов в системе,
// 		 * т.е. рассматриваем столько элементов, сколько множество содержит
// 		*/

// 		readSet = activeSet; // делаем копию т.к. select() изменяет множество, а нам дальеш нужно знать, кого читать
// 		if (select(FD_SETSIZE, &readSet, NULL, NULL, NULL) < 0) { //FIXME
// 			perror("select");
// 			return 1;
// 		}
// 		// на этой строчке у нас либо возникла ошибка в select, либо появились данные в одном из каналов readSet

// 		// нужно проверить в каком сокете (в каком элементе readSet) появились данные (т.е. 1)
// 		for (int i = 0; i < FD_SETSIZE; i++) {
// 			if (FD_ISSET(i, &readSet)) {
// 				if (i == socketFd) {
// 					// подтверждение соединения
// 					// пришел запрос на новое соединение (клиент выполнил connect => мы должны сделать accept)
// 					Accept();
// 				}
// 				else {
// 					// пришли данные в уже существующем соединение и нам нужно работать с клиентом
					
// 					/* читаем сообщение от клиента */
// 					char buf[100];
// 					bzero(&buf, 100);
// 					int readBytes = recv(i, &buf, 100, 0);
// 					if (readBytes == -1) {
// 						close(i);
// 						FD_CLR(i, &activeSet); // удаляем из активного множества
// 						perror("recv");
// 					}
					
// 					else if (!strncmp(buf, "stop", 4)) {
// 						close(i);
// 						FD_CLR(i, &activeSet);
// 					}
// 					else {
// 						/* отправляем сообщение клиенту */
// 						std::string msg = "hi i'm a message for a client with fd=" + std::to_string(i);
// 						msg += ", your message: ";
// 						msg += buf;
// 						int sentBytes = send(i, msg.c_str(), msg.length(), 0);
// 						/* sentBytes может быть меньше msg.length(), хз пока что с этим делать */
// 						if (sentBytes == -1) {
// 							perror("send");
// 							return 1;
// 						}
// 					}
// 					std::cout << "From client (fd = " << i << "): " << buf << "\n";
// 				}
// 			}
// 		}
// 		// close(newFd); // shutdown() (но после него все равно нужно вызвать close)
// 	}
// 	close(socketFd);
// 	return 0;
// }
