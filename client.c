#include<stdio.h>
#include<string.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#include <unistd.h>

int main(){
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2),&wsaData)!= 0)
    {
        printf("WSAStartup failed\n");
        return 1;
    }
    fd_set rfds; 
    struct sockaddr_in server;
    struct timeval tv;
    int sock, readSize, addressSize;

    int retval;
    int num1, num2, ans;

    memset(&server,0,sizeof(server));
    server.sin_family = PF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(5678);
    
    sock = socket(PF_INET,SOCK_DGRAM,0);
    if (sock == INVALID_SOCKET)
    {
        printf("socket valid failed");
        WSACleanup();
        return 1;
    }
    
    addressSize = sizeof(server);

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    
    tv.tv_sec=5;
    tv.tv_usec =0 ;

    scanf("%d",&num1);
    sendto(sock,(char*)&num1,sizeof(num1),0,(struct sockaddr*)&server,sizeof(server));

    retval = select(1, &rfds, NULL, NULL, &tv); 

    if(retval == -1) {
         perror("select error !"); 
    } else if (retval) { 
         scanf("%d", &num2);
         sendto(sock, (char*)&num2, sizeof(num2), 0, (struct sockaddr*)&server, sizeof(server));
    } else {
         num2 = 100;
         sendto(sock, (char*)&num2, sizeof(num2), 0, (struct sockaddr*)&server, sizeof(server));
    }

     
     readSize = recvfrom(sock, (char*)&ans, sizeof(ans), 0, (struct sockaddr*)&server, &addressSize);
     printf("Read Message: %d\n", ans);

     closesocket(sock);
     WSACleanup();
     return 0;
    
}