#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream> // Для работы с файлами

#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048

std::map<int, std::string> client_names; // Хранит имена пользователей
std::vector<int> client_sockets; // Хранит сокеты всех клиентов
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Функция для отправки сообщения всем, кроме отправителя
void broadcast_message(const char *message, int sender_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int client_socket : client_sockets) {
        if (client_socket != sender_socket) {
            send(client_socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Функция для отправки приватного сообщения
void send_private_message(const char *message, const std::string &recipient, int sender_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (auto &client : client_names) {
        if (client.second == recipient) {
            send(client.first, message, strlen(message), 0);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Функция для обработки клиента
void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    char name[BUFFER_SIZE];

    // Получаем имя клиента
    recv(client_socket, name, BUFFER_SIZE, 0);
    client_names[client_socket] = std::string(name);
    std::string welcome_message = std::string(name) + " has joined the chat!\n";
    broadcast_message(welcome_message.c_str(), client_socket);

    while (true) {
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            // Клиент отключился
            pthread_mutex_lock(&clients_mutex);
            client_sockets.erase(std::remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
            client_names.erase(client_socket);
            pthread_mutex_unlock(&clients_mutex);
            close(client_socket);
            std::string leave_message = std::string(name) + " has left the chat.\n";
            broadcast_message(leave_message.c_str(), client_socket);
            break;
        }

        buffer[bytes_received] = '\0';
        std::string message(buffer);

        if (message[0] == '@') {
            // Приватное сообщение
            size_t space = message.find(' ');
            if (space != std::string::npos) {
                std::string recipient = message.substr(1, space - 1); // Имя получателя
                std::string private_message = "[PM from " + std::string(name) + "]: " + message.substr(space + 1);
                send_private_message(private_message.c_str(), recipient, client_socket);
            }
        } else {
            // Общее сообщение в чат
            std::string chat_message = std::string(name) + ": " + message;
            broadcast_message(chat_message.c_str(), client_socket);
        }
    }

    return nullptr;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t tid;

    // Создаем сокет
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    // Настраиваем адрес сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = 0; // Автоматический выбор свободного порта
    server_addr.sin_addr.s_addr = inet_addr("172.26.141.157");

    // Привязываем сокет к адресу
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error binding socket\n";
        return 1;
    }

    // Получаем выбранный порт
    getsockname(server_socket, (struct sockaddr *)&server_addr, &client_len);
    int server_port = ntohs(server_addr.sin_port);
    std::cout << "Server started on port " << server_port << std::endl;

    // Записываем порт в файл
    std::ofstream port_file("port.txt");
    if (port_file.is_open()) {
        port_file << server_port;
        port_file.close();
    } else {
        std::cerr << "Error writing port to file\n";
        return 1;
    }

    // Слушаем входящие соединения
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        std::cerr << "Error listening on socket\n";
        return 1;
    }

    while (true) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            std::cerr << "Error accepting connection\n";
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        client_sockets.push_back(client_socket);
        pthread_mutex_unlock(&clients_mutex);

        // Создаем поток для обработки клиента
        pthread_create(&tid, nullptr, handle_client, &client_socket);
        pthread_detach(tid);
    }

    close(server_socket);
    return 0;
}