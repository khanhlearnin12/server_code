#include<winsock2.h>
#include<ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#include<stdio.h>
#include<string.h>


int main(){
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    struct sockaddr_in server,client;
    int sock, addressSize;
    int num1,num2,ans;

    memset(&server,0,sizeof(server));
    server.sin_family = PF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(5678);
    sock = socket(PF_INET,SOCK_DGRAM,0);
    if (sock == INVALID_SOCKET)
    {
        printf("socket creation failed");
        WSACleanup();
        return 1;
    }
    

    if(bind(sock,(struct sockaddr*)&server ,sizeof(server))== INVALID_SOCKET){
        printf("bind faile");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    addressSize =  sizeof(client);

    recvfrom(sock,(char*)&num1,sizeof(num1),0,(struct sockaddr *)&client,&addressSize);
    printf("Read Message: %d\n",num1);

    recvfrom(sock,(char*)&num2,sizeof(num2),0,(struct sockaddr *)&client,&addressSize);
    printf("Read Message: %d\n",num2);

    ans = num1 + num2;
    printf("Result: %d\n",ans);

    sendto(sock,(char*)&ans,sizeof(ans),0,(struct sockaddr *)&client,sizeof(client));

    closesocket(sock);
    WSACleanup();
    return 0;
}