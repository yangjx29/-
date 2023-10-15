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
typedef struct {
    short head;
    char payload[MAX_BUFFER_SIZE];
} PDU;

void cli_biz(int confd, short cid) {
    PDU p = {};
    while (1) {
        // 从键盘接收数据
        fgets(p.payload, sizeof(p.payload), stdin);
        // 检查退出指令
        if (strcmp(p.payload, "exit\n") == 0 || strcmp(p.payload, "EXIT\n") == 0 || strcmp(p.payload, "q\n") == 0) {
            printf("client: %d, cid : %d exit!\n", getpid(), cid);
            break;
        }
        p.head = htons(cid);
        write(confd, &p, sizeof(p));
        size_t bytes_read;
        // 接收服务器返回的镜像数据
        memset(&p, 0, sizeof(p));
        bytes_read = read(confd, &p.head, sizeof(p.head));
        if (bytes_read == -1) {
            perror("read");
            exit(0);
        } else if (bytes_read == 0) {
            printf("server closed!\n");
            exit(0);
        } else if (bytes_read > 0) {
            bytes_read = read(confd, p.payload, sizeof(p.payload));
            printf("client: %d, vert_code: %d, reponse: %s", getpid(), ntohs(p.head), p.payload);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s tcp client: <ip address> <port> <cid>\n", argv[0]);
        return 1;
    }
    char* ip = argv[1];
    int port = atoi(argv[2]);
    int cid = atoi(argv[3]);
    // 创建套接字
    int confd = socket(AF_INET, SOCK_STREAM, 0);
    if (confd == -1) {
        perror("socket");
        exit(0);
    }
    // 设置服务器地址
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    // 建立连接
    int ret = connect(confd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        perror("connect");
        exit(0);
    }
    printf("client:%d,Server is connected! Server ip:%s, port: %d\n", getpid(), ip, port);
    cli_biz(confd, cid);

    // 关闭连接
    close(confd);
    // 输出提示信息
    printf("client: %d, connfd is closed\n", getpid());
    printf("client %d, client is to return\n", getpid());
    return 0;
}