#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <stdlib.h>
#include <iostream>


void printIPV4(char *ip, struct addrinfo *servinfo) {
    std::cout << "IP addresses for " << ip << "\n";
    struct addrinfo *tmp_servinfo;
    for (tmp_servinfo = servinfo; tmp_servinfo != NULL; tmp_servinfo = tmp_servinfo->ai_next) {
        void *addr;
        if (tmp_servinfo->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)tmp_servinfo->ai_addr;
            // std::cout << ipv4->sin_port << "\n";
            // std::cout <<  tmp_servinfo->ai_addr << "\n";
            addr = &(ipv4->sin_addr);

            // перевести IP в строку и распечатать:
            char ipstr[INET_ADDRSTRLEN];
            inet_ntop(tmp_servinfo->ai_family, addr, ipstr, sizeof(ipstr));
            std::cout << "IPv4: " << ipstr << "\n";
        }
    }
}

/*
 * если собираемся слушать входящие подключения (по одному за раз до select'а),
 * нужно выполнить следующую последовательность вызовов:
 * 
 * сначала заполнить адресные структуры:
 *      getaddrinfo();
 * создать сокет, связать и слушать:
 *      socket();
 *      bind(); (перед ним setsockopt(), чтобы убрать залипание)
 *      listen();
 * принять входящие подключения:
 *      accept();
 * связываемся по дескриптору сокета new_fd
*/

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cout << "need an ip and a port\n";
        return 1;
    }

    struct addrinfo *servinfo;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(argv[1], argv[2], &hints, &servinfo);
        if (status != 0) {
            perror("getaddrinfo");
            return 1;
        }

    printIPV4(argv[1], servinfo);

    freeaddrinfo(servinfo); // освободить связанный список, не знаю пока, нужно ли это
    
    int socketFd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (socketFd == -1) {
        perror("socket");
        return 1;
    }

    /* залипание порта: нужно перед вызовом bind выставить на
        будущем слушающем сокете опцию SO_REUSEADDR */
    int opt = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }

    if (bind(socketFd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        perror("bind");
        return 1;
    }

    /* для клиента */
    // if (connect(socketFd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
    //     perror("connect");
    //     return 1;
    // }

    /*
     * нужно дождаться входящих подключений и затем их как-то обрабатывать.
     * Это двухшаговый процесс: сначала слушаем - listen(), затем принимаем accept()
    */
    if (listen(socketFd, 10) == -1) {
        perror("listen");
        return 1;
    }

    /********************************
            про accept()
    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); 

     * клиент пытается подключиться вызовом connect() к вашей машине
       на порт, который вы слушаете вызовом listen()
     * Это соединение будет поставлено в очередь ждать accept()'а
     * Вызываем accept() и говорим ему принять ожидающие подключения
     * accept() вернёт новый fd сокета для использования с одним подключением
     * Исходный до сих пор слушает новые подключения,
       а вновь созданный полностью готов к send() и recv()
    ********************************/
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);

    int newFd = accept(socketFd, (struct sockaddr *)&clientaddr, &len); // украла эту строку из экзамена
    if (newFd == -1) {
        perror("accept");
        return 1;
    }

    return 0;
}