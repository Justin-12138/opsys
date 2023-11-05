// 引入所需的头文件
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include<errno.h>
#include<arpa/inet.h>

#define PORT 2345 // 定义服务器的端口号
#define SERVER_IP "10.33.71.232" // 定义服务器的IP地址

int main()
{
    int sockfd; // 客户端的套接字描述符
    struct sockaddr_in server_addr; // 服务器的地址信息

    // 创建客户端的套接字
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "Socket Error is %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // 初始化服务器的地址信息
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 连接到服务器
    if(connect(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) == -1){
        fprintf(stderr, "Connect failed\n");
        exit(EXIT_FAILURE);
    }

    // 打印连接成功的信息
    printf("连接成功: %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    char sendbuf[512]; // 定义发送缓冲区，用于存储要发送的消息
    char recvbuf[1024]; // 定义接收缓冲区，用于存储接收到的消息

    // 循环发送接收消息
    while(1){
        // 从标准输入获取消息
        fgets(sendbuf, sizeof(sendbuf), stdin);
        // 将消息发送给服务器
        send(sockfd, sendbuf, strlen(sendbuf), 0);

        // 如果发送的消息是"exit\n"，则退出循环
        if(strcmp(sendbuf, "exit\n") == 0) break;
        // 接收服务器的响应消息
        recv(sockfd, recvbuf, sizeof(recvbuf), 0);
        // 打印服务器的响应消息
        printf("收到服务端响应: %s", recvbuf);
        // 清空发送缓冲区和接收缓冲区
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }

    // 关闭客户端的套接字
    close(sockfd);
    // 退出程序
    exit(EXIT_SUCCESS);
    return 0;
}
