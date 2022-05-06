#pragma once

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <stdlib.h>
#include <iostream>
#include <string>

#include <unistd.h> // close

#include <exception>
#include <sys/poll.h>
#include <vector>
#include <map>
#include "User.hpp"
#include <fcntl.h>

class Server {

private:
	struct addrinfo *serverInfo;
	std::string port;
	std::string password;
	int socketFd;
	fd_set activeSet;
	pollfd pollFd;
	std::vector<struct pollfd> userFd;
	std::map<int, User *> users;

	std::string hiMsg;

	void createServerInfo();
	void createSocket();
	void Socket();
	void Listen();
	int Accept();
	void readDataFromClient(int fd);
	int Send(int fd, const void *msg, size_t msgSize, int flags);

public:
	Server(std::string port, std::string password) : port(port), password(password), hiMsg("cat!@127.0.0.1\n") {
		createServerInfo();
	}

	~Server() {}

	// class ServerStandartFunctionsException : public std::exception {
	// 	private:
	// 		std::string error;
	// 	public:
	// 		ServerStandartFunctionsException(std::string const& error) : error(error) {};
	// 		const char* what() const throw() {return error.c_str(); };
	// 		~ServerStandartFunctionsException() throw() {};
	// };

	struct addrinfo *getServerInfo() {return serverInfo; }
	int const &getSocketFd() const {return this->socketFd; }
	std::string const &getPort() const {return this->port; }

	void init();

	int mainLoop();

	void sendToAllUsers(std::string &msg, int currentUserFd);
};