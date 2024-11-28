#include <iostream>
#include <cstring>    
#include <thread>             
#include <sys/socket.h>     
#include <netinet/in.h>       // sockaddr_in и для констант айпишек
#include <unistd.h>           // close, read

void requestProc(int clientSocket);

int main() {
    // создаем сокет
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); // AF_INET — IPv4, SOCK_STREAM — TCP, 0 - протокол по умолчанию
    if (serverSocket < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1; 
    }

    sockaddr_in serverAddr; // храним адрес 
    serverAddr.sin_family = AF_INET;                // IPv4
    serverAddr.sin_addr.s_addr = INADDR_ANY;        // привязываем ко всем доступным адресам ("слушать на всех доступных сетевых интерфейсах")
    serverAddr.sin_port = htons(8080);              // (host to network short) порт сервера преобразует в сетевой порядок байт 

    // привязываем сокет к адресу
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }

    if (listen(serverSocket, 5) < 0) { 
        std::cerr << "Listen failed" << std::endl;
        return 1;
    }

    std::cout << "Server is running on port 8080..." << std::endl;

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            continue; 
        }
        std::thread([clientSocket]() {
            requestProc(clientSocket); 
        }).detach(); // отсоединяем поток, чтобы работал независимо
    }

    close(serverSocket); 
    return 0;
}

void requestProc(int clientSocket) {
    char buffer[1024]; 
    std::cout << "Client connected" << std::endl;
    while (true) {
        memset(buffer, 0, sizeof(buffer));  // почистить буфер, всегда забываю
        int bytesRead = read(clientSocket, buffer, 1024);  
        if (bytesRead <= 0) {  
            std::cout << "Client disconnected" << std::endl;
            break;
        }
        std::cout << "Received: " << buffer << std::endl;  

        if (strcmp(buffer, "ping") == 0) {
            // const char *response = "ppong"; // проверка на неправильное 
            const char *response = "pong";
            send(clientSocket, response, strlen(response), 0); 
            std::cout << "Sent: " << response << std::endl;
        }
    }
    close(clientSocket);
}