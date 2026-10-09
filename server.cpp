#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<thread>
#include<chrono>
#include<vector>
#include<condition_variable>
#include<mutex>
#include<string>
using namespace std;

struct clientinfo{
    int socket; 
    string name;
};
vector<clientinfo>clients;
char choice;
mutex mtx;
condition_variable cv;
const int PORT = 8080;
int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
int clientcounter = 1;





void SendMessage(int clientSocket){
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (clients.size() == 1){
            cout << "Waiting for another client to connect..." <<'\n';
            send(clientSocket, "Waiting for another client to connect...", 40, 0 );
            bool connected = cv.wait_for(lock, std::chrono::seconds(60),[&] {return clients.size() >= 2;});
            if (!connected){
                cout << "No other client connected. Closing connection." <<'\n';
                send(clientSocket, "No other client connected. Closing connection.", 46, 0);
                for (auto it = clients.begin(); it != clients.end(); ++it){
                    if (it->socket == clientSocket){
                        clients.erase(it);
                        break;
                    }
                }
                close(clientSocket);
                return;
            }
        }
    }
    send(clientSocket, "Server found client 2", 21, 0);
    string col = "";
    while (true){
        char buffer[1024] = {};
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0){
            cout << "Client disconnected or error." <<'\n';
            std::lock_guard<std::mutex> lock(mtx);
            for (auto it = clients.begin(); it != clients.end(); ++it){
                if (it->socket == clientSocket){
                    clients.erase(it);
                    break;
                }
            }
            close(clientSocket);
            cv.notify_all();
            return;
        }
        col.append(buffer, bytesReceived);
        size_t pos;
        while((pos = col.find('\n')) != string::npos){
            string pas = col.substr(0, pos + 1);
            std::lock_guard<std::mutex> lock(mtx);
            string sendername;
            for(int i = 0; i< clients.size(); i++){
                if(clientSocket == clients[i].socket){
                    sendername = clients[i].name;
                }
            }
            string fullMessage = sendername + ": " + pas;
            for(int i = 0 ; i < clients.size(); i++){
                if(clients[i].socket!= clientSocket){
                    int totalsent = 0;
                    int byte = fullMessage.size();
                        while(0 < byte){
                            int n = send(clients[i].socket, fullMessage.c_str() + totalsent, byte, 0);
                            if(n <= 0){cout<<"Error"<<'\n'; break;}
                            totalsent += n;  
                            byte -= n; 
                        }
                }       
            }
            col.erase(0, pos + 1);
        }
    }
}


        
    




int main()
{
    sockaddr_in serverAddress{};
    serverAddress.sin_len = sizeof(serverAddress);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    if (::bind(serverSocket,(sockaddr*)&serverAddress,sizeof(serverAddress)) < 0){
        perror("bind");
        close(serverSocket);
        return 1;
    }

    cout << "Server started" << '\n';
    listen(serverSocket, 5);
    while(true){
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        {
            std::lock_guard<std::mutex> lock(mtx);
            clientinfo newclient;
            newclient.socket = clientSocket;
            newclient.name = "Client" + to_string(clientcounter);
            clientcounter ++;
            clients.push_back(newclient);
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