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
    string col = "";
    while (true){
        char buffer[1024] = {};
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0){
            cout << "Client disconnected or error." <<'\n';
            std::lock_guard<std::mutex> lock(mtx);
        for (auto it = clients.begin(); it != clients.end(); ++it){
                string left ="[Server]: " + it->name + " left from the chat\n";
            if(it->socket == clientSocket){
                for(int i = 0; i < clients.size(); i++){
                if(clients[i].socket != clientSocket){
                    int totalsent = 0;
                    int byte = left.size();
                        while(0 < byte){
                            int n = send(clients[i].socket, left.c_str() + totalsent, byte, 0);
                            if(n <= 0){cout<<"Error"<<'\n'; break;}
                            totalsent += n;  
                            byte -= n; 
                        }
                }
                }
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
            if(pas.find("/nick ") == 0){
                string newname = pas.substr(6);
                newname.pop_back();
            if(newname.find(' ') == string::npos && newname.length() > 0){
                for(int i =0; i< clients.size(); i++){
                    if(clients[i].socket == clientSocket){
                        clients[i].name = newname;
                    }
                }
            }
                string servermessgae = "[Server]: Your name changed to " + newname + " \n";
                send(clientSocket, servermessgae.c_str(), servermessgae.size(), 0);
                col.erase(0, pos + 1);
                continue;
            }
            if(pas.find("/msg ") == 0){
                size_t space_pos = pas.find(' ', 5);
                if(space_pos != string::npos){
                    string target = pas.substr(5, space_pos - 5);
                    string text = pas.substr(space_pos + 1);
                    string send1 = "Unknow";
                    for(int i = 0; i < clients.size(); i++){
                        if(clients[i].socket == clientSocket){
                            send1 = clients[i].name;
                            break;
                        }
                    }
                    string full = "[Private from " + send1 + "]: " + text;
                    bool found = false;
                    for(int i = 0; i < clients.size(); i++){
                        if(clients[i].name == target){
                            int totalsent = 0;
                            int byte = full.size();
                            while(0 < byte){
                                int n = send(clients[i].socket, full.c_str() + totalsent, byte, 0);
                                if(n <= 0){cout<<"Error"<<'\n'; break;}
                                totalsent += n;  
                                byte -= n;
                        }
                    found = true;
                    break;
                    }
                }
                if (!found) {
                    string errorMsg = "[Server]: User " + target + " not found\n";
                    int totalsent = 0;
                    int byte = errorMsg.size();
                    while(0 < byte){
                        int n = send(clientSocket, errorMsg.c_str() + totalsent, byte, 0);
                        if(n <= 0){cout<<"Error"<<'\n'; break;}
                        totalsent += n;  
                        byte -= n;
                    }
                }
            }
            col.erase(0, pos + 1);
            continue;
        }
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


        
    




int main(){
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
            string join = "[Server]:  " + newclient.name + " joined to the chat\n";
            for(int i = 0; i < clients.size(); i++){
                if(clients[i].socket != clientSocket){
                    int totalsent = 0;
                    int byte = join.size();
                        while(0 < byte){
                            int n = send(clients[i].socket, join.c_str() + totalsent, byte, 0);
                            if(n <= 0){cout<<"Error"<<'\n'; break;}
                            totalsent += n;  
                            byte -= n; 
                        }
                }
            }
        }
        thread t(SendMessage, clientSocket);
        t.detach();
    }
    close(serverSocket);
    return 0;
}