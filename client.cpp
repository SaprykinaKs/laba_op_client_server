// клиент отправляет "ping" всегда
#include <iostream>
#include <cstring>  
#include <thread>            
#include <sys/socket.h>      
#include <arpa/inet.h>        // sockaddr_in и преобразует айпишки
#include <unistd.h>           // close, write

void sendMessages(int serverSocket);
void receiveMessages(int serverSocket);

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0); // AF_INET — IPv4, SOCK_STREAM — TCP, 0 - протокол по умолчанию
    if (clientSocket < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr; //  храним адрес 
    serverAddr.sin_family = AF_INET;                // IPv4
    serverAddr.sin_port = htons(8080);              // (host to network short) порт сервера преобразует в сетевой порядок байт 
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); // айпи адрес в бинарный формат

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection to server failed" << std::endl;
        return 1;
    }

    std::cout << "Connected to server" << std::endl;

    std::thread sender([&]() {
        sendMessages(clientSocket); 
    });
    std::thread receiver([&]() {
        receiveMessages(clientSocket);
    });

    sender.join();
    receiver.join();

    close(clientSocket); 
    return 0;
}


void sendMessages(int serverSocket) {
    while (true) {
        // const char *message = "pping";  // проверка на неправильное
        const char *message = "ping";  

        // send(serverSocket, message, strlen(message), 0); // последний параметр - флаг
        // ладно, можно MSG_NOSIGNAL поставить и при дисконекте хотя бы норм завершаться будет
        ssize_t bytesSent = send(serverSocket, message, strlen(message), MSG_NOSIGNAL); 
        if (bytesSent == -1) {
            std::cerr << "Error sending message: " << strerror(errno) << std::endl; 
            break;  
        } else if (bytesSent == 0) {
            std::cerr << "Connection closed by server" << std::endl;
            break;  // если соединение было закрыто сервером
        }

        std::cout << "Sent: " << message << std::endl;
        sleep(1); 
    }
}

void receiveMessages(int serverSocket) {
    char buffer[1024]; 
    while (true) {
        memset(buffer, 0, sizeof(buffer));  // и тут буфер почистить не забыть бы
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
    }
}