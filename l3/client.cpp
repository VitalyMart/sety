#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Использование: " << argv[0] << " <IP> <порт>" << std::endl;
        return 1;
    }

    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

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
    for (int i = 1; i <= 5; ++i) {  // Изменено на 5 циклов для теста
        snprintf(buffer, BUFFER_SIZE, "%d", i);
        send(sockfd, buffer, strlen(buffer), 0);
        std::cout << "Отправлено: " << buffer << std::endl;

        int recv_len = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            std::cout << "Ответ сервера: " << buffer << std::endl;
        }

        sleep(i);
    }

    close(sockfd);
    return 0;
}
