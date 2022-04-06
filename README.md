# IRC

Нужно создать свой IRC сервер.

Клиент создавать не нужно, есть готовые клиенты, чтобы потестить наш сервер. (KVIrc)

[IRC RFC](https://datatracker.ietf.org/doc/html/rfc1459#page-48)

[IRC RFC на русском](https://www.lissyara.su/doc/rfc/rfc1459/)

[книга про сокеты и сетевому программированию](https://beej.us/guide/bgnet/translations/bgnet_A4_rus.pdf)

[select и poll](https://it.wikireading.ru/24833)

[sockaddr и sockaddr_in](https://www.russianblogs.com/article/8587603498/)

[видео по ft_irc](https://www.youtube.com/watch?v=I9o-oTdsMgI)

### Клиент

* KVIrc - клиент с интерфейсом
* утилита nc - без синтаксиса

### Синтаксис

### Команды

* регистрация:
  - PASS:
  - NICK:
  - USER:
* сообщения:
  - PRIVMSG: 
  - NOTICE:
* присоединение и создание канала:
  - JOIN
* команда для операторы:
  - KICK: удалить клиента из канала


### htons() (**H**ost **to** **N**etwork **S**hort)
Допустим, нужно представить двухбайтное шестнадцатиричное число (b34f). Его байты (b3 4f) должны храниться в памяти последовательно.

Так вот эти байты на компьютере могут храниться в разном порядке:
* Big-Endian (или Network Byte Order)
b3 и следом 4f
* Little-Endian
сначала 4f, затем b3

Сеть использует Big-Endian, так что мы должн быть уверены, что, когда заполняем структуры для отправки, 
числа в этой структуре построены в Big-Endian.

Поэтому нам нужно прогонять данные для структуры через функции вроде **htons**.

### inet_pton() (presentation to network)
Преобразует строковый IP адрес (ddd.ddd.ddd.ddd) в структуру struct in_addr либо struct in6_addr в зависимости от
указанных AF_INET или AF_INET6. Замена inet_addr().

Еще есть inet_ntop() для обратного преобразоввния.

### struct sockaddr_in

Используется как замена структуре sockaddr (потому что помимо семейства протокола (AF_INET или AF_INET6) в sockaddr еще есть поле char sa_data[14], в котором должны храниться адрес назначения и номер порта для сокета, и легче заполнить структуру sockaddr_in, чем массив sa_data) - если нужно послать в функцию (например, в connect() или bind()) sockaddr_in приводится к sockaddr.

struct sockaddr_in {
 short int sin_family; // Семейство адресов, AF_INET
 unsigned short int sin_port; // Номер порта
 struct in_addr sin_addr; // Интернет адрес
 unsigned char sin_zero[8]; // Размер как у struct sockaddr
}; 

### struct in_addr
