// 引入所需的头文件
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<pthread.h>
#include<time.h>
#include<fcntl.h>

#define PORT 2345 // 定义服务器监听的端口号
#define MAXSIZE 2048 // 定义缓冲区的最大大小

// 定义客户端信息的结构体
struct client_info {
    int sockfd; // 客户端的套接字描述符
    struct sockaddr_in addr; // 客户端的地址信息
};

int client_count = 0; // 当前连接到服务器的客户端数量
pthread_mutex_t lock; // 用于同步对客户端数量的访问

// 处理客户端请求的函数
void *handle_client(void *arg) {
    struct client_info *info = (struct client_info *)arg; // 获取客户端信息
    int newsockfd = info->sockfd; // 获取客户端的套接字描述符
    char mybuf[512]; // 定义缓冲区，用于存储从客户端接收的消息

    // 打印客户端的连接信息
    printf("客户端连接成功: %s:%d\n", inet_ntoa(info->addr.sin_addr), ntohs(info->addr.sin_port));
    client_count++; // 增加客户端数量
    printf("当前客户端连接数量: %d\n", client_count); // 打印当前的客户端数量

    // 循环处理客户端的请求
    while(1){
        // 从客户端接收消息
        int len = recv(newsockfd, mybuf, sizeof(mybuf), 0);

        // 如果接收到的消息长度小于等于0，或者接收到的消息是"exit\n"，则断开与客户端的连接
        if(len <= 0 || strcmp(mybuf, "exit\n") == 0) {
            client_count--; // 减少客户端数量
            printf("客户端断开连接，当前客户端连接数量: %d\n", client_count); // 打印当前的客户端数量
            break;
        }

        // 打印从客户端接收到的消息
        printf("client %s:%d: %s", inet_ntoa(info->addr.sin_addr), ntohs(info->addr.sin_port), mybuf);

        // 构造服务器的响应消息
        char response[MAXSIZE];
        snprintf(response, sizeof(response), "server response from %s:%d: %s", inet_ntoa(info->addr.sin_addr), ntohs(info->addr.sin_port), mybuf);
        // 将响应消息发送给客户端
        send(newsockfd, response, strlen(response), 0);

        // 获取当前的时间
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        char time_str[64];
        sprintf(time_str, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

        // 构造日志消息
        char log[MAXSIZE];
        snprintf(log, sizeof(log), "%s: %s:%d: %s", time_str, inet_ntoa(info->addr.sin_addr), ntohs(info->addr.sin_port), mybuf);

        // 将日志消息写入到文件中
        pthread_mutex_lock(&lock); // 加锁，防止多个线程同时写入文件
        int fd = open("recv.txt", O_WRONLY | O_CREAT | O_APPEND, 0644); // 打开文件
        write(fd, log, strlen(log)); // 写入日志消息
        close(fd); // 关闭文件
        pthread_mutex_unlock(&lock); // 解锁

        // 清空缓冲区
        memset(mybuf, 0, sizeof(mybuf));
    }

    // 释放客户端信息的内存空间
    free(info);
    // 关闭与客户端的连接
    close(newsockfd);
    return NULL;
}

// 主函数
int main(int argc, char *argv[])
{
    int sockfd; // 服务器的套接字描述符
    struct sockaddr_in server_addr; // 服务器的地址信息
    struct sockaddr_in client_addr; // 客户端的地址信息
    int sin_size; // 地址信息的大小

    // 创建服务器的套接字
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "create socket failed\n");
        exit(EXIT_FAILURE);
    }

    // 初始化服务器的地址信息
    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 将服务器的套接字与地址信息绑定
    if(bind(sockfd, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr)) < 0){
        fprintf(stderr, "bind failed\n");
        exit(EXIT_FAILURE);
    }

    // 让服务器的套接字开始监听连接请求
    if(listen(sockfd, 10) < 0){
        perror("listen fail\n");
        exit(EXIT_FAILURE);
    }

    // 初始化地址信息的大小
    sin_size = sizeof(struct sockaddr_in);

    // 初始化互斥锁
    pthread_mutex_init(&lock, NULL);

    // 循环处理客户端的连接请求
    while(1){
        // 分配内存空间用于存储客户端的信息
        struct client_info *info = malloc(sizeof(struct client_info));
        // 接受客户端的连接请求
        info->sockfd = accept(sockfd, (struct sockaddr *)(&info->addr), &sin_size);
        // 如果接受连接请求失败，则释放内存空间并继续下一次循环
        if(info->sockfd < 0){
            perror("accept error");
            free(info);
            continue;
        }

        // 创建一个新的线程来处理客户端的请求
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, info);
    }
    // 销毁互斥锁
    pthread_mutex_destroy(&lock);

    // 关闭服务器的套接字
    close(sockfd);

    // 退出程序
    exit(EXIT_SUCCESS);
    return 0;
}
