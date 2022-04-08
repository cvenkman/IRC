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

#include <unistd.h>// close

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

    fd_set activeSet, readSet; // множества дескрипторов для select
    FD_ZERO(&activeSet);
    /*
     * в activeSet устанавливаем 1 по тому номеру, который соотвествует socketFd
     * т.е. сейчас в activeSet один сокет socketFd, который принимает запросы на соединение
    */
    FD_SET(socketFd, &activeSet); // FIXME

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

    struct sockaddr_in clientAddr; // для accept()
    // socklen_t clientAddrLen = sizeof(clientAddr);
    int newFd;

    while (1) {
        /* FD_SETSIZE - максимальное кол-во каналов в системе,
         * т.е. рассматриваем столько элементов, сколько множество содержит
        */

        /********************************
            про select()
        int select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
        параметры по порядку:
        * сколько элементов просматривать в множестве
        * множество которое нужно проверять на возможное чтение
        * множество которое нужно проверять на возможную запись
        * множество которое нужно проверять на наличие исключительных ситуаций
        * таймаут в течении которого надо с этим работать (если NULL, то висим на этой функции
            пока один из каналов в множестве activeSet не проявит активность)

        * блокируемся на select, а не на accept
        ********************************/

        readSet = activeSet; // делаем копию т.к. select() изменяет множество, а нам дальеш нужно знать, кого читать
        if (select(FD_SETSIZE, &readSet, NULL, NULL, NULL) < 0) { //FIXME
            perror("select");
            return 1;
        }
        // на этой строчке у нас либо возникла ошибка в select, либо появились данные в одном из каналов readSet

        // нужно проверить в каком сокете (в каком элементе readSet) появились данные (т.е. 1)
        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &readSet)) {
                if (i == socketFd) {
                    // подтверждение соединения
                    // пришел запрос на новое соединение (клиент выполнил connect => мы должны сделать accept)
                    socklen_t clientAddrLen = sizeof(clientAddr);
                    newFd = accept(socketFd, (struct sockaddr *)&clientAddr, &clientAddrLen); // украла эту строку из экзамена
                    if (newFd == -1) {
                        perror("accept");
                        return 1;
                    }
                    std::cout << "Server: new connect; client's fd: " << newFd << "\n";
                    /* новый сокет newFd добавили в множество активных сокетов activeSet (установить в 1 newFd в activeSet) */
                    FD_SET(newFd, &activeSet);
                }
                else {
                    // пришли данные в уже существующем соединение и нам нужно работать с клиентом
                    
                    /* читаем сообщение от клиента */
                    char buf[100];
                    bzero(&buf, 100);
                    int readBytes = recv(i, &buf, 100, 0);
                    /*
                    * recv() может возвращать 0.
                    * Это означает, что удалённая сторона (клиент) закрыла для вас подключение
                    */
                    if (readBytes == -1) {
                        close(i);
                        FD_CLR(i, &activeSet); // удаляем из активного множества
                        perror("recv");
                    }
                    else if (!strncmp(buf, "stop", 4)) {
                        close(i);
                        FD_CLR(i, &activeSet);
                    }
                    else {
                        /* отправляем сообщение клиенту */
                        std::string msg = "hi i'm a message for a client with fd=" + std::to_string(i);
                        msg += ", your message: ";
                        msg += buf;
                        int sentBytes = send(i, msg.c_str(), msg.length(), 0);
                        /* sentBytes может быть меньше msg.length(), хз пока что с этим делать */
                        if (sentBytes == -1) {
                            perror("send");
                            return 1;
                        }
                    }
                    std::cout << "From client (fd = " << i << "): " << buf << "\n";
                }
            }
        }

        // /*
        // * теперь у нас есть socketFd и newFd:
        // * на socketFd - слушать запросы на соединение
        // * на newFd - обслуживать соединение (новое подключение) конкретного клиента
        // */
        // int newFd = accept(socketFd, (struct sockaddr *)&clientaddr, &len); // украла эту строку из экзамена
        // if (newFd == -1) {
        //     perror("accept");
        //     return 1;
        // }





        // close(newFd); // shutdown() (но после него все равно нужно вызвать close)
    }


//getpeername - это не пробовала
//gethostname - это не работает


    close(socketFd);
    return 0;
}