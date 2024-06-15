#include <iostream>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#endif


#define CHECKERROR(msg) if (err == -1) std::cout << msg << " error: " << WSAGetLastError() << std::endl;

int sockID;

int err;
sockaddr_in partner;
int partnerSize = sizeof (partner);

const int bufferSize = 1024;
char buffer[bufferSize];
int recieved;
const char *welcomeMessage = "Hello";

void listenToPartner(){
    recieved = recv(sockID, buffer, 1024, 0);
    err = recieved;
    CHECKERROR("Recieve")
    buffer[recieved] = 0;
    std::cout << "Partner: " << buffer << std::endl;
}
int writeToPartner(){
    std::string in;
    std::cin >> in;
    if (in == "c") return 0;
    const char *inC = in.c_str();
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
    CHECKERROR("Socket binding");
    sockaddr_in a;
    int s = sizeof(a);
    err = getsockname(sockID, (sockaddr *)&a, &s);
    CHECKERROR("CHECKING IP ")
    std::cout << "Reserved socket with port " << htons(a.sin_port) << " and IP " << inet_ntoa(a.sin_addr) << " tried " << inet_ntoa(serverAddress.sin_addr) << std::endl;
}

void connectServer(){
    std::cout << "Waiting for connection..." << std::endl;
    do{
        err = recvfrom(sockID, buffer, 1024, MSG_PEEK, (sockaddr *)&partner, &partnerSize);
        CHECKERROR("Connecting server");
    } while (err == -1);
    char *ip = inet_ntoa(((sockaddr_in *)&partner)->sin_addr);
    std::cout << "Connection detected from " << ip << std::endl;
    delete[] ip;
    connect(sockID, (sockaddr *)&partner, sizeof(partner));
}

void connectClient(){
    partner.sin_family = AF_INET;
    bool connected = false;
    while (!connected){
        std::cout << "IP to connect to: ";
        std::string input;
        std::getline(std::cin, input);
        const char *input_c = input.c_str();
        long ulAddr = inet_addr(input_c);
        if ( ulAddr == INADDR_NONE || ulAddr == INADDR_ANY) {
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
            CHECKERROR("Connecting");
            continue;
        }
        std::cout << "CONNECTION ESTABLISHED" << std::endl;

        if (err != -1) connected = true;
    }
}

void cleanup(){
    closesocket(sockID); 
    WSACleanup();
}

int main() {
    #ifdef _WIN32
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(wVersionRequested, &wsaData) != 0){
        std::cout << "STARTERRR" << std::endl;
    }
    #endif

    setupSocket();

    std::cout << "server OR client" << std::endl;
    std::string choice;
    std::getline(std::cin, choice);
    if (choice == "server" || choice == "s") connectServer();
    else if (choice == "client" || choice == "c") connectClient();
    else{
        std::cout << "bad answer";
        cleanup();
        return -1;
    }
    
    // recieving data
    std::cout << "type \"c\" to close client" << std::endl;
    while (true) {
        listenToPartner();
        if (writeToPartner() == 0) break;
    }

    cleanup();
    return 0;
}