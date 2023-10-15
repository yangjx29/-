#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 60

typedef struct {
    short head;
    char payload[MAX_BUFFER_SIZE + 2];
} PDU;

// 记录在线人数
int cnt = 0;

void set_nonblocking(int connfd) {
    int flag = fcntl(connfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(connfd, F_SETFL, flag);
}

int sigint_flag = 0;

void sigint_handler(int signum) {
    sigint_flag = 1;
    printf("[srv]%d SIGINT is coming\n", getpid());
}

// 当客户端关闭连接之后服务器再发送消息,内核会返回SIGPIPE
void sigpipe_handler(int signum) {
    printf("[srv]%d SIGPIPE is coming! client is closed\n", getpid());
}

// void handler_client(int connfd, struct sockaddr_in cli_addr, short vert_code) {
// PDU p = {};
// // 获取客户信息
// char cli_ip[16];
// inet_ntop(AF_INET, &cli_addr.sin_addr, cli_ip, sizeof(cli_ip));
// int ip_port = ntohs(cli_addr.sin_port);

// size_t bytes_read;
// while (1) {
//     bytes_read = read(connfd, &p.head, sizeof(p.head));
//     if (bytes_read == -1) {
//         if (errno == errno == EAGAIN || errno == EWOULDBLOCK) {
//             continue;
//         }

//         perror("read");
//         exit(0);
//     }
//     if (bytes_read == 0) {
//         printf("bytes_read = 0, client closed!\n");
//         break;
//     } else if (bytes_read > 0) {
//         read(connfd, p.payload, sizeof(p.payload));
//         printf("[chd]:%d, [cid]:%hd, [request]: %s\n", getpid(), ntohs(p.head), p.payload);
//         p.head = htons(vert_code);
//         write(connfd, &p, sizeof(p));
//     }
// }
// }

int main(int argc, char* argv[]) {
    if (argc != 4) {
        // argv[0] 一般是文件名称
        printf("Usage: %s tcp server echo needs <ip address> <port> <vert_code>\n", argv[0]);
        exit(0);
    }
    char* ip = argv[1];
    int port = atoi(argv[2]);
    short vert_code = atoi(argv[3]);

    // 设置SIGINT信号捕捉器
    struct sigaction sa_sigint;
    memset(&sa_sigint, 0, sizeof(sa_sigint));
    // sigemptyset(&sa_sigint.sa_mask); 加上会报错
    sa_sigint.sa_flags = 0;
    sa_sigint.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa_sigint, NULL);

    // 设置SIGPIPE信号捕捉器
    struct sigaction sa_sigpipe;
    memset(&sa_sigpipe, 0, sizeof(sa_sigpipe));
    // sigemptyset(&sa_sigpipe.sa_mask);
    sa_sigpipe.sa_flags = 0;
    sa_sigpipe.sa_handler = sigpipe_handler;
    sigaction(SIGPIPE, &sa_sigpipe, NULL);

    // 创建套接字
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket");
        exit(0);
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);
    int ret = bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        perror("bind");
        exit(0);
    }
    ret = listen(listenfd, 8);
    if (ret == -1) {
        perror("listen");
        exit(0);
    }
    printf("Server has initialized!\n");

    // 使用epoll多路复用技术
    // 创建epoll实例
    int epfd = epoll_create(1);
    // 将监听文件描述符的检测信息添加到内核中
    struct epoll_event epev;
    epev.data.fd = listenfd;
    epev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &epev);

    // 进行通信
    while (!sigint_flag) {
        // 创建一个结构体数组保存待会内核返回的文件描述符信息
        struct epoll_event epevs[1024];
        int ret = epoll_wait(epfd, epevs, 1024, -1);
        if (ret == -1) {
            perror("epoll_wait");
            exit(0);
        }

        char cli_ip[16];
        int cli_port;
        for (int i = 0; i < ret; i++) {
            if (epevs[i].data.fd == listenfd) {
                // 有新的客户端连接
                cnt++;
                printf("当前人数: %d\n", cnt);
                struct sockaddr_in cli_addr;
                socklen_t len = sizeof(cli_addr);
                int connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &len);
                if (connfd == -1) {
                    if (errno == EINTR) {
                        // 注意此时不是错误
                        continue;
                    }
                    perror("accept");
                    exit(0);
                }
                // 设置read非阻塞
                set_nonblocking(connfd);
                inet_ntop(AF_INET, &cli_addr.sin_addr.s_addr, cli_ip, sizeof(cli_ip));
                cli_port = ntohs(cli_addr.sin_port);
                printf("client is accepted! ip: %s, port: %d\n", cli_ip, cli_port);
                // 将新的连接的文件描述符也加入监听
                epev.data.fd = connfd;
                epev.events = EPOLLIN;
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &epev);
            } else {
                // 有数据到达
                // handler_client(epevs[i].data.fd, cli_addr, vert_code);
                PDU p = {};
                // 获取客户信息
                char cli_ip[16];
                int connfd = epevs[i].data.fd;
                size_t bytes_read;
                bytes_read = read(connfd, &p.head, sizeof(p.head));
                if (bytes_read == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        continue;
                    }
                    perror("read");
                    exit(0);
                }
                if (bytes_read == 0) {
                    printf("bytes_read = 0, client%d closed!\n", ntohs(p.head));
                    // 关闭对该文件描述符的监听
                    epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL);
                    close(epevs[i].data.fd);
                    cnt--;
                    printf("当前人数: %d\n", cnt);
                } else if (bytes_read > 0) {
                    read(connfd, p.payload, sizeof(p.payload));
                    if (strcmp(p.payload, "EXIT\n") == 0 || strcmp(p.payload, "exit\n") == 0 || strcmp(p.payload, "q\n") == 0) {
                        printf("client %d is closed!\n", ntohs(p.head));
                        epoll_ctl(epfd, EPOLL_CTL_DEL, epevs[i].data.fd, NULL);
                        close(epevs[i].data.fd);
                        cnt--;
                        printf("当前人数: %d\n", cnt);
                    }
                    printf("[chd]:%d, [cid]:%hd, [request]: %s\n", getpid(), ntohs(p.head), p.payload);
                    p.head = htons(vert_code);
                    write(connfd, &p, sizeof(p));
                }
            }
        }
    }

    close(listenfd);
    printf("[srv](%d) Server is exiting...\n", getpid());
    return 0;
}
