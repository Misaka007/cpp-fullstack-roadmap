- [一、VC和VS](#一vc和vs)
- [二、Boost库](#二boost库)
- [三、scoped_ptr(局部智能指针)](#三scoped_ptr局部智能指针)
  - [3.1、如何使用](#31如何使用)
  - [3.2、理解](#32理解)
- [四、scoped_ptr的内部实现过程](#四scoped_ptr的内部实现过程)

## 一、VC和VS

VC版并不是标准C++，VS版符合标准C++，其语法相当严格。

**缺点：VC和VS都只能释放一个具体类型空间，不能对数组空间进行释放，还有写时拷贝的问题；**

所以引发了Boost库的出现来解决此类问题。

## 二、Boost库

推荐看一下Boost库完全开发指南。

Boost本身是开源库，在C++中的地位举足轻重，第三章内存管理，智能指针；

C++中也提供了智能指针，但是并不能解决所有问题。

smart_ptr库中：new delete的运用不正确，是C++中造成资源获取/释放问题的根源。

## 三、scoped_ptr(局部智能指针)

### 3.1、如何使用

**scoped_ptr是一个很类似auto_ptr的智能指针，但是scoped_ptr的所有权更加严格，不能转让，一旦scoped_ptr获取了对象的管理权，你就无法再从它那里取回来。**

平常的智能指针加上 memory 头文件，Boost库的搭建，就是拷到相应的目录下；然后编译，出错进去把该注释的都注释上。

```cpp
#include<iostream>
#include<string>
#include<boost/smart_ptr.hpp> //：在include目录下的boost目录下的smart_ptr.hpp;
using namespace std;                    //打开就是其它智能指针的声明；
using namespace boost;  //boost库必须引入的命名空间


int main(void){
    int *p = new int(10);

    scoped_ptr<int> ps(p);
    cout<<*ps<<endl;  //有了指针功能*运算符
    string *px = new string("hello");
    scoped_ptr<string> ps1(px);
    cout<<ps1->size()<<endl;//有了->，指针
}
```

### 3.2、理解

在库的引入下，确实具有智能指针的特性，必须加上。

```cpp
#include<boost/smart_ptr.hpp>  和  using namespace boost;，才具有智能指针的特性。
```

> **对比：VC和VS版的拷贝构造，赋值都没问题。**

> **scoped_ptr(局部智能指针)，对空间的管理权不能交由其它对象，不能进行拷贝构造和赋值。**

## 四、scoped_ptr的内部实现过程

**如何达到拥有权不能转移的目的，拷贝构造和赋值语句声明为私有的，不需要实现。**

模拟源码实现如下：

```cpp
#include<iostream>
using namespace std;                            

template<class T>
class scoped_ptr{
public:
    scoped_ptr(T *p = 0) : px(p){}
    ~scoped_ptr(){
        delete px;
    }
public:
    T& operator*()const{
        return *px;
    }
    T* operator->()const{
        return px;
    }
    T* get()const{
        return px;
    }/*
    void reset(T *p = 0){
        if(p != px && px){
            delete px;
        }
        px = p;
    }*/ //boost库中有一个更好的解决方案
    typedef scoped_ptr<T> this_type;
    void reset(T *p = 0){
        this_type(p).swap(*this); //无名临时对象技术
    }
    void swap(scoped_ptr &b){
        T *tmp = b.px;
        b.px = px;
        px = tmp;
    }
private: //不想让其拥有哪些功能，声明为私有即可。
    scoped_ptr(const scoped_ptr<T> &);//声明为私有，外部就无法进行拷贝构造了
    scoped_ptr& operator=(const scoped_ptr<T> &);//外部就调不动赋值语句了
    void operator==(scoped_ptr const &) const;  //对象不具有比较==
    void operator!=(scoped_ptr const &) const;  //对象不具有比较!=
    T *px;
};

class Test{
public:
    void fun(){
        cout<<"This is Test fun()"<<endl;
    }
};

int main(void){
    int *p = new int(10);

    scoped_ptr<int> ps(p);
    cout<<*ps<<endl;
    int *q = new int(20);
    ps.reset(q); //重新设置函数，将原有空间释放，重新管理一个空间

    
    scoped_ptr<Test> ps1(new Test);
    ps1->fun();
}
```

对reset()函数的理解：this_type(p).swap(*this);模型如下：

```text
临时 scoped_ptr(p) <----swap----> 当前 scoped_ptr(old)
临时对象析构后释放 old；当前对象接管 p。
```

通过新生成的无名临时变量，将新地址与旧地址交换，在最后脱离函数范围，对象消亡，调用析构函数，释放原先空间，达到不内存泄漏，并且对新空间进行管理。

缺点：不能对数组空间进行管理。





## 面试辅导

### 面试考点全景图

| 维度 | 回答要点 | 面试高频追问 |
| --- | --- | --- |
| 核心定义 | `scoped_ptr` 属于Boost 与资源管理，重点是智能指针和资源包装器提供的所有权表达。 | 它解决了什么问题，边界在哪里？ |
| 关键角色 | 所有者、控制块、观察者、删除器。 | 各角色的职责、依赖方向和生命周期是什么？ |
| 优点 | 通过清晰边界降低耦合，并让复杂度、资源或并发控制可被推理。 | 性能收益是否可量化？ |
| 代价 | 会引入额外抽象、状态管理或运行时/维护成本。 | 何时不应使用？ |
| 适用场景 | 需求满足本节的约束，并且需要明确演进、资源或并发边界时。 | 如何在现有工程中渐进式落地？ |

### 面试官追问链

1. **基础：** 请定义 `scoped_ptr`，并用本节示例说明它解决的实际问题。
2. **机制：** 说明资源由谁释放、何时释放，以及共享所有权的代价；关键对象、调用顺序或数据流是什么？
3. **深入：** 与原始指针或其他智能指针相比，所有权模型、引用计数、数组支持和迁移成本有什么取舍？
4. **工程：** 出现异常、资源不足、并发竞争或输入非法时如何保证正确性？
5. **优化：** 在基准数据明确后，如何定位瓶颈并以复杂度、延迟或内存指标验证优化？

### 横向对比

| 对比项 | `scoped_ptr` | 原始指针或其他智能指针 | 选择建议 |
| --- | --- | --- | --- |
| 核心目标 | 智能指针和资源包装器提供的所有权表达 | 解决相邻但边界不同的问题 | 先按问题边界选择，不按名词选择。 |
| 主要成本 | 抽象、状态与维护成本 | 成本模型不同，可能偏向性能或灵活性 | 用实际负载和团队维护能力评估。 |
| 适用条件 | 需要说明资源由谁释放、何时释放，以及共享所有权的代价 | 需要不同的数据流、生命周期或调用方式 | 不能同时满足时，优先保证正确性与可观测性。 |

### 容易忽略的知识点

- 原始指针不等于所有权；避免循环引用，跨 DLL/自定义删除器时必须匹配分配与释放方式。
- 面试回答要区分**语言/库的保证**与**当前实现的经验行为**，避免把未定义行为或实现细节当作标准。
- 先说明前置条件和失败路径；“能跑”不是工程正确性，资源回收、日志和测试同样是答案的一部分。

### 代码注释提示

```cpp
// 面试考点：这里说明 `scoped_ptr` 的职责边界，而不是重复代码字面含义。
// 所有权/并发：明确谁创建、谁释放资源；共享状态必须说明同步策略。
// 复杂度：标注关键路径的时间和空间复杂度，说明优化前提。
// 扩展性：通过稳定接口隔离变化，避免让调用方依赖具体实现。
```

### 可能的面试题

- **可能的面试题：** “请结合 `scoped_ptr` 说明设计/实现选择，并给出一个失败场景。”
- **简要答案：** 先确定唯一/共享/非拥有语义，再解释控制块、循环引用和异常安全。
