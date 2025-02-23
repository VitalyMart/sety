#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BACKLOG 5  
#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    
    while (true) {
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }

        buffer[bytes_received] = '\0';
        std::cout << "Получено: " << buffer << std::endl;

        int number = atoi(buffer);
        number *= 2;  
        std::string response = std::to_string(number);

        send(client_socket, response.c_str(), response.length(), 0);
    }

    close(client_socket);
    exit(0);
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

    // Выводим PID родительского процесса (сервера)
    pid_t parent_pid = getpid();
    std::cout << "PID родительского процесса (сервера): " << parent_pid << std::endl;

    listen(server_socket, BACKLOG);
    signal(SIGCHLD, SIG_IGN);  

    while (true) {
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            std::cerr << "Ошибка принятия подключения" << std::endl;
            continue;
        }

        std::cout << "Подключен клиент: " << inet_ntoa(client_addr.sin_addr) 
                  << ":" << ntohs(client_addr.sin_port) << std::endl;
        
        pid_t pid = fork();
        if (pid == 0) {  
            std::cout << "Дочерний процесс PID: " << getpid() 
                      << ", родительский процесс PID: " << getppid() << std::endl;
            
            close(server_socket);  
            handle_client(client_socket);
        }
        close(client_socket);
    }

    close(server_socket);
    return 0;
}
