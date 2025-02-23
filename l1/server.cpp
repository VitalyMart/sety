#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024
#define SERVER_IP "172.26.141.157"

int main() {
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        std::cerr << "Ошибка создания сокета" << std::endl;
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = 0;  // Пусть ОС выберет свободный порт

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Ошибка привязки сокета" << std::endl;
        return 1;
    }

    getsockname(server_socket, (struct sockaddr*)&server_addr, &client_len);
    std::cout << "Сервер запущен на IP: " << SERVER_IP
              << " порту: " << ntohs(server_addr.sin_port) << std::endl;

    while (true) {
        int recv_len = recvfrom(server_socket, buffer, BUFFER_SIZE, 0,
                                (struct sockaddr*)&client_addr, &client_len);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            int number = atoi(buffer);
            int doubled = number * 2;

            std::cout << "Получено от " << inet_ntoa(client_addr.sin_addr)
                      << ":" << ntohs(client_addr.sin_port) << " -> " << number << std::endl;

            // Имитация задержки обработки (равной числу)
            sleep(number);

            std::string response = std::to_string(doubled);
            sendto(server_socket, response.c_str(), response.length(), 0,
                   (struct sockaddr*)&client_addr, client_len);
        }
    }

    close(server_socket);
    return 0;
}
