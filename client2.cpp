#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<cstring>
#include<thread>
#include<chrono>
#include<string>
#include<mutex>
#include<unistd.h>
#include<cstring>
#include<cstdlib>
using namespace std;
int client2 = socket(AF_INET,SOCK_STREAM,0);


void sendMessage(int clientScoket, string message){
    while(true){
    cout<<"Enter the message to send to server: ";
    getline(cin, message);
    if(message.size() == 0){
        cout<<"You dont wrote the message: "<<endl;
        getline(cin, message);
        if(message.size() == 0){
            cout<<"You dont wrote the message one more. Progarm will end"<<endl;
            break;
        }
        else{
        message += '\n';
        int totalsend = 0;
        int byte = message.length();
        while(byte > 0){
            int n = send(clientScoket, message.c_str() + totalsend, byte, 0);
            if(n<= 0){cout<<"Error"<<'\n'; break;}
            totalsend += n;
            byte -= n;
        }
        }
    }
    else{
        message += '\n';
        int totalsend = 0;
        int byte = message.length();
        while(byte > 0){
            int n = send(clientScoket, message.c_str() + totalsend, byte, 0);
            if(n<= 0){cout<<"Error"<<'\n'; break;}
            totalsend += n;
            byte -= n;
        }
    }
    }
    }
    
    
    


void readyMessage(int clientSocket){
    string col = "";
    while(true){  
    char buffer[1024] = {};  
    int rec = recv(clientSocket, buffer, sizeof(buffer), 0);
    if(rec < 0){
        cout<<"message error"<<endl;
        exit(0);
    }
    else if(rec == 0){
        cout<<"Server Error"<<endl;
        exit(0);
    }
    else{
        col.append(buffer, rec);
        size_t pos;
        while((pos = col.find('\n')) != string::npos){
            string pas = col.substr(0, pos + 1);
            cout<<"Message: "<<pas<<'\n';
            col.erase(0, pos + 1);
        }
    }
    }
}

bool chec(int clientSocket)
{
    char buffer[1024] = {};
    int rec = recv(clientSocket, buffer, sizeof(buffer), 0);
    if(rec < 0){
        cout<<"message error"<<endl;
        return false;
    }
    else if(rec == 0){
        cout<<"Server Error"<<endl;
        return false;
    }
    else if(strcmp(buffer, "Server found client 2")== 0){
        return true;
    }
    if(strcmp(buffer, "Waiting for another client to connect...") == 0){
    char buffer2[1024] = {};
    int buf = recv(clientSocket, buffer2, sizeof(buffer2), 0);
    if(buf < 0){
        cout<<"message error"<<endl;
        return false;
    }   
    else if(buf == 0){
        cout<<"Server Error"<<endl;
        return false;
    }
    if(strcmp(buffer2, "No other client connected. Closing connection.")== 0){
        cout<<"No client in server"<<endl;
        return false;
    }
    else if(strcmp(buffer2, "Server found client 2")== 0){
            cout<<"There is another client now you can send the messages"<<endl;
            return true;
    }
    else if(strcmp(buffer, "Server found client 2")== 0){
        return true;
    }else{
        return false;
    }
    }else{
        return false;
    }
}



int main(){
    if(client2 < 0 ){
        cout<<"Socket Error"<<endl;
        return 0;
    }
    sockaddr_in serverAddress{};
    serverAddress.sin_len = sizeof(serverAddress);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    int ch = inet_pton(AF_INET, "IP", &serverAddress.sin_addr);
    if(ch ==  1){
        cout<<"connect to server"<<endl;
    }
    else if(ch == 0){
        cout<<"Wrong IP"<<endl;
        return 0;
    }
    else if(ch == -1){
        cout<<"Error"<<endl;
        return 0;
    }
    if (connect(client2, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        perror("connect");
        close(client2);
        return 1;
    }

    string messages;

    bool red = chec(client2);
    if(red == true){
    thread receiver(readyMessage, client2);
    receiver.detach();
    sendMessage(client2, messages);
    }
    else if(red == false){
        close(client2);
        return 0;
    }
    close(client2);
    return 0;


}
