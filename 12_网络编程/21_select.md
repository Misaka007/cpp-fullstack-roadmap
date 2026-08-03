- [一、Linux I/O多路复用](#一linux-io多路复用)
- [二、select()模式](#二select模式)
- [三、select()机制分析](#三select机制分析)
- [四、I/O复用的本质](#四io复用的本质)
- [五、代码实现](#五代码实现)
  - [5.1、utili.h](#51utilih)
  - [5.2、ser.c](#52serc)
  - [5.3、cli.c](#53clic)
  - [5.4、运行结果](#54运行结果)

## 一、Linux I/O多路复用

之前：我们的处理是，每到来一个客户端，都为其开辟一个新的进/线程，对其进行一对一的服务，这是VIP的模式；在高并发情况下，将造成资源消耗过大。

现在，对应高并发：一个线程为多个客户服务；

**同一个时刻，只能为一个客户服务(作用排队);**

模型分析

此时就会产生select()、poll()、epoll()模式

## 二、select()模式

API函数：

```cpp
  int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
```

参数分析：nfds+1，在其后的读、写、异常、超时找模式运行；

要用到的函数：

## 三、select()机制分析

1. **recv、send、select......都是阻塞函数；但是在这里用阻塞函数--->解决非阻塞问题；**
2. **当可读事件发生时，区别两种情况：**
   - **请求与服务器的连接；**
   - **已经连接好了，直接进行通信；**
3. **每次都要重置，只留下一个客户端即可。**
4. **select()是轮询模式，走访所有的套接字；时间设置为0，不阻塞，直接返回。**

## 四、I/O复用的本质

**对其返回值(select().....)需要特别注意，<==>按不同的情况进行处理；**

**I/O复用只关心：服务器在跟哪个客户打交道；**

## 五、代码实现

### 5.1、utili.h

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
```

### 5.2、ser.c

```cpp
#include"../utili.h"

typedef struct server_context_st{
    int cli_cnt; //有多少个客户端
    int clifds[SIZE]; //客户端套接字集合
    fd_set allfds; //套接字集合
    int maxfd;     //套接字中最大的一个
}server_context_st;

static server_context_st *s_srv_ctx = NULL;

static void server_uninit(){
    if(s_srv_ctx){
        free(s_srv_ctx);
        s_srv_ctx = NULL;
    }   
}
static void server_init(){
    int i;
    s_srv_ctx = (server_context_st*)malloc(sizeof(server_context_st));
    assert(s_srv_ctx != NULL);
    memset(s_srv_ctx, 0, sizeof(server_context_st));
    for(i=0; i<SIZE; ++i){
        s_srv_ctx->clifds[i] = -1; 
    }   
}
static int create_server_proc(const char *ip, int port){
    printf("ip>%s\n",ip);
    printf("port:>%d\n",port);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addrSer;
    addrSer.sin_family = AF_INET;
    addrSer.sin_port = htons(port);
    addrSer.sin_addr.s_addr = inet_addr(ip);
    socklen_t len = sizeof(struct sockaddr);

    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    bind(fd, (struct sockaddr*)&addrSer, len);
    listen(fd, LISTEN_QUEUE);
    return fd;
}

static int accept_client_proc(int srvfd){
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(struct sockaddr);
    int clifd = accept(srvfd, (struct sockaddr*)&cliaddr, &len);

    printf("Server Accept Client Connect OK.\n");
    int i;
    for(i=0; i<SIZE; ++i){
        if(s_srv_ctx->clifds[i] == -1){
            s_srv_ctx->clifds[i] = clifd;
            s_srv_ctx->cli_cnt++;
            break;
        }
    }

    if(i == SIZE){
        printf("too many client.\n");
    }
}

static void handle_client_msg(int fd, char *buf){
    printf("recv buffer :>%s\n",buf);
    send(fd, buf, strlen(buf)+1, 0);
}

static void recv_client_msg(fd_set *readfds){
    int clifd;
    char buffer[BUFFER_SIZE];
    int i;

    for(i=0; i<=s_srv_ctx->cli_cnt; ++i){
        clifd = s_srv_ctx->clifds[i];
        if(clifd < 0){
            continue;
        }   
        if(FD_ISSET(clifd, readfds)){
            recv(clifd, buffer, BUFFER_SIZE, 0);
            handle_client_msg(clifd, buffer);
        }
    }
}

static void handle_client_proc(int srvfd){
    int clifd = -1;
    fd_set *readfds = &s_srv_ctx->allfds;

    int retval;

    int i;
    struct timeval tv;
    while(1){
        FD_ZERO(readfds);
        FD_SET(srvfd, readfds);
        s_srv_ctx->maxfd = srvfd;
        for(i=0; i<s_srv_ctx->cli_cnt; ++i){
            clifd = s_srv_ctx->clifds[i];
            FD_SET(clifd, readfds);
            s_srv_ctx->maxfd = (clifd > s_srv_ctx->maxfd ? clifd : s_srv_ctx->maxfd);
        }     
        //retval  =select(maxfd+1, NULL, NULL, readfds)

        tv.tv_sec = 0;
        tv.tv_usec = 0;
        retval = select(s_srv_ctx->maxfd+1, readfds, NULL, NULL, &tv);
        if(retval == -1){   //错误返回
            perror("select");
            return ;
        }
        if(retval == 0){   //处理超时
            printf("Server Wait Time Out.\n");
            continue;
        }

        if(FD_ISSET(srvfd, readfds)){
            accept_client_proc(srvfd); //处理客户端的连接
        }else{
            recv_client_msg(readfds); //服务器接收客户端的消息
        }

    }
}
int main(int argc, char *argv[]){
    server_init();
    int srvfd = create_server_proc(SERVER_IP, SERVER_PORT);
    handle_client_proc(srvfd);
    return 0;
}
```

### 5.3、cli.c

```cpp
#include"../utili.h"

static void handle_connection(int sockfd){
    fd_set readfds;
    int retval = 0;
    char buffer[BUFFER_SIZE];
    int maxfd;
    while(1){
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        maxfd = sockfd;

        retval = select(maxfd+1, &readfds, NULL, NULL, NULL);
        if(retval == -1){
            perror("select");
            return;
        }   

        if(FD_ISSET(sockfd, &readfds)){
            recv(sockfd, buffer, BUFFER_SIZE, 0); 
            printf("client recv self msg:> %s\n",buffer);
            //sleep(1);
            printf("Msg:>");
            scanf("%s",buffer);
            send(sockfd, buffer, strlen(buffer)+1, 0); 
        }   
    }   
}

int main(int argc, char *argv[]){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addrSer;
    addrSer.sin_family = AF_INET;
    addrSer.sin_port = htons(SERVER_PORT);
    addrSer.sin_addr.s_addr = inet_addr(SERVER_IP);
    int retval = connect(sockfd, (struct sockaddr*)&addrSer, sizeof(struct sockaddr));
    if(retval == -1){
        perror("connect");
        return -1;
    }else{
        printf("Client Connect Server OK.\n");
    }

    send(sockfd, "hello server.", strlen("hello server")+1, 0);
    handle_connection(sockfd);
    return 0;
}
```

### 5.4、运行结果

> **文本化验证：** 运行测试后应覆盖正常输入、边界输入和错误路径；核对输出顺序、返回值与关键数据结构状态是否符合预期。

服务器端：一直在等待客户端的连接，比较快，图不好截取；

客户端1：

客户端2：




## 补充：select 的使用边界

```text
每轮循环：复制 master fd_set -> select -> 遍历 [0, max_fd]
```

- `select` 会修改传入的 `fd_set`，下一轮必须重新设置或从 master 集合复制。
- `FD_SETSIZE` 限制可监控 fd 数量，且每次都需要线性扫描到 `max_fd`；适合连接数较少的兼容性场景。
- 可读事件不代表一次 `read` 就能读到完整应用层消息；仍需协议缓冲和分帧。

## 面试辅导

### 面试考点全景图

| 维度 | 回答要点 | 面试高频追问 |
| --- | --- | --- |
| 核心定义 | `select` 属于网络编程，重点是Linux 并发、IPC、I/O 或网络服务中的协作机制。 | 它解决了什么问题，边界在哪里？ |
| 关键角色 | 客户端、服务端、内核对象、缓冲区、同步原语。 | 各角色的职责、依赖方向和生命周期是什么？ |
| 优点 | 通过清晰边界降低耦合，并让复杂度、资源或并发控制可被推理。 | 性能收益是否可量化？ |
| 代价 | 会引入额外抽象、状态管理或运行时/维护成本。 | 何时不应使用？ |
| 适用场景 | 需求满足本节的约束，并且需要明确演进、资源或并发边界时。 | 如何在现有工程中渐进式落地？ |

### 面试官追问链

1. **基础：** 请定义 `select`，并用本节示例说明它解决的实际问题。
2. **机制：** 说明数据流、阻塞点、并发控制和故障恢复；关键对象、调用顺序或数据流是什么？
3. **深入：** 与相近 IPC、I/O 或并发模型相比，通信边界、拷贝次数、阻塞语义、扩展性和故障隔离有什么取舍？
4. **工程：** 出现异常、资源不足、并发竞争或输入非法时如何保证正确性？
5. **优化：** 在基准数据明确后，如何定位瓶颈并以复杂度、延迟或内存指标验证优化？

### 横向对比

| 对比项 | `select` | 相近 IPC、I/O 或并发模型 | 选择建议 |
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
// 面试考点：这里说明 `select` 的职责边界，而不是重复代码字面含义。
// 所有权/并发：明确谁创建、谁释放资源；共享状态必须说明同步策略。
// 复杂度：标注关键路径的时间和空间复杂度，说明优化前提。
// 扩展性：通过稳定接口隔离变化，避免让调用方依赖具体实现。
```

### 可能的面试题

- **可能的面试题：** “请结合 `select` 说明设计/实现选择，并给出一个失败场景。”
- **简要答案：** 先画清调用与数据流，再分析阻塞、竞态、超时、背压和资源回收。
