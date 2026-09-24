#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<thread>
#include<chrono>
#include<vector>
#include <condition_variable>
#include<mutex>
using namespace std;

vector<int> clients;
char choice;
mutex mtx;
condition_variable cv;


void SendMessage(int clientSocket){
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (clients.size() == 1){
            cout << "Waiting for another client to connect..." << endl;

            bool connected = cv.wait_for(lock, std::chrono::seconds(60),[] {return clients.size() >= 2;});
            if (!connected){
                cout << "No other client connected. Closing connection." << endl;

                for (auto it = clients.begin(); it != clients.end(); ++it){
                    if (*it == clientSocket){
                        clients.erase(it);
                        break;
                    }
                }
                close(clientSocket);
                return;
            }
        }
    }
    while (true){
        char buffer[1024] = {};
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived <= 0){
            cout << "Client disconnected or error." << endl;

            std::lock_guard<std::mutex> lock(mtx);

            for (auto it = clients.begin(); it != clients.end(); ++it){
                if (*it == clientSocket){
                    clients.erase(it);
                    break;
                }
            }

            close(clientSocket);
            cv.notify_all();
            return;
        }

        int targetSocket = -1;{
            std::lock_guard<std::mutex> lock(mtx);

            for (int socket : clients){
                if (socket != clientSocket){
                    targetSocket = socket;
                    break;
                }
            }
        }
        if (targetSocket == -1){
            cout << "Waiting for another client to connect..." << endl;
            continue;
        }
        send(targetSocket, buffer, bytesReceived,0);
    }
}


int main()
{
    
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress{};
    serverAddress.sin_len = sizeof(serverAddress);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    inet_pton(AF_INET, "IP", &serverAddress.sin_addr);

    if (::bind(serverSocket,(sockaddr*)&serverAddress,sizeof(serverAddress)) < 0){
        perror("bind");
        close(serverSocket);
        return 1;
    }

    cout << "Server started" << endl;
    listen(serverSocket, 5);
    
    while(true){
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        {
            std::lock_guard<std::mutex> lock(mtx);
            clients.push_back(clientSocket);
            if (clients.size() >= 2) {
                cv.notify_all(); 
            }
        }
        thread t(SendMessage, clientSocket);
        t.detach();
    }
    close(serverSocket);
    return 0;
}