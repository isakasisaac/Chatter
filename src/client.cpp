#include <iostream>
#include <string>
#include <winsock2.h>

#define CHECKERROR(msg) if (err == -1) std::cout << msg << " error: " << WSAGetLastError() << std::endl;

int sockID;

int err;
sockaddr_in partner;
int partnerSize = sizeof(partner);

const int bufferSize = 1024;
char buffer[bufferSize];
int recieved;
const char *welcomeMessage = "Hello";
std::string input;

void listenToPartner(){
    recieved = recv(sockID, buffer, 1024, 0);
    err = recieved;
    CHECKERROR("Recieve")
    buffer[recieved] = 0;
    std::cout << "Partner: " << buffer << std::endl;
}
int writeToPartner(){
    std::cin >> input;
    if (input == "c") return 0;
    const char *inC = input.c_str();
    send(sockID, inC, strlen(inC), 0);
    delete[] inC;
    return 1;
}

void setupSocket(){
    sockID = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockID == INVALID_SOCKET){
        err = -1;
        CHECKERROR("Creating socket")
    }
    // specifying the address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    std::cout << "Port to use: ";
    short port;
    std::cin >> port;
    std::cin.get();
    serverAddress.sin_port = htons(port);

    err = bind(sockID, (sockaddr *)&serverAddress, sizeof(serverAddress));
    CHECKERROR("Socket binding")
}

void connectServer(){
    std::cout << "Waiting for connection..." << std::endl;
    do{
        err = recvfrom(sockID, buffer, 1024, MSG_PEEK, (sockaddr *)&partner, &partnerSize);
        CHECKERROR("Connecting server")
    } while (err == -1);
    char *ip = inet_ntoa(partner.sin_addr);
    std::cout << "Connection detected from " << ip << std::endl;
    delete[] ip;
    connect(sockID, (sockaddr *)&partner, sizeof(partner));
}

void connectClient(){
    partner.sin_family = AF_INET;
    while (true){
        std::cout << "IP to connect to: ";
        std::getline(std::cin, input);
        const char *input_c = input.c_str();
        long ulAddr = inet_addr(input_c);
        if (ulAddr == INADDR_NONE || ulAddr == INADDR_ANY){
            std::cout << "Invalid IP" << std::endl;
            continue;
        }
        partner.sin_addr.s_addr = ulAddr;

        std::cout << "Port to connect to: ";
        short port;
        std::cin >> port;
        partner.sin_port = htons(port);
        std::cout << "Trying connection to " << input << ":" << port << std::endl;
        err = connect(sockID, (sockaddr *)&partner, sizeof(partner));
        recieved = send(sockID, welcomeMessage, strlen(welcomeMessage), 0);
        std::cout << "s " << recieved << std::endl;
        if (recieved == SOCKET_ERROR){
            err = recieved;
            CHECKERROR("Connecting")
            continue;
        }

        if (err != -1){
            std::cout << "CONNECTION ESTABLISHED" << std::endl;
            break;
        }
    }
}

void cleanup(){
    closesocket(sockID);
    WSACleanup();
}

int main(){
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(wVersionRequested, &wsaData) != 0) std::cout << "STARTERRR" << std::endl;

    setupSocket();

    std::cout << "server OR client" << std::endl;
    std::getline(std::cin, input);
    if (input == "server" || input == "s") connectServer();
    else if (input == "client" || input == "c") connectClient();
    else{
        std::cout << "bad answer";
        cleanup();
        return -1;
    }

    // recieving data
    std::cout << "type \"c\" to close client" << std::endl;
    while (true){
        listenToPartner();
        if (writeToPartner() == 0) break;
    }

    cleanup();
    return 0;
}