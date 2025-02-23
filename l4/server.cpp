#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define SERVER_IP "172.26.141.157"

int main() {
    int server_socket, client_socket, max_sd, activity;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    fd_set read_fds;
    int client_sockets[MAX_CLIENTS] = {0};

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Ошибка создания сокета" << std::endl;
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = 0;  

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Ошибка привязки сокета" << std::endl;
        return 1;
    }

    getsockname(server_socket, (struct sockaddr*)&server_addr, &client_len);
    std::cout << "Сервер запущен на IP: " << SERVER_IP
              << " порту: " << ntohs(server_addr.sin_port) << std::endl;

    listen(server_socket, MAX_CLIENTS);

    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        max_sd = server_socket;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] > 0) FD_SET(client_sockets[i], &read_fds);
            if (client_sockets[i] > max_sd) max_sd = client_sockets[i];
        }

        activity = select(max_sd + 1, &read_fds, nullptr, nullptr, nullptr);

        if (FD_ISSET(server_socket, &read_fds)) {
            client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
            if (client_socket < 0) {
                std::cerr << "Ошибка принятия подключения" << std::endl;
                continue;
            }

            std::cout << "Клиент подключен: " << inet_ntoa(client_addr.sin_addr) 
                      << ":" << ntohs(client_addr.sin_port) << std::endl;

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_socket;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (FD_ISSET(sd, &read_fds)) {
                char buffer[BUFFER_SIZE];
                int valread = recv(sd, buffer, BUFFER_SIZE, 0);
                if (valread <= 0) {
                    close(sd);
                    client_sockets[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    std::cout << "Получено: " << buffer << std::endl;
                }
            }
        }
    }

    close(server_socket);
    return 0;
}
