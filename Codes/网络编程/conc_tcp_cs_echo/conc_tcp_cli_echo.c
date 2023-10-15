/*多进程客户端*/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 60
// 定义PDU
typedef struct
{
    short head;
    char payload[MAX_BUFFER_SIZE + 2];
} PDU;

void cli_biz(int connfd, short cid) {
    PDU p = {};
    char input[MAX_BUFFER_SIZE];

    while (1) {
        fgets(p.payload, sizeof(p.payload), stdin);

        // 检查退出指令
        if (strcmp(p.payload, "EXIT\n") == 0 || strcmp(p.payload, "exit\n") == 0 || strcmp(p.payload, "q\n") == 0) {
            printf("[cli](%d)[cid](%hd)[ECH_RQT] EXIT\n", getpid(), cid);
            break;
        }
        p.head = htons(cid);

        // 输出p
        printf("[cli](%d)[cid](%hd)[ECH_RQT] %s", getpid(), cid, p.payload);

        // 在input末尾加上/0
        // strcpy(p.payload, p.payload);
        //  发送请求数据给服务器
        write(connfd, &p, sizeof(p));

        char buffer[MAX_BUFFER_SIZE];
        ssize_t bytes_read;

        // 接收服务器镜像返回的消息回声数据
        bytes_read = read(connfd, &p.head, sizeof(p.head));
        bytes_read = read(connfd, p.payload, sizeof(p.payload));
        if (bytes_read > 0) {
            // strcpy(p.payload, p.payload);
            printf("[cli](%d)[vcd](%hd)[ECH_REP] %s", getpid(), ntohs(p.head), p.payload);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s tcp cli echo.o <ip address> <port> <cid>\n", argv[0]);
        return 1;
    }

    char *ip_address = argv[1];
    int port = atoi(argv[2]);
    int cid = atoi(argv[3]);

    // 创建套接字
    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connfd < 0) {
        perror("socket");
        return 1;
    }

    // 设置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // 将点分十进制的ip地址转化为用于网络传输的数值格式
    if (inet_pton(AF_INET, ip_address, &(server_addr.sin_addr)) <= 0) {
        perror("inet_pton");
        return 1;
    }

    // 连接服务器
    if (connect(connfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        return 1;
    }

    // 输出服务器连接信息 ip和端口号
    printf("[cli](%d)[srv_sa](%s:%d) Server is connected!\n", getpid(), ip_address, port);

    // 执行业务函数
    cli_biz(connfd, cid);

    // 关闭连接
    close(connfd);

    // 输出提示信息
    printf("[cli](%d) connfd is closed!\n", getpid());
    printf("[cli](%d) Client is to return!\n", getpid());

    return 0;
}
