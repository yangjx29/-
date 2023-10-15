#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// #define MAXPDU 64
#define MAX_BUFFER_SIZE 1024
// 定义请求PDU结构体
typedef struct
{
    char operator;
    int64_t operand1;
    int64_t operand2;
} RequestPDU;

// 定义响应PDU结构体
typedef struct
{
    int64_t result;
} ResponsePDU;

int should_exit = 0;
int listenfd;

// SIGINT信号处理函数
void sig_int(int signo) {
    printf("[srv] SIGINT is coming!\n");
    should_exit = 1;

    // 关闭监听套接字
    close(listenfd);

    printf("[srv] listenfd is closed!\n");
}

// 业务函数处理请求和发送响应
void srv_biz(int connfd, int vert_code) {
    char request[MAX_BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;

    while (!should_exit) {
        char response[MAX_BUFFER_SIZE] = {0};
        memset(response, 0, sizeof(response));  // 清空response数组
        memset(request, 0, sizeof(request));    // 清空request数组
        // 接收客户端的请求PDU
        bytes_read = read(connfd, request, sizeof(request) - 1);
        if (bytes_read == -1) {
            perror("read");
            break;
        } else if (bytes_read == 0) {
            printf("[srv] Client closed the connection.\n");
            break;
        }
        // 输出请求信息至 stdout
        printf("[ECH_RQT]%s\n", request);

        // 发送响应PDU给客户端
        snprintf(response, sizeof(response), "(%d)%s\n", vert_code, request);
        bytes_written = write(connfd, response, strlen(response));
        if (bytes_written == -1) {
            perror("write");
            break;
        }
    }

    // 关闭连接
    close(connfd);
    printf("[srv] Client is closed!\n");
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s tcp srv arith. <ip address> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 设置SIGINT信号处理函数
    struct sigaction sa;
    sa.sa_handler = sig_int;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    const char *ip_address = argv[1];
    int port = atoi(argv[2]);
    int vert_code = atoi(argv[3]);

    // 创建监听套接字
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_address);
    server_addr.sin_port = htons(port);

    // 绑定套接字
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // 监听套接字
    if (listen(listenfd, 8) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("[srv] server[%s:%d][%d] is initializing!\n", ip_address, port, vert_code);

    while (!should_exit) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // 接受客户端连接请求
        int connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);
        if (connfd == -1) {
            if (should_exit) {
                // 若在accept期间收到SIGINT信号，则退出循环
                break;
            }
            perror("accept");
            continue;
        }

        // 输出客户端信息至stdout
        printf("[srv] client[%s:%d] is accepted!\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // 处理业务
        srv_biz(connfd, vert_code);

        // 关闭连接
        // close(connfd);

        // 输出已断开连接的客户端信息至stdout
        printf("[srv] client[%s:%d] is closed!\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }
    // close(listenfd);
    // printf("[srv] listenfd is closed!\n");
    printf("[srv] server is going to exit!\n");

    return 0;
}
