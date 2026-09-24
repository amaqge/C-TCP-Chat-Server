#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<cstring>
#include<thread>
#include<chrono>
#include<string>
using namespace std;

void sendMessage(int clientScoket, string message){
    while(true){
    cout<<"Enter the message to send to server: ";
    getline(cin, message);
    send(clientScoket, message.c_str(), message.length(), 0);
    }
}

void readyMessage(int clientSocket){
    while(true){
    char buffer[1024] = {};
    recv(clientSocket, buffer, sizeof(buffer), 0);
    cout<<"Message from Client2: "<<buffer<<endl;
    }
}


int main(){
    int client2 = socket(AF_INET,SOCK_STREAM,0);

    sockaddr_in serverAddress{};
    serverAddress.sin_len = sizeof(serverAddress);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    inet_pton(AF_INET, "IP", &serverAddress.sin_addr);

    if (connect(client2, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        perror("connect");
        close(client2);
        return 1;
    }

    string messages;
    thread receiver(readyMessage, client2);
    receiver.detach();
    sendMessage(client2, messages);
    close(client2);


}