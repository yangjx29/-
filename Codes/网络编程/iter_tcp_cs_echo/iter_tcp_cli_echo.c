#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_BUFFER_SIZE 1024

// 处理客户端业务的函数
void cli_biz(int connfd)
{
    char command[MAX_BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;

    while (1)
    {
	memset(command, 0, sizeof(command)); // 清空command数组
        // 读取用户命令行指令
        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            perror("fgets");
            break;
        }

	// 输出请求信息至 stdout
        printf("[ECH_RQT]%s", command);
        // 检查退出指令
        if (strncmp(command, "EXIT", 4) == 0)
        {
            break;
        }

        if (strncmp(command, "exit", 4) == 0)
        {
            break;
        }

        //在command末尾加上/0
        strcpy(command, command); // 将src复制到dest，并自动在末尾添加空字符

        // 发送请求PDU给服务器
        bytes_written = write(connfd, command, strlen(command));
        if (bytes_written == -1)
        {
            perror("write error");
            break;
        }

        char response[MAX_BUFFER_SIZE];
        memset(response, 0, sizeof(response));
        // 接收服务器返回的响应PDU
        bytes_read = read(connfd, response, sizeof(response));
        if (bytes_read == -1)
        {
            perror("read");
            break;
        }
        else if (bytes_read == 0)
        {
            printf("[cli] Server closed the connection.\n");
            break;
        }

        // 输出响应信息至 stdout
        printf("[ECH_REP]%s", response);
    }

    // 关闭连接
    close(connfd);
    printf("[cli] connfd is closed!\n");
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <ip_address> <port>\n", argv[0]);
        return 1;
    }

    const char *ip_address = argv[1];
    int port = atoi(argv[2]);

    int connfd;

    // 创建套接字
    connfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connfd == -1)
    {
        perror("socket");
        return 1;
    }

    // 设置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_address);
    server_addr.sin_port = htons(port);

    // 连接服务器
    if (connect(connfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect");
        return 1;
    }

    char server_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(server_addr.sin_addr), server_ip, sizeof(server_ip));
    int server_port = ntohs(server_addr.sin_port);
    printf("[cli] server[%s:%d] is connected!\n", server_ip, server_port);

    // 执行客户端业务
    cli_biz(connfd);

    printf("[cli] client is to return!\n");

    return 0;
}

