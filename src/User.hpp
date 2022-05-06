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

class User {
private:
	std::string nick;
	int fd;
	bool isConnected;
public:
	User(int fd) : fd(fd), isConnected(false) {};
	~User() {};

	std::string getNick() {return nick; }
	int getFd() {return fd; }
	bool getUserConnection() {return isConnected; };
	void setUserConnection(bool newConnection) {isConnected = newConnection; };
};