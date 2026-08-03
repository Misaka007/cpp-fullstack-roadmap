- [一、poll()](#一poll)
- [二、代码实现](#二代码实现)
  - [2.1、utili.h](#21utilih)
  - [2.2、ser.c](#22serc)
  - [2.3、cli.c](#23clic)
  - [2.4、运行结果](#24运行结果)

## 一、poll()

poll()系统调用和select()类似，也是轮询一定数量的文件描述符，以测试其是否有就绪者。

API函数：

```cpp
int poll(struct pollfd *fds, nfds_t nfds, int timeout);

// 参数：nfds+1；

struct pollfd {
    int fd;        /* file descriptor */
    short events;     /* requested events */
    short revents;    /* returned events */
};
```

**其中，fd成员是指定的文件描述符，events成员告诉poll监听哪些事件，revents是由内核修改。**

**有一些宏作为辅助事件参数；**

## 二、代码实现

### 2.1、utili.h

```cpp
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<assert.h>

#include<sys/select.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT  8787
#define LISTEN_QUEUE 5
#define SIZE 10
#define BUFFER_SIZE 256

#include<poll.h>
#define OPEN_MAX 1000
```

### 2.2、ser.c

```cpp
#include"../utili.h"


static int socket_bind(const char *ip, int port){
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addrSer;
    addrSer.sin_family = AF_INET;
    //addrSer.sin_addr.s_addr = inet_addr(ip);
    inet_pton(AF_INET, ip, &addrSer.sin_addr);
    addrSer.sin_port = htons(port);
    bind(listenfd, (struct sockaddr*)&addrSer, sizeof(struct sockaddr));
    return listenfd;
}

static void handle_connection(struct pollfd *connfds, int num){
    char buffer[BUFFER_SIZE];
    int i;
    for(i=1; i<=num; ++i){
        if(connfds[i].fd == -1) 
            continue;
        if(connfds[i].revents & POLLIN){
            recv(connfds[i].fd, buffer, BUFFER_SIZE, 0); 
            printf("Server accept client msg:>%s\n",buffer);
            send(connfds[i].fd, buffer, strlen(buffer)+1, 0); 
        }   
    }   
}
static void do_poll(int listenfd){
    struct sockaddr_in addrCli;
    int connfd;
    struct pollfd clientfds[OPEN_MAX];
    clientfds[0].fd = listenfd;
    clientfds[0].events = POLLIN;
    int i;
    for(i=1; i<OPEN_MAX; ++i){
        clientfds[i].fd = -1;
    }

    int nready;
    int max = 0;
    for(;;){
        nready = poll(clientfds, max+1, -1);
        if(nready == -1){
            perror("poll");
            return;
        }
        if(nready == 0){
            printf("Server Wait Time Out.\n");
            continue;
        }
        if(clientfds[0].revents & POLLIN){  //收发消息
            socklen_t len = sizeof(struct sockaddr);
            connfd = accept(listenfd, (struct sockaddr*)&addrCli, &len);
            int i;
            for(i=1; i<OPEN_MAX; ++i){
                if(clientfds[i].fd == -1){
                    clientfds[i].fd = connfd;
                    break;
                }
            }

            if(i == OPEN_MAX){
                printf("Over Load.\n");
                exit(1);
            }

            clientfds[i].events = POLLIN;
            max = (i>max ? i : max);
        }

        handle_connection(clientfds, max);  //建立连接
    }
}
int main(void){
    int listenfd;
    listenfd = socket_bind(SERVER_IP, SERVER_PORT);
    listen(listenfd, LISTEN_QUEUE);
    do_poll(listenfd);
    return 0;
}
```

### 2.3、cli.c

```cpp
#include"../utili.h"

static void handle_connection(int sockfd){
    struct pollfd pfds[2];
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;
    pfds[1].fd = STDIN_FILENO;
    pfds[1].events = POLLIN;

    char buffer[BUFFER_SIZE];
    for(;;){
        poll(pfds, 2, -1);  //-1表示永不超时
        if(pfds[0].revents & POLLIN){
            recv(sockfd, buffer, BUFFER_SIZE, 0); 
            printf("msg:> %s\n",buffer);
        }   
        if(pfds[1].revents & POLLIN){
            scanf("%s", buffer);
            //read(STDIN_FILENO, buffer, BUFFER_SIZE);
            send(sockfd, buffer, strlen(buffer)+1, 0); 
        }   
    }   
}
int main(void){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    struct sockaddr_in addrSer;
    addrSer.sin_family = AF_INET;
    addrSer.sin_port = htons(SERVER_PORT);
    addrSer.sin_addr.s_addr = inet_addr(SERVER_IP);
    connect(sockfd, (struct sockaddr*)&addrSer, sizeof(struct sockaddr));

    handle_connection(sockfd);
    return 0;
}
```

### 2.4、运行结果

> **文本化验证：** 运行测试后应覆盖正常输入、边界输入和错误路径；核对输出顺序、返回值与关键数据结构状态是否符合预期。

服务器端：

客户端1：

客户端2：




## 补充：poll 的事件处理规则

```text
pollfd[]: { fd, events(关注), revents(实际发生) }
```

- `poll` 没有 `FD_SETSIZE` 位图限制，但每轮仍需线性遍历 `pollfd` 数组，连接数很大时扩展性有限。
- `POLLERR`、`POLLHUP`、`POLLNVAL` 应优先处理；`POLLIN` 与 EOF 可以同时出现，读取返回 0 才表示对端有序关闭。
- 删除连接后应及时压缩或复用数组槽位，避免大量无效 fd 拉高扫描成本。

## 面试辅导

### 面试考点全景图

| 维度 | 回答要点 | 面试高频追问 |
| --- | --- | --- |
| 核心定义 | `poll` 属于网络编程，重点是Linux 并发、IPC、I/O 或网络服务中的协作机制。 | 它解决了什么问题，边界在哪里？ |
| 关键角色 | 客户端、服务端、内核对象、缓冲区、同步原语。 | 各角色的职责、依赖方向和生命周期是什么？ |
| 优点 | 通过清晰边界降低耦合，并让复杂度、资源或并发控制可被推理。 | 性能收益是否可量化？ |
| 代价 | 会引入额外抽象、状态管理或运行时/维护成本。 | 何时不应使用？ |
| 适用场景 | 需求满足本节的约束，并且需要明确演进、资源或并发边界时。 | 如何在现有工程中渐进式落地？ |

### 面试官追问链

1. **基础：** 请定义 `poll`，并用本节示例说明它解决的实际问题。
2. **机制：** 说明数据流、阻塞点、并发控制和故障恢复；关键对象、调用顺序或数据流是什么？
3. **深入：** 与相近 IPC、I/O 或并发模型相比，通信边界、拷贝次数、阻塞语义、扩展性和故障隔离有什么取舍？
4. **工程：** 出现异常、资源不足、并发竞争或输入非法时如何保证正确性？
5. **优化：** 在基准数据明确后，如何定位瓶颈并以复杂度、延迟或内存指标验证优化？

### 横向对比

| 对比项 | `poll` | 相近 IPC、I/O 或并发模型 | 选择建议 |
| --- | --- | --- | --- |
| 核心目标 | Linux 并发、IPC、I/O 或网络服务中的协作机制 | 解决相邻但边界不同的问题 | 先按问题边界选择，不按名词选择。 |
| 主要成本 | 抽象、状态与维护成本 | 成本模型不同，可能偏向性能或灵活性 | 用实际负载和团队维护能力评估。 |
| 适用条件 | 需要说明数据流、阻塞点、并发控制和故障恢复 | 需要不同的数据流、生命周期或调用方式 | 不能同时满足时，优先保证正确性与可观测性。 |

### 容易忽略的知识点

- 警惕部分读写、EINTR、惊群、伪唤醒、死锁、fd 泄漏和关闭时序。
- 面试回答要区分**语言/库的保证**与**当前实现的经验行为**，避免把未定义行为或实现细节当作标准。
- 先说明前置条件和失败路径；“能跑”不是工程正确性，资源回收、日志和测试同样是答案的一部分。

### 代码注释提示

```cpp
// 面试考点：这里说明 `poll` 的职责边界，而不是重复代码字面含义。
// 所有权/并发：明确谁创建、谁释放资源；共享状态必须说明同步策略。
// 复杂度：标注关键路径的时间和空间复杂度，说明优化前提。
// 扩展性：通过稳定接口隔离变化，避免让调用方依赖具体实现。
```

### 可能的面试题

- **可能的面试题：** “请结合 `poll` 说明设计/实现选择，并给出一个失败场景。”
- **简要答案：** 先画清调用与数据流，再分析阻塞、竞态、超时、背压和资源回收。
