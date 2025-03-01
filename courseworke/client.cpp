#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Создаем сокет
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Настроим адрес сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Подключаемся к серверу
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    std::cout << "Connected to server. Type your messages." << std::endl;

    // Имя клиента для демонстрации
    std::string client_name;
    std::cout << "Enter your name: ";
    std::getline(std::cin, client_name);

    // Поток для приема сообщений
    while (true) {
        std::cout << client_name << ": ";
        std::cin.getline(buffer, BUFFER_SIZE);

        // Отправляем сообщение на сервер
        std::string message = client_name + ": " + buffer;
        send(sockfd, message.c_str(), message.length(), 0);

        // Получаем ответ от сервера
        ssize_t bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::cout << "Message from server: " << buffer << std::endl;
        }
    }

    close(sockfd);
    return 0;
}
