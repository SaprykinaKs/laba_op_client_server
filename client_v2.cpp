// клиент отправляет "ping" только после ответа от сервера "pong"
#include <iostream>
#include <cstring>  
#include <thread>            
#include <sys/socket.h>      
#include <arpa/inet.h>        // sockaddr_in и преобразует айпишки
#include <unistd.h>           // close, write
#include <atomic>             // флаг для синхронизации между потоками

void sendMessages(int serverSocket, std::atomic<bool>& allowed);
void incomingMessages(int serverSocket, std::atomic<bool>& allowed);

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection to server failed" << std::endl;
        return 1;
    }

    std::cout << "Connected to server" << std::endl;

    std::atomic<bool> allowed(true);  // флаг

    std::thread sender([&]() {
        sendMessages(clientSocket, allowed);
    });
    std::thread receiver([&]() {
        incomingMessages(clientSocket, allowed);
    });

    sender.join();
    receiver.join();

    close(clientSocket); 
    return 0;
}

void sendMessages(int serverSocket, std::atomic<bool>& allowed) {
    while (true) {
        if (allowed.load()) {
            // const char *message = "pping";  // проверка на неправильное
            const char *message = "ping";

            ssize_t bytesSent = send(serverSocket, message, strlen(message), 0);
            if (bytesSent == -1) {
                std::cerr << "Error sending message: " << strerror(errno) << std::endl;
                break; 
            }

            std::cout << "Sent: " << message << std::endl;
            allowed.store(false);  // блокируем отправку
            sleep(1); 
        }
    }
}

void incomingMessages(int serverSocket, std::atomic<bool>& allowed) {
    char buffer[1024]; 
    while (true) {
        memset(buffer, 0, sizeof(buffer));  
        int bytesRead = read(serverSocket, buffer, 1024);  
        if (bytesRead <= 0) {
            if (bytesRead == 0) {
                std::cout << "Server closed connection" << std::endl;
            } else {
                std::cerr << "Error receiving message: " << strerror(errno) << std::endl;
            }
            break;
        }

        std::cout << "Received: " << buffer << std::endl;
        if (strcmp(buffer, "pong") == 0) {
            allowed.store(true);  // разрешаем отправку
        }
    }
}
