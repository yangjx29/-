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

void sigchld_handler(int signum) {
    pid_t pid_chld;
    int stat;
    while (1) {
        // 若子进程为结束,该函数阻塞。使用WNOHANG则不阻塞,若没有子进程结束立即返回0
        pid_chld = waitpid(-1, &stat, WNOHANG);
        // if (pid_chld == -1) {
        //     perror("waitpid");
        //     exit(0);
        // }
        if (pid_chld <= 0) {
            break;
        }

        printf("[srv]%d [child]%d Child has terminated!\n", getpid(), getppid());
    }
}

// 当客户端关闭连接之后服务器再发送消息,内核会返回SIGPIPE
void sigpipe_handler(int signum) {
    printf("[srv]%d SIGPIPE is coming! client is closed\n", getpid());
}

// 处理接收发送数据的业务函数
void handler_client(int connfd, struct sockaddr_in cli_addr, short vert_code) {
    PDU p = {};
    // 获取客户信息
    char cli_ip[16];
    inet_ntop(AF_INET, &cli_addr.sin_addr, cli_ip, sizeof(cli_ip));
    int ip_port = ntohs(cli_addr.sin_port);

    size_t bytes_read;
    while (1) {
        bytes_read = read(connfd, &p.head, sizeof(p.head));
        if (bytes_read == -1) {
            perror("read");
            exit(0);
        }
        if (bytes_read == 0) {
            printf("bytes_read = 0, client closed!\n");
            return;
        } else if (bytes_read > 0) {
            read(connfd, p.payload, sizeof(p.payload));
            printf("[chd]:%d, [cid]:%hd, [request]: %s\n", getpid(), ntohs(p.head), p.payload);
            p.head = htons(vert_code);
            write(connfd, &p, sizeof(p));
        }
    }
}

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

    // 设置SIGCHLD信号捕捉器,最好不要用signal,这里是为了巩固下
    signal(SIGCHLD, sigchld_handler);

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

    // 使用多进程通信
    while (!sigint_flag) {
        // 建立接收连接的信息
        struct sockaddr_in cli_addr;
        socklen_t len = sizeof(cli_addr);

        int confd = accept(listenfd, (struct sockaddr*)&cli_addr, &len);
        if (confd == -1) {
            if (errno == EINTR) {
                // 注意此时不是错误
                continue;
            }
            perror("accept");
            exit(0);
        }
        char cli_ip[16];
        inet_ntop(AF_INET, &cli_addr.sin_addr, cli_ip, sizeof(cli_ip));
        int cli_port = ntohs(cli_addr.sin_port);
        printf("client is accepted! ip: %s, port: %d\n", cli_ip, port);
        // 多进程通信
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(0);
        }
        if (pid > 0) {
            close(confd);
        } else if (pid == 0) {
            // 子进程进行通信
            printf("child process is created! pid: %d, ppid: %d\n", getpid(), getppid());
            close(listenfd);                             // 注意这是在子进程中关闭监听套接字,在主进程依然在监听
            handler_client(confd, cli_addr, vert_code);  // 执行数据接收任务
            close(confd);                                // 子进程结束, 关闭通信套接字
            printf("child process is closed! pid: %d, ppid: %d\n", getpid(), getppid());
            return 0;
        }
    }
    close(listenfd);  // 这是在主进程中关闭监听套接字
    printf("[srv](%d) Server is exiting...\b", getpid());
    return 0;
}
