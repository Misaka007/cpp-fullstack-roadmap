- [一、C与Mysql](#一c与mysql)
- [二、C调用Mysql的基础模型](#二c调用mysql的基础模型)
- [三、C查询Mysql](#三c查询mysql)
- [四、C开发Mysql客户端](#四c开发mysql客户端)

## 一、C与Mysql

因为Mysql是用C语言开发的，所以会有一系列的API可以调用；

## 二、C调用Mysql的基础模型

```cpp
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mysql/mysql.h>

int main(void){
    int ret = 0;
    MYSQL mysql;
    MYSQL *connect = NULL;

    connect = mysql_init(&mysql);  //初始化
    if(connect == NULL){
        ret = -1; 
        printf("func mysql_init() err\n");
        return ret;
    }   
    connect = mysql_real_connect(connect, "localhost", "root", "123456", "mydb1", 0, NULL, 0); 
    if(connect == NULL){    //连接mysql
        ret = -1; 
        printf("func mysql_real_connect() err\n");
        return ret;
    }   
    printf("func mysql_real_connect() ok\n");

    mysql_close(&mysql);
    printf("hello world\n");
     
    return ret;
```

运行命令：

> gcc dm01_hello.c -o dm01_hello -I/usr/include -L/usr/lib64/mysql -lmysqlclient -lm -lrt -ldl -lstdc++ -lpthread

> **文本化验证：** 执行示例后应核对返回值、输出顺序和资源状态；同时覆盖空输入、边界输入与失败路径。

## 三、C查询Mysql

```cpp
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mysql/mysql.h>

/*
 *中文问题
 *mysql_query   查询
 *mysql_store_result 获取句柄
 *
 *locate mysql.h 可以查找这个.h文件所在的目录
 *
 *
 */
int main(void){
    int ret = 0;
    MYSQL mysql;
    MYSQL *connect = NULL;

    connect = mysql_init(&mysql);
    if(connect == NULL){
        ret = mysql_errno(&mysql);

        printf("func mysql_init() err\n");
        return ret;
    }

    connect = mysql_real_connect(connect, "localhost", "root", "123456", "mydb1", 0, NULL, 0);
    //中文问题的解决
    mysql_set_character_set(&mysql, "utf8");
    if(connect == NULL){
        ret = mysql_errno(&mysql);
        printf("func mysql_real_connect() err\n");
        return ret;
    }
    //查询
    const char *query = "select * from student";
    ret = mysql_query(&mysql, query);
    if(ret != NULL){
        ret = mysql_errno(&mysql);

        printf("func mysql_query() err\n");
        return ret;
    }
    //获取结果集和
    //结果集和中可能含有多行数据，获取结果集
    //mysql_store_result设计理念:告诉句柄,我一下子全部把数据从服务器端取到客户端,然后缓存起来
    MYSQL_RES *result = mysql_store_result(&mysql);
    //使用的过程中从服务器端获取结果
    //MYSQL_RES *result = mysql_use_result(&mysql);
    
    //可得该数据库中这张表每行有多少元素
    unsigned int num = mysql_field_count(&mysql);
    int i;
    MYSQL_ROW row = NULL;  //在mysql.h中可以看到
    //打印表头
    MYSQL_FIELD *fields = mysql_fetch_fields(result);
    for(i = 0; i < num; i++){
        printf("%s\t", fields[i].name);
    }
    printf("\n");
    //打印表中内容
    while(row = mysql_fetch_row(result)){
        for(i = 0; i < num; i++){
            printf("%s\t", row[i]);
        }
        printf("\n");
    }

/*
 *  这里是我们自己看到该表一行有多少元素
    while(row = mysql_fetch_row(result)){
        printf("%s, %s, %s, %s, %s, %s\n", row[0], row[1], row[2], row[3], row[4], row[5]);
    }
*/
    mysql_free_result(result);  
    
    mysql_close(&mysql);
    printf("hello world\n");

    return ret;   
}
```

> **文本化验证：** 执行示例后应核对返回值、输出顺序和资源状态；同时覆盖空输入、边界输入与失败路径。

## 四、C开发Mysql客户端

只实现了查询的功能：

```cpp
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mysql/mysql.h>

int main(int argc, char **argv){
    int ret = 0;
    MYSQL mysql;
    MYSQL *connect = NULL;
    char sqlbuf[80];

    connect = mysql_init(&mysql);
    if(connect == NULL){
        ret = mysql_errno(&mysql);

        printf("func mysql_init() err\n");
        return ret;
    }

    connect = mysql_real_connect(connect, "localhost", "root", "123456", argv[1], 0, NULL, 0);
    //中文问题的解决
    mysql_set_character_set(&mysql, "utf8");
    if(connect == NULL){
        ret = mysql_errno(&mysql);
        printf("func mysql_real_connect() err\n");
        return ret;
    }

    for(;;){
        memset(sqlbuf, 0, sizeof(sqlbuf));
        printf("mysql> :");
        //scanf()语句对tab 空格 回车 都省去了，对sql语句将会发生截断,用gets()可保持sql语句的原样性
        gets(sqlbuf);

        //退出
        if(strncmp("exit", sqlbuf, 4) == 0 || strncmp("quit", sqlbuf, 4) == 0){
            break;
        }
        //查询是否为SQL语句
        //ret = mysql_query(&mysql, "set name utf8");
        ret = mysql_query(&mysql, sqlbuf);
        if(ret != NULL){
            ret = mysql_errno(&mysql);

            printf("func mysql_query() err\n");
            return ret;
        }


        if(strncmp("select", sqlbuf, 6) == 0 || strncmp("SELECT", sqlbuf, 6) == 0){

            MYSQL_RES *result = mysql_store_result(&mysql);
            
            unsigned int num = mysql_field_count(&mysql);  //表头有多少列
            int i;     
            MYSQL_ROW row = NULL;  //在mysql.h中可以看到
            //打印表头
            MYSQL_FIELD *fields = mysql_fetch_fields(result);
            for(i = 0; i < num; i++){  //打印表头
                printf("%s\t", fields[i].name);
            }
            printf("\n");

            //打印表中内容
            while(row = mysql_fetch_row(result)){
                for(i = 0; i < num; i++){
                    printf("%s\t", row[i]);
                }
                printf("\n");
            }
            mysql_free_result(result);
        }
    }

    mysql_close(&mysql);
    printf("hello world\n");

    return ret;
}
```

1、看看mysql.h文件：

2、可以知道：MYSQL_ROW的真实类型：char **；

> **文本化验证：** 执行示例后应核对返回值、输出顺序和资源状态；同时覆盖空输入、边界输入与失败路径。

3、看看Mysql：

4、由于客户端的C语言开发数据库，我只实现了查询功能，其他的功能没有实现，导致没有打印出来，但是现在已经可以通过这个客户端对数据库进行操作了；





## 面试辅导

### 面试考点全景图

| 维度 | 回答要点 | 面试高频追问 |
| --- | --- | --- |
| 核心定义 | `MySQL客户端` 属于内核与数据库，重点是系统边界、工具链或客户端接口相关的工程机制。 | 它解决了什么问题，边界在哪里？ |
| 关键角色 | 调用方、系统接口、资源句柄、错误码。 | 各角色的职责、依赖方向和生命周期是什么？ |
| 优点 | 通过清晰边界降低耦合，并让复杂度、资源或并发控制可被推理。 | 性能收益是否可量化？ |
| 代价 | 会引入额外抽象、状态管理或运行时/维护成本。 | 何时不应使用？ |
| 适用场景 | 需求满足本节的约束，并且需要明确演进、资源或并发边界时。 | 如何在现有工程中渐进式落地？ |

### 面试官追问链

1. **基础：** 请定义 `MySQL客户端`，并用本节示例说明它解决的实际问题。
2. **机制：** 说明用户态/内核态边界、资源边界和可观测性；关键对象、调用顺序或数据流是什么？
3. **深入：** 与相近系统接口或工具相比，运行边界、可移植性、性能、错误模型有什么取舍？
4. **工程：** 出现异常、资源不足、并发竞争或输入非法时如何保证正确性？
5. **优化：** 在基准数据明确后，如何定位瓶颈并以复杂度、延迟或内存指标验证优化？

### 横向对比

| 对比项 | `MySQL客户端` | 相近系统接口或工具 | 选择建议 |
| --- | --- | --- | --- |
| 核心目标 | 系统边界、工具链或客户端接口相关的工程机制 | 解决相邻但边界不同的问题 | 先按问题边界选择，不按名词选择。 |
| 主要成本 | 抽象、状态与维护成本 | 成本模型不同，可能偏向性能或灵活性 | 用实际负载和团队维护能力评估。 |
| 适用条件 | 需要说明用户态/内核态边界、资源边界和可观测性 | 需要不同的数据流、生命周期或调用方式 | 不能同时满足时，优先保证正确性与可观测性。 |

### 容易忽略的知识点

- 工程上必须检查返回值、编码与资源释放；不要把工具使用经验当作底层机制。
- 面试回答要区分**语言/库的保证**与**当前实现的经验行为**，避免把未定义行为或实现细节当作标准。
- 先说明前置条件和失败路径；“能跑”不是工程正确性，资源回收、日志和测试同样是答案的一部分。

### 代码注释提示

```cpp
// 面试考点：这里说明 `MySQL客户端` 的职责边界，而不是重复代码字面含义。
// 所有权/并发：明确谁创建、谁释放资源；共享状态必须说明同步策略。
// 复杂度：标注关键路径的时间和空间复杂度，说明优化前提。
// 扩展性：通过稳定接口隔离变化，避免让调用方依赖具体实现。
```

### 可能的面试题

- **可能的面试题：** “请结合 `MySQL客户端` 说明设计/实现选择，并给出一个失败场景。”
- **简要答案：** 先解释调用链和数据流，再说明失败处理、资源释放与诊断手段。
