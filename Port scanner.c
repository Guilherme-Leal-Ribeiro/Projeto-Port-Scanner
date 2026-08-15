#define _WIN32_WINNT 0x0600 //New version API

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT_TIMEOUT_US 200000 //Timeout for check port
#define IP_BUFFER_SIZE 64 //Size to IP

void startWinSock();
void menuScanner();
void scanner(int port, SOCKET sock, int* closed_count, int* filtered_count);
SOCKET socketCreate();
int socketConnect(SOCKET sock, int port, const char* ip);
void buildSocket(int port, const char* ip, int* closed_count, int* filtered_count);
int requestIp(char* ip_buffer);

//start socket library
void startWinSock(){
    WSADATA wsa;
    int result = WSAStartup(MAKEWORD(2,2), &wsa);

    if (result != 0) {
        printf("WSAStartup falhou com erro: %d\n", result);
        exit(1); 
    }
}

void menuScanner(){
    //ip address need to be correct
    printf("Digite o IP para escanear\n");
    char ip_buffer[IP_BUFFER_SIZE];
    if (requestIp(ip_buffer) == -1) {
        printf("Erro ao ler o IP\n");
        return;
    }

    //filtered/closed ports count
    int closed_count = 0;
    int filtered_count = 0;

    int port_range[2];
    printf("Digite o intervalo de portas para escanear\n");
    int reads = scanf("%d %d", &port_range[0], &port_range[1]);
    if (reads != 2 || port_range[0] > port_range[1] || port_range[0] <= 0 || port_range[1] > 65535) {
        printf("Intervalo invalido\n");
        return;
    }

    for(int i = port_range[0]; i <= port_range[1]; i++){
        buildSocket(i, ip_buffer, &closed_count, &filtered_count);
    }

    printf("%d Portas bloqueadas\n", closed_count);
    printf("%d Portas filtradas\n", filtered_count);
    printf("\n");
}

// reads IP as string
int requestIp(char* ip_buffer){
    if (fgets(ip_buffer, IP_BUFFER_SIZE, stdin) == NULL) {
        return -1;
    }

    ip_buffer[strcspn(ip_buffer, "\n")] = '\0';
    return 0;
}

SOCKET socketCreate(){
    //memory space to start connection
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("Erro ao criar socket: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    //turn socket into non-blocking
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);

    return sock;
}

//Flow creation/connection/scanner sockets
void buildSocket(int port, const char* ip, int* closed_count, int* filtered_count){
    SOCKET sock = socketCreate();
    if (sock == INVALID_SOCKET) {
        return;
    }

    if (socketConnect(sock, port, ip) == -1) {
        closesocket(sock);
        return;
    }

    scanner(port, sock, closed_count, filtered_count);
}

int socketConnect(SOCKET sock, int port, const char* ip){
    //socket protocol information
    struct sockaddr_in target = {0};
    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    target.sin_addr.s_addr = inet_addr(ip);

    //start connection with selected address
    int connect_result = connect(sock, (struct sockaddr*)&target, sizeof(target));
    if (connect_result == SOCKET_ERROR) {
        int code = WSAGetLastError();
        if (code != WSAEWOULDBLOCK) {
            printf("Erro ao conectar na porta %d: %d\n", port, code);
            return -1;
        }
    }
    return 0;
}

void scanner(int port, SOCKET sock, int* closed_count, int* filtered_count){
    //determine time to attempt writing on each socket

    fd_set write_set;
    FD_ZERO(&write_set);
    FD_SET(sock, &write_set);

    struct timeval timeout = {0};
    timeout.tv_usec = PORT_TIMEOUT_US;

    //determine if it's possible to write to the connections within the established time
    int select_result = select(0, NULL, &write_set, NULL, &timeout);
    if (select_result == SOCKET_ERROR) {
        printf("Erro a tentar testar escrita na porta %d\n", port);
        closesocket(sock);
        return;
    }

    //determine port state
    if(select_result > 0 && FD_ISSET(sock, &write_set)){
        int error = 0;
        int size = sizeof(error);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error, &size);

        if(error == 0){
            printf("\nPorta %d aberta\n", port);
        }
        else{
            (*closed_count)++;
        }
        }
    else{
        (*filtered_count)++;
    }

    closesocket(sock);
}

int main(){
    startWinSock();

    while(1){
        printf("1. Port Scanner\n");
        printf("0. Sair\n");

        int option;
        scanf("%d", &option);

        //clean buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        if(option == 1){
            menuScanner();
        }
        else if (option == 0){
            WSACleanup();
            exit(0);
        }
        else{
            printf("Opcao inexistente, digite outra\n");
        }
    }
}
