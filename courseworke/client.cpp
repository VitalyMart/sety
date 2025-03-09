#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fstream>

#define BUFFER_SIZE 2048

int client_socket;
bool running = true;

// Функция для получения сообщений от сервера
void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    while (running) {
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            std::cerr << "Server disconnected or error occurred.\n";
            running = false;
            break;
        }
        buffer[bytes_received] = '\0';
        std::cout << buffer << std::flush; // Принудительно сбрасываем буфер вывода
    }
    return nullptr;
}

// Функция для ввода сообщений
void *input_messages(void *arg) {
    std::string message;
    while (running) {
        std::getline(std::cin, message);
        if (message == "/quit") {
            running = false;
            break;
        }
        send(client_socket, message.c_str(), message.size(), 0);
    }
    return nullptr;
}

// Функция для подключения к серверу
bool connect_to_server(const char *server_ip, int server_port) {
    struct sockaddr_in server_addr;

    // Создаем сокет
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        std::cerr << "Error creating socket\n";
        return false;
    }

    // Настраиваем адрес сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported\n";
        return false;
    }

    // Подключаемся к серверу
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed\n";
        return false;
    }

    return true;
}

// Функция для чтения порта из файла
int read_port_from_file(const std::string &filename) {
    std::ifstream port_file(filename);
    int port = 0;
    if (port_file.is_open()) {
        port_file >> port;
        port_file.close();
    } else {
        std::cerr << "Error reading port from file\n";
    }
    return port;
}

int main() {
    const char *server_ip = "172.26.141.157"; // Захардкоженный IP сервера
    int server_port = read_port_from_file("port.txt"); // Чтение порта из файла

    if (server_port == 0) {
        std::cerr << "Invalid port number. Exiting...\n";
        return 1;
    }

    // Попытка подключения к серверу
    while (!connect_to_server(server_ip, server_port)) {
        std::cerr << "Failed to connect to the server. Retrying in 5 seconds...\n";
        sleep(5); // Ждем 5 секунд перед повторной попыткой
    }

    std::cout << "Connected to the server. Enter your name: ";
    std::string name;
    std::getline(std::cin, name);

    // Отправляем имя серверу
    send(client_socket, name.c_str(), name.size(), 0);

    // Создаем поток для получения сообщений
    pthread_t receive_thread;
    pthread_create(&receive_thread, nullptr, receive_messages, nullptr);

    // Создаем поток для ввода сообщений
    pthread_t input_thread;
    pthread_create(&input_thread, nullptr, input_messages, nullptr);

    // Ожидаем завершения потока ввода сообщений
    pthread_join(input_thread, nullptr);

    // Останавливаем поток получения сообщений
    running = false;
    pthread_join(receive_thread, nullptr);

    close(client_socket);
    std::cout << "Disconnected from the server.\n";
    return 0;
}