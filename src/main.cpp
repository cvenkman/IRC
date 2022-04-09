#include "Server.hpp"

void printIPV4(struct addrinfo *servinfo) {
    std::cout << "IP addresses for localhost: " << "\n";
    struct addrinfo *tmp_servinfo;
    for (tmp_servinfo = servinfo; tmp_servinfo != NULL; tmp_servinfo = tmp_servinfo->ai_next) {
        void *addr;
        if (tmp_servinfo->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)tmp_servinfo->ai_addr;
            addr = &(ipv4->sin_addr);
            // перевести IP в строку и распечатать:
            char ipstr[INET_ADDRSTRLEN];
            inet_ntop(tmp_servinfo->ai_family, addr, ipstr, sizeof(ipstr));
            std::cout << "IPv4: " << ipstr << "\n";
        }
    }
}


void checkPort(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "need an ip and a port\n";
        exit(EXIT_FAILURE);
    }
    for (int i = 0; argv[1][i] != '\0'; i++) {
        if (isdigit(i)) {
            std::cout << "not valid port\n";
            exit(EXIT_FAILURE);
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
    checkPort(argc, argv);

    std::cout << "start\n\n";

    Server *server = new Server(argv[1]);

    server->init();
    server->mainLoop();


    return 0;
}