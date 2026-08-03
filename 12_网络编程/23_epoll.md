- [一、epoll()](#一epoll)
- [二、epoll_wait()](#二epoll_wait)
- [三、epoll()的核心思想](#三epoll的核心思想)
- [四、代码实现](#四代码实现)
  - [4.1、utili.h](#41utilih)
  - [4.2、ser.c](#42serc)
  - [4.3、cli.c](#43clic)
  - [4.4、运行结果](#44运行结果)

## 一、epoll()

epoll()是Linux特有的I/O复用函数，它的实现与使用上和select()、poll()、有很大差异。

epoll()用一组函数来完成任务，而不是单个函数；其次，epoll()把文件描述放到内核事件表中，只需一个额外的文件描述符，来标识内核中唯一的这个事件表。

需要使用的API：

```cpp
int epoll_create(int size);

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
```

需要使用的结构体信息：

```cpp
typedef union epoll_data {
  void        *ptr;
  int          fd;  //一般情况下，都用的是这个文件描述符
  uint32_t     u32;
  uint64_t     u64;
} epoll_data_t;

struct epoll_event {
  uint32_t     events;      /* Epoll events */
  epoll_data_t data;        /* User data variable */
};
```

```cpp
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
```

## 二、epoll_wait()

关键：对epoll_wait()函数的核心理解

1. **返回值：事件表中就绪客户端的个数；**
2. **参数events：将事件表中的就绪客户端的信息放到了events数组中。**

## 三、epoll()的核心思想

**就是创建一个内核事件表，存放所监听客户端的套接字和当前的事件，在利用epoll_wait()函数查找就绪的套接字，最后经过增加、删除、修改利用epoll_ctl()函数进行；当然了，这其中还有一批搭配使用的宏；**

## 四、代码实现

### 4.1、utili.h

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

#include<sys/epoll.h>
#define FDSIZE      1000
#define EPOLLEVENTS 100
```

### 4.2、ser.c

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

static void add_event(int epollfd, int fd, int state){
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

static void delete_event(int epollfd, int fd, int state){
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}
static void modify_event(int epollfd, int fd, int state){
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

static void handle_accept(int epollfd, int listenfd){
    int clifd;
    struct sockaddr_in addrCli;
    socklen_t len = sizeof(struct sockaddr);
    clifd = accept(listenfd, (struct sockaddr*)&addrCli, &len);
    if(clifd != -1){
        add_event(epollfd, clifd, EPOLLIN);
    }
}

static void do_read(int epollfd,  int fd, char *buf){
    int nread = read(fd, buf, BUFFER_SIZE);
    if(nread == -1){
        close(fd);
        delete_event(epollfd, fd, EPOLLIN);
    }else{
        printf("read msg:>%s\n",buf);
        modify_event(epollfd, fd, EPOLLOUT);
    }
}
static void do_write(int epollfd, int fd, char *buf){
    int nwrite = write(fd, buf, strlen(buf)+1);
    if(nwrite == -1){
        close(fd);
        delete_event(epollfd, fd, EPOLLOUT);
    } else{
        modify_event(epollfd, fd , EPOLLIN);
    }
    memset(buf, 0, BUFFER_SIZE);

}

static void handle_events(int epollfd, struct epoll_event *events, int num,
                            int listenfd, char *buf){
    int i;
    int fd;
    for(i=0; i<num; ++i){
        fd = events[i].data.fd;
        if((fd==listenfd) && (events[i].events&EPOLLIN)) //根据其结果分别进入三种状态
            handle_accept(epollfd, listenfd);  //申请与服务器连接
        else if(events[i].events & EPOLLIN)
            do_read(epollfd, fd, buf);  //只读
        else if(events[i].events & EPOLLOUT)        
            do_write(epollfd, fd, buf);  //只写
    }
}


static void do_epoll(int listenfd){
    int ret;
    char buffer[BUFFER_SIZE];
    struct epoll_event events[EPOLLEVENTS];
    int epollfd = epoll_create(FDSIZE);
    add_event(epollfd, listenfd, EPOLLIN);
    for(;;){
        //select poll
        ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        handle_events(epollfd, events, ret, listenfd, buffer);
    }
    close(epollfd);
}

int main(void){
    int listenfd;
    listenfd = socket_bind(SERVER_IP, SERVER_PORT);
    listen(listenfd, LISTEN_QUEUE);
    do_epoll(listenfd);
    return 0;
}
```

### 4.3、cli.c

```cpp
#include"../utili.h"

static void add_event(int epollfd, int fd, int state){
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

static void delete_event(int epollfd, int fd, int state){
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd; 
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

static void modify_event(int epollfd, int fd, int state){
    struct epoll_event ev; 
    ev.events = state;
    ev.data.fd = fd; 
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}



static void do_read(int epollfd,  int fd, int sockfd, char *buf){
    int nread = read(fd, buf, BUFFER_SIZE);
    if(nread == -1) {
        close(fd);
        delete_event(epollfd, fd, EPOLLIN);
    }else{
        if(fd == STDIN_FILENO)
            add_event(epollfd, fd, EPOLLIN);
        else{
            delete_event(epollfd, fd, EPOLLIN);
            add_event(epollfd, STDOUT_FILENO, EPOLLOUT);
        }
    }
    printf("Ser :>%s", buf);
}

static void do_write(int epollfd, int fd, int sockfd, char *buf){
    int nwrite = write(fd, buf, strlen(buf)+1);
    if(nwrite == -1){
        perror("write");
        close(fd);
    }else{
        if(fd == STDOUT_FILENO){
            delete_event(epollfd, fd, EPOLLOUT);
        }else{
            modify_event(epollfd, fd, EPOLLIN);
        }
    }    
    memset(buf, 0, BUFFER_SIZE);
}


static void handle_events(int epollfd, struct epoll_event *events, int num,
                            int sockfd, char *buf){
    int i;
    int fd;
    for(i=0; i<num; ++i){
        fd = events[i].data.fd;
        if(events[i].events & EPOLLIN)
            do_read(epollfd, fd, sockfd, buf);
        else if(events[i].events, fd, sockfd, buf)
            do_write(epollfd, fd, sockfd, buf);
    }
}
static void handle_connection(int sockfd){
    struct epoll_event events[EPOLLEVENTS];
    int epollfd = epoll_create(FDSIZE);
    add_event(epollfd, STDIN_FILENO, EPOLLIN);

    int ret;
    char buffer[BUFFER_SIZE];
    for(;;){
        ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        handle_events(epollfd, events, ret, sockfd, buffer);
    }
    close(epollfd);
}

int main(void){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addrSer;
    addrSer.sin_family = AF_INET;
    addrSer.sin_port = htons(SERVER_PORT);
    addrSer.sin_addr.s_addr = inet_addr(SERVER_IP);
    connect(sockfd, (struct sockaddr*)&addrSer, sizeof(struct sockaddr));

    handle_connection(sockfd);
    close(sockfd);
    return 0;
}
```

### 4.4、运行结果

> **文本化验证：** 运行测试后应覆盖正常输入、边界输入和错误路径；核对输出顺序、返回值与关键数据结构状态是否符合预期。

服务器端就是等待客户端的使用；

客户端1

客户端2

</br>

**利用epoll()函数，不用轮询每个套接字，效率更高效一些；**




## 补充：epoll 的触发与并发处理

| 模式 | 特点 | 处理要求 |
| --- | --- | --- |
| LT（水平触发） | 条件持续满足会重复通知 | 可逐次读取，逻辑简单 |
| ET（边缘触发） | 状态变化时通知一次 | 非阻塞 fd 必须读/写到 `EAGAIN` |
| ONESHOT | 一次事件后禁用 | 工作线程处理完成后 `MOD` 重新激活 |

- `epoll_ctl(ADD/MOD/DEL)` 管理兴趣集合，`epoll_wait` 只返回活跃事件，避免全量扫描。
- 不要将 fd 整数与指针混用存入 `data` 字段；连接对象释放前必须从 epoll 删除或确保事件引用不悬空。
- 多线程 `epoll_wait` 需要明确连接所有权与事件分发策略，避免重复处理和关闭竞态。

## 面试辅导

### 面试考点全景图

| 维度 | 回答要点 | 面试高频追问 |
| --- | --- | --- |
| 核心定义 | `epoll` 属于网络编程，重点是Linux 并发、IPC、I/O 或网络服务中的协作机制。 | 它解决了什么问题，边界在哪里？ |
| 关键角色 | 客户端、服务端、内核对象、缓冲区、同步原语。 | 各角色的职责、依赖方向和生命周期是什么？ |
| 优点 | 通过清晰边界降低耦合，并让复杂度、资源或并发控制可被推理。 | 性能收益是否可量化？ |
| 代价 | 会引入额外抽象、状态管理或运行时/维护成本。 | 何时不应使用？ |
| 适用场景 | 需求满足本节的约束，并且需要明确演进、资源或并发边界时。 | 如何在现有工程中渐进式落地？ |

### 面试官追问链

1. **基础：** 请定义 `epoll`，并用本节示例说明它解决的实际问题。
2. **机制：** 说明数据流、阻塞点、并发控制和故障恢复；关键对象、调用顺序或数据流是什么？
3. **深入：** 与相近 IPC、I/O 或并发模型相比，通信边界、拷贝次数、阻塞语义、扩展性和故障隔离有什么取舍？
4. **工程：** 出现异常、资源不足、并发竞争或输入非法时如何保证正确性？
5. **优化：** 在基准数据明确后，如何定位瓶颈并以复杂度、延迟或内存指标验证优化？

### 横向对比

| 对比项 | `epoll` | 相近 IPC、I/O 或并发模型 | 选择建议 |
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
// 面试考点：这里说明 `epoll` 的职责边界，而不是重复代码字面含义。
// 所有权/并发：明确谁创建、谁释放资源；共享状态必须说明同步策略。
// 复杂度：标注关键路径的时间和空间复杂度，说明优化前提。
// 扩展性：通过稳定接口隔离变化，避免让调用方依赖具体实现。
```

### 可能的面试题

- **可能的面试题：** “请结合 `epoll` 说明设计/实现选择，并给出一个失败场景。”
- **简要答案：** 先画清调用与数据流，再分析阻塞、竞态、超时、背压和资源回收。
