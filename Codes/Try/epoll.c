#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

int main() {
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == 0) {
        perror("socket");
        exit(0);
    }
    struct sockaddr_in addr;
    addr.sin_port = htons(9999);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(lfd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) {
        perror("bind");
        exit(0);
    }

    listen(lfd, 8);
    // 用epoll_create创建实力
    int epfd = epoll_create(1);
    // 将监听的文件描述符加入
    struct epoll_event epev;
    epev.data.fd = lfd;
    epev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &epev);

    // 保存内核检测之后返回的文件描述符信息
    struct epoll_event epevs[1024];
    while (1) {
        ret = epoll_wait(epfd, epevs, 1024, -1);
        if (ret == -1) {
            perror("epoll_wait");
            exit(0);
        }
        // ret存储的是文件描述符发生变化的数量
        printf("ret = %d\n", ret);
        for (int i = 0; i < ret; i++) {
            if (epevs[i].data.fd == lfd) {
                // 监听的文件描述符有数据,即有新的连接
                struct sockaddr_in cliaddr;
                socklen_t len = sizeof(cliaddr);
                int cfd = accept(lfd, (struct sockaddr*)&cliaddr, &len);
                // 封装进内核中
                epev.events = EPOLLIN;
                epev.data.fd = cfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &epev);
            } else {
                if (epevs[i].events & EPOLLOUT) {
                    printf("写数据");
                    continue;
                }
                // 有数据到达,需要通信
                char buf[1024] = {0};
                int read_len = read(epevs[i].data.fd, buf, sizeof(buf));
                if (read_len == -1) {
                    perror("read");
                    exit(0);
                } else if (read_len == 0) {
                    printf("client %d closed...\n", epevs[i].data.fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, epevs[i].data.fd, NULL);
                } else if (read_len > 0) {
                    printf("read buf: %s\n", buf);
                    write(epevs[i].data.fd, buf, strlen(buf) + 1);
                }
            }
        }
        close(lfd);
        close(epfd);
        return 0;
    }
}
