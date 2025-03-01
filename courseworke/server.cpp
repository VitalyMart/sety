#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <vector>
#include <algorithm>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Структура для хранения информации о клиенте
struct Client {
    int socket;
    std::string address;
};

// Массив клиентов
std::vector<Client> clients;

// Мьютекс для синхронизации доступа к массиву клиентов
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

// Функция для обработки клиентов
void* handle_client(void* arg) {
    Client client = *(Client*)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Ожидаем сообщения от клиента
    while ((bytes_read = recv(client.socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_read] = '\0';
        std::cout << "Received from " << client.address << ": " << buffer << std::endl;

        // Отправляем сообщение всем клиентам, кроме отправителя
        pthread_mutex_lock(&client_mutex);
        for (const Client& other_client : clients) {
            if (other_client.socket != client.socket) {
                std::string message = "From " + client.address + ": " + buffer;
                send(other_client.socket, message.c_str(), message.length(), 0);
            }
        }
        pthread_mutex_unlock(&client_mutex);
    }

    // Закрытие сокета клиента и удаление из списка
    close(client.socket);
    pthread_mutex_lock(&client_mutex);
    clients.erase(std::remove_if(clients.begin(), clients.end(), 
                                 [&](const Client& c) { return c.socket == client.socket; }), clients.end());
    pthread_mutex_unlock(&client_mutex);

    std::cout << "Client " << client.address << " disconnected" << std::endl;
    return nullptr;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Создаем сокет
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Настройка адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Привязка сокета к порту
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return -1;
    }

    // Ожидание подключения
    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        return -1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    // Основной цикл, принимающий подключения
    while (true) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Получаем IP-адрес клиента
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::string client_address(client_ip);

        // Добавляем клиента в список
        Client new_client = {client_socket, client_address};
        pthread_mutex_lock(&client_mutex);
        clients.push_back(new_client);
        pthread_mutex_unlock(&client_mutex);

        std::cout << "New client connected: " << client_address << std::endl;

        // Создаем поток для нового клиента
        pthread_t client_thread;
        pthread_create(&client_thread, nullptr, handle_client, (void*)&new_client);
        pthread_detach(client_thread);
    }

    return 0;
}
