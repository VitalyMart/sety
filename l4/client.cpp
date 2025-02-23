#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Использование: " << argv[0] << " <IP> <порт> <число>" << std::endl;
        return 1;
    }

    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int i = atoi(argv[3]);

    if (i < 1 || i > 10) {
        std::cerr << "Число должно быть от 1 до 10" << std::endl;
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Ошибка создания сокета" << std::endl;
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        std::cerr << "Ошибка преобразования IP-адреса" << std::endl;
        return 1;
    }

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Ошибка подключения к серверу" << std::endl;
        return 1;
    }

    char buffer[BUFFER_SIZE];
    for (int j = 0; j < 5; ++j) {
        snprintf(buffer, BUFFER_SIZE, "%d", i);
        send(sockfd, buffer, strlen(buffer), 0);
        std::cout << "Отправлено: " << buffer << std::endl;
        sleep(i);
    }

    close(sockfd);
    return 0;
}
