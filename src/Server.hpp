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
class Server {

private:
	struct addrinfo *serverInfo;
	std::string port;
	int socketFd;
	fd_set activeSet;

	void createServerInfo();
	void createSocket();
	void Socket();
	void Listen();
	int Accept(int fd);
	void readDataFromClient(int fd);

public:
	Server(std::string port) : port(port) {
		createServerInfo();
	}

	~Server() {}

	class ServerStandartFunctionsException : public std::exception {
		private:
			std::string error;
		public:
			ServerStandartFunctionsException(std::string const& error) : error(error) {};
			const char* what() const throw() {return error.c_str(); };
			~ServerStandartFunctionsException() throw() {};
	};

	struct addrinfo *getServerInfo() {return serverInfo; }
	int const &getSocketFd() const {return this->socketFd; }
	std::string const &getPort() const {return this->port; }

	void init();

	int mainLoop();

};