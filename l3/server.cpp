#include <iostream>
#include <fstream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BACKLOG 5
#define BUFFER_SIZE 1024
#define FILE_NAME "server_log.txt"

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    delete (int*)arg;
    char buffer[BUFFER_SIZE];

    while (true) {
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }

        buffer[bytes_received] = '\0';
        std::cout << "Получено: " << buffer << std::endl;

        
        pthread_mutex_lock(&file_mutex);
        std::ofstream file(FILE_NAME, std::ios::app);
        if (file.is_open()) {
            file << buffer << std::endl;
            file.close();
        }
        pthread_mutex_unlock(&file_mutex);

        // Отправка ответа клиенту
        int number = atoi(buffer);
        number *= 2;
        std::string response = std::to_string(number);
        send(client_socket, response.c_str(), response.length(), 0);
    }

    close(client_socket);
    return nullptr;
}

int main() {
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {    
        std::cerr << "Ошибка создания сокета" << std::endl;
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; 
    server_addr.sin_addr.s_addr = inet_addr("172.26.141.157"); 
    server_addr.sin_port = 0;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Ошибка привязки сокета" << std::endl;
        close(server_socket);
        return 1;
    }

    getsockname(server_socket, (struct sockaddr*)&server_addr, &client_len);
    std::cout << "Сервер запущен на IP: " << inet_ntoa(server_addr.sin_addr)
              << " порту: " << ntohs(server_addr.sin_port) << std::endl;

    listen(server_socket, BACKLOG);

    while (true) {
        int* client_socket = new int(accept(server_socket, (struct sockaddr*)&client_addr, &client_len));
        if (*client_socket < 0) {
            std::cerr << "Ошибка принятия подключения" << std::endl;
            delete client_socket;
            continue;
        }

        std::cout << "Подключен клиент: " << inet_ntoa(client_addr.sin_addr) 
                  << ":" << ntohs(client_addr.sin_port) << std::endl;

        pthread_t thread;
        pthread_create(&thread, nullptr, handle_client, client_socket);
        pthread_detach(thread);
    }

    close(server_socket);
    return 0;
}
