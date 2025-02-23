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

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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

    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(server_addr);

    for (int i = 1; i <= 50; ++i) {
        snprintf(buffer, BUFFER_SIZE, "%d", i);
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, addr_len);
        std::cout << "Отправлено: " << buffer << std::endl;

        int recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, nullptr, nullptr);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            std::cout << "Ответ сервера: " << buffer << std::endl;
        }

        sleep(i); // Задержка в i секунд перед отправкой следующего числа
    }

    close(sockfd);
    return 0;
}
