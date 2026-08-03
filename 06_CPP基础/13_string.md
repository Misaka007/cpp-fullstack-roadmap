- [一、string的初始化，遍历，字符串连接](#一string的初始化遍历字符串连接)
- [二、string的查找，替换](#二string的查找替换)
- [三、区间的删除和插入](#三区间的删除和插入)
- [四、string的大小写转换-->函数指针](#四string的大小写转换--函数指针)

C++ 中 string 的学习使用！

string的初始化，在C++中字符串是一种数据类型；

## 一、string的初始化，遍历，字符串连接

```cpp
#include<iostream>
#include<string>
#include<stdio.h>
using namespace std;

int main(void){  
//string的初始化,在C++中字符串是一种数据类型;
    string s1 = "abcdefg";
    string s2("abcdefg");
    string s3(s2);
    string s4 = s1;  //调用拷贝构造函数;
    string s5(10, 'a');//10个空间中的字符都是'a';
    s5 = s1; 

    cout<<"s3:"<<s3<<endl;
    cout<<"s5:"<<s5<<endl;

//string的遍历,重载了[]操作符;
    //1、数组方式遍历[]
    for(int i = 0; i < s1.length(); i++){
        cout<<s1[i]<<" ";  //出现错误(下标越界),不会向外面剖出异常,引起程序的中断;
    }   
    cout<<endl;
    //2、迭代器
    string::iterator it; 
    for(it = s1.begin(); it != s1.end(); it++){
        cout<<*it<<" ";
    }
    cout<<endl;
    //3、函数at()遍历
    for(int i = 0; i < s1.length(); i++){
        cout<<s1.at(i)<<" "; //会剖出异常,合理的解决下标越界;
    }
    cout<<endl;

//字符指针和string的转换
    //此时,把s1====>char * 把内存首地址给露出来;
    printf("s1:%s \n", s1.c_str());

    //s1中的内容拷贝到buf中;
    char buf[123] = {0};
    s1.copy(buf, 2, 0);//n, pos;下标从0开始拷贝2个字符到buf中,不会是C风格的,注意自己加上0结束标志;
    cout<<buf<<endl;

//string子符串的连接
    s1 = s1 + s2; //直接+就表：字符串的连接;
    s1 += s2; //+=也是字符串的连接;

    s1.append(s4); //调用方法append()也是字符串的连接;

    cout<<s1<<endl;       

    return 0;
}
```

> **文本化验证：** 执行示例后应核对返回值、输出顺序和资源状态；同时覆盖空输入、边界输入与失败路径。

## 二、string的查找，替换

```cpp
#include<iostream>
#include<string>
#include<string.h>
using namespace std;

int main(void){
//字符串的查找和替换
    string s1 = "wbm hello wbm 111 wbm 222 wbm 333";

    //1、第一次出现wbm的下标
    int index = s1.find("wbm", 0); 
    cout<<"index :"<<index<<endl;

    //2、求wbm每一次出现的数组下标
    
/*  int offindex = s1.find("wbm", 0);
    while(offindex != -1){
        cout<<"offindex :"<<offindex<<endl;
        offindex += strlen("wbm");
        offindex = s1.find("wbm", offindex);
    }*/

    //3、把小写wbm换成大写
    int offindex = s1.find("wbm", 0); 
    while(offindex != -1){
        cout<<"offindex :"<<offindex<<endl;
        s1.replace(offindex, strlen("wbm"), "WBM"); //从下标offindex开始,删除n个字符,替换为后面的字符;
        offindex += strlen("wbm");
        offindex = s1.find("wbm", offindex);
    }
    cout<<"s1:"<<s1<<endl;

    string s3 = "aaa bbb ccc";
    s3.replace(0, 3, "AAA");  //替换的函数;
    cout<<"s3:"<<s3<<endl;

    return 0;
}

```

> **文本化验证：** 执行示例后应核对返回值、输出顺序和资源状态；同时覆盖空输入、边界输入与失败路径。

## 三、区间的删除和插入

```cpp
#include<iostream>
#include<string>
#include<algorithm>
using namespace std;

int main(void){
//区间删除和插入
    string s1 = "hello1 hello2 hell03";
    string::iterator it = find(s1.begin(), s1.end(), 'l');
    if(it != s1.end()){
        s1.erase(it); //删除算法;
    }   
    cout<<"s1 :"<<s1<<endl;

    s1.erase(s1.begin(), s1.end()); //删除从pos开始的n个字符;
    cout<<"s1全部删除:"<<s1<<endl;
    cout<<"s1的长度:"<<s1.length()<<endl;

    string s2 = "BBB";
    s2.insert(0, "AAA");  //头插法
    s2.insert(s2.length(), "CCC");//尾插法
    cout<<s2<<endl;


    return 0;
}
```

> **文本化验证：** 执行示例后应核对返回值、输出顺序和资源状态；同时覆盖空输入、边界输入与失败路径。

## 四、string的大小写转换-->函数指针

```cpp
#include<iostream>
#include<string>
#include<algorithm>
using namespace std;

int main(void){
    string s1 = "AAAbbb";

    transform(s1.begin(), s1.end(), s1.begin(), 0, toupper);//toupper可以是：函数的入口地址,函数对象,
    cout<<s1<<endl;

    string s2 = "AAAbbb";
    transform(s2.begin(), s2.end(), s2.begin(), 0, tolower);
    cout<<s2<<endl;

    return 0;
}
```





## 补充：`std::string` 的存储与接口约束

- `std::string` 连续存储字符并维护长度；`c_str()` 提供以 `\0` 结尾的只读视图，但遇到修改、扩容或对象销毁后指针可能失效。
- `size()` 是 O(1)，`append`/`+=` 的摊销复杂度通常为 O(k)；频繁拼接时可使用 `reserve` 减少重分配。
- `operator[]` 不检查边界，`at()` 越界会抛 `std::out_of_range`；二进制数据可包含 `\0`，不能只依赖 C 字符串函数。
- 不要假设短字符串优化（SSO）的具体阈值；它属于实现细节。

## 面试辅导

### 面试考点全景图

| 维度 | 回答要点 | 面试高频追问 |
| --- | --- | --- |
| 核心定义 | `string` 属于C++ 基础，重点是语言语义、对象生命周期和资源所有权。 | 它解决了什么问题，边界在哪里？ |
| 关键角色 | 引用/指针、值语义/对象语义、RAII。 | 各角色的职责、依赖方向和生命周期是什么？ |
| 优点 | 通过清晰边界降低耦合，并让复杂度、资源或并发控制可被推理。 | 性能收益是否可量化？ |
| 代价 | 会引入额外抽象、状态管理或运行时/维护成本。 | 何时不应使用？ |
| 适用场景 | 需求满足本节的约束，并且需要明确演进、资源或并发边界时。 | 如何在现有工程中渐进式落地？ |

### 面试官追问链

1. **基础：** 请定义 `string`，并用本节示例说明它解决的实际问题。
2. **机制：** 定义语义并说明编译期与运行时成本；关键对象、调用顺序或数据流是什么？
3. **深入：** 与相近语言机制相比，是否改变对象所有权、生命周期或调用绑定方式有什么取舍？
4. **工程：** 出现异常、资源不足、并发竞争或输入非法时如何保证正确性？
5. **优化：** 在基准数据明确后，如何定位瓶颈并以复杂度、延迟或内存指标验证优化？

### 横向对比

| 对比项 | `string` | 相近语言机制 | 选择建议 |
| --- | --- | --- | --- |
| 核心目标 | 语言语义、对象生命周期和资源所有权 | 解决相邻但边界不同的问题 | 先按问题边界选择，不按名词选择。 |
| 主要成本 | 抽象、状态与维护成本 | 成本模型不同，可能偏向性能或灵活性 | 用实际负载和团队维护能力评估。 |
| 适用条件 | 需要定义语义并说明编译期与运行时成本 | 需要不同的数据流、生命周期或调用方式 | 不能同时满足时，优先保证正确性与可观测性。 |

### 容易忽略的知识点

- 避免只背语法；回答时要交代所有权、别名关系、异常路径和优化是否由编译器保证。
- 面试回答要区分**语言/库的保证**与**当前实现的经验行为**，避免把未定义行为或实现细节当作标准。
- 先说明前置条件和失败路径；“能跑”不是工程正确性，资源回收、日志和测试同样是答案的一部分。

### 代码注释提示

```cpp
// 面试考点：这里说明 `string` 的职责边界，而不是重复代码字面含义。
// 所有权/并发：明确谁创建、谁释放资源；共享状态必须说明同步策略。
// 复杂度：标注关键路径的时间和空间复杂度，说明优化前提。
// 扩展性：通过稳定接口隔离变化，避免让调用方依赖具体实现。
```

### 可能的面试题

- **可能的面试题：** “请结合 `string` 说明设计/实现选择，并给出一个失败场景。”
- **简要答案：** 先解释语义，再说明对象何时构造、复制、移动和销毁，最后给出异常安全或性能取舍。
