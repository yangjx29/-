#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 60

// 定义PDU
typedef struct
{
    short head;
    char payload[MAX_BUFFER_SIZE + 2];
} PDU;

int sigint_flag = 0;

void sigint_handler(int signum) {
    sigint_flag = 1;
    printf("[srv](%d) SIGINT is coming!\n", getpid());
}

void sigchld_handler(int signum) {
    pid_t pid_chld;
    int stat;
    while ((pid_chld = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("[srv](%d)[chd](%d) Child has terminated!\n", getpid(), pid_chld);
    }
}

// 当客户端关闭了,服务器再次发送数据时内核就会返回SIGPIPE信号
void sigpipe_handler(int signum) {
    printf("[srv](%d) SIGPIPE is coming!\n", getpid());
}

// 子进程业务函数
void handle_client(int connfd, struct sockaddr_in client_addr, short vert_code) {
    PDU p = {};
    // 获取客户端信息
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);

    // // 输出客户端连接信息
    // printf("[srv](%d)[cli_sa](%s:%d) client is accepted!\n", getpid(), client_ip, client_port);

    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_read;

    // 读取客户端请求
    // while ((bytes_read = read(connfd, buffer, sizeof(buffer) - 1)) > 0)
    while (1) {
        bytes_read = read(connfd, &p.head, sizeof(p.head));
        if (bytes_read == 0) {
            return;
        }
        read(connfd, p.payload, sizeof(p.payload));

        printf("[chd](%d)[cid](%hd)[ECH_RQT] %s\n", getpid(), ntohs(p.head), p.payload);
        // strcpy(p.payload, p.payload);
        //  将客户端原始数据镜像回送
        p.head = htons(vert_code);
        write(connfd, &p, sizeof(p));
    }
    // 注意，这下面起始号是执行不到的
    // 客户端连接已断开
    printf("[chd](%d)[ppid](%d)[cli_sa](%s:%d) Client is closed!\n", getpid(), getppid(), client_ip, client_port);

    // 关闭连接
    // close(connfd);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s tcp srv echo <ip address> <port> <vcd>\n", argv[0]);
        return 1;
    }

    char *ip_address = argv[1];
    int port = atoi(argv[2]);
    short vert_code = atoi(argv[3]);

    // 设置 SIGINT 信号处理器
    struct sigaction sa_int;
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa_int, NULL);

    // 设置 SIGCHLD 信号处理器
    struct sigaction sa_chld;
    memset(&sa_chld, 0, sizeof(sa_chld));
    sa_chld.sa_handler = sigchld_handler;
    sigaction(SIGCHLD, &sa_chld, NULL);

    // 设置 SIGPIPE 信号处理器
    signal(SIGPIPE, sigpipe_handler);

    // 创建套接字
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket");
        return 1;
    }

    // 设置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip_address, &(server_addr.sin_addr)) <= 0) {
        perror("inet_pton");
        return 1;
    }

    // 绑定套接字
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return 1;
    }

    // 监听套接字
    if (listen(listenfd, SOMAXCONN) < 0) {
        perror("listen");
        return 1;
    }

    // 输出服务器初始化信息 √
    printf("[srv](%d)[srv_sa](%s:%d)[vcd](%hd) Server has initialized!\n", getpid(), ip_address, port, vert_code);

    // 请求受理循环
    while (!sigint_flag) {
        // 接受客户端连接
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        // 接收客户端套接字
        int connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (connfd < 0) {
            if (errno == EINTR) {
                // accept() 被信号中断，重新执行
                continue;
            } else {
                perror("accept");
                return 1;
            }
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);
        // 输出客户端连接信息
        printf("[srv](%d)[cli_sa](%s:%d) client is accepted!\n", getpid(), client_ip, client_port);

        // 调用fork()创建子进程处理客户端业务
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        } else if (pid == 0) {
            // 子进程代码
            printf("[chd](%d)[ppid](%d) Child process is created!\n", getpid(), getppid());
            close(listenfd);                                // 关闭子进程中的监听套接字
            handle_client(connfd, client_addr, vert_code);  // 处理客户端业务
            close(connfd);
            printf("[chd](%d)[ppid](%d) connfd is closed!\n", getpid(), getppid());
            return 0;
        } else {
            // 父进程代码
            close(connfd);  // 关闭父进程中的连接套接字
        }
    }

    // 关闭监听套接字
    close(listenfd);

    // 输出服务器退出信息
    printf("[srv](%d) Server is exiting...\n", getpid());

    return 0;
}
