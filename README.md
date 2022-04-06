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


### htons()
Допустим, нужно представить двухбайтное шестнадцатиричное число (b34f). Его байты (b3 4f) должны храниться в памяти последовательно.

Так вот эти байты на компьютере могут храниться в разном порядке:
* Big-Endian (или Network Byte Order)
b3 и следом 4f
* Little-Endian
сначала 4f, затем b3

Сеть использует Big-Endian, так что мы должн быть уверены, что, когда заполняем структуры для отправки, 
числа в этой структуре построены в Big-Endian.

Поэтому нам нужно прогонять данные для структуры через функции вроде **htons** (**H**ost **to** **N**etwork **S**hort)
