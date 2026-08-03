- [一、共享性智能指针(shared_ptr)](#一共享性智能指针shared_ptr)
- [二、怎么使用shared_ptr](#二怎么使用shared_ptr)
- [三、框架的搭建](#三框架的搭建)

## 一、共享性智能指针(shared_ptr)

引用计数型指针：shared_ptr是一个最像指针的“智能指针”，是boost.smart_ptr库中最有价值，最重要，也是最有用的。

**shared_ptr实现的是引用技术型的智能指针，可以被拷贝和赋值，在任意地方共享它，当没有代码使用(此时引用计数为0)它才删除被动态分配的对象。shared_ptr也可以被安全的放到标准容器中；**

## 二、怎么使用shared_ptr

举一个操作的例子：

```cpp
#include<iostream>
#include<boost/smart_ptr.hpp>
using namespace std;
using namespace boost;

int main(void){
    int *p = new int(10);
    shared_ptr<int> ps(p);
//    cout<<*ps<<endl;

    cout<<ps.unique()<<endl; //判断对空间是否唯一，
    cout<<ps.use_count()<<endl;
    shared_ptr<int> ps1 = ps;
    cout<<ps.unique()<<endl; //此时有两个对空间有共享，所以不唯一，是0
    cout<<ps.use_count()<<endl;
    shared_ptr<int> ps2;
    ps2 = ps1;
    cout<<ps.use_count()<<endl;
}
```

关键在shared_ptr中存在共享引用计数。

## 三、框架的搭建

阅读源代码：

shared_ptr 中的私有数据成员：

```cpp
private:
    T *px;
    shared_count pn; //对象成员，肯定先调这个对象的构造函数；
```

**之前的引用计数通过一个指针，现在的引用计数通过一个对象，pn
构造函数的调用顺序：先虚基类，父类，对象成员，最后构造自己；**

此时的模型如下：

```text
shared_ptr p1 --+
                +--> 控制块 { use_count = 2 } --> object
shared_ptr p2 --+
```

其后调用对象成员的构造函数：

shared_counted中的私有数据成员：

```cpp
private:
    sp_counted_base *pi; //有一个指向引用计数器父类的指针；
```

此时就得先写：sp_counted_base类了；

sp_counted_base类中的私有数据成员：

```cpp
private:
    long use_count_;
```

然后看到在shared_counted的构造函数：

```cpp
public:
    template<class T>  //此时类型不定，写模板函数
        shared_count(T *p) : pi(new sp_counted_impl_xx(p)){ //特别重要，这个构造函数
```

此时就得写sp_counted_impl_xx类了：这是继承sp_counted_base类

其内部数据时成员：

```cpp
private:
    T *px_;
```

此时整体的建构体系就已经形成：

其内存关系如下：

1. 先实现了shared_ptr类，因为有对象成员，其后调用构造函数；
2. 实现了shared_count; 其数据成员有sp_counted_base；
3. 因为编译器的顺序，先类名，在数据成员，最后函数，所以此时先实现sp_counted_base;
4. 因为shared_counted中的构造函数要在堆上开辟sp_counted_impl_xx空间，最后实现是sp_counted_impl_xx，它有继承sp_counted_base,所以构造函数的调用顺序就很清楚了。

构造函数的调用顺序：sp_counted_base、sp_counted_impl_xx、shared_count、shared_ptr。

此时的具体实现代码如下：

```cpp
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include<iostream>
using namespace std;

#endif
////////////////////////////////////////////////////////////////////////////
#ifndef _SHARED_PTR_H_
#define _SHARED_PTR_H_

#include"shared_count.h"

template<class T>
class shared_ptr{
public:
    shared_ptr(T *p = 0) : px(p), pn(p){
        cout<<"Create shared_ptr object!"<<endl;
    }
    ~shared_ptr(){
        cout<<"Free shared_ptr object"<<endl;
    }
private:
    T *px;
    shared_count pn;
};

#endif
///////////////////////////////////////////////////////////////////////////////
#ifndef _SHARED_COUNT_H_
#define _SHARED_COUNT_H_

#include"config.h"
#include"sp_counted_base.h"
#include"sp_counted_impl_xx.h"

class shared_count{
public:
    template<class T>  //此时类型不定，写模板函数
        shared_count(T *p) : pi(new sp_counted_impl_xx<T>(p)){
        cout<<"Create shared_cout object!"<<endl;
    }
    ~shared_count(){
        cout<<"Free shared_count object"<<endl;
    }
private:
    sp_counted_base *pi;
};


#endif
///////////////////////////////////////////////////////////////////////////////
#ifndef SP_COUNTED_BASE_H_
#define SP_COUNTED_BASE_H_

#include"config.h"

class sp_counted_base{
public:
    sp_counted_base() : use_count_(1){
        cout<<"Create sp_counted_base object"<<endl;
    }
    ~sp_counted_base(){
        cout<<"Free sp_counted_base object"<<endl;
    }
private:
    long use_count_;
};

#endif
//////////////////////////////////////////////////////////////////////////////////////
#ifndef SP_COUNTED_IMPL_XX_H_
#define SP_COUNTED_IMPL_XX_H_

#include"sp_counted_base.h"

template<class T>
class sp_counted_impl_xx : public sp_counted_base{
public:
    sp_counted_impl_xx(T *p) : px_(p){
        cout<<"Create sp_counted_impl_xx object"<<endl;
    }
    ~sp_counted_impl_xx(){
        cout<<"Free sp_counted_impl_xx object"<<endl;
    }
private:
    T *px_;
};

#endif
//////////////////////////////////////////////////////////////////////////////////////////////
#include<iostream>
#include"shared_ptr.h"
using namespace std;

int main(void){
    int *p = new int(10);
    shared_ptr<int> ps(p);   
}

```

以上就是只搭好了大致的框架，并没有考虑内存泄漏、析构的具体写法和其它函数的实现；

那么整个模型如下：

```text
shared_ptr --> 控制块 --> object
weak_ptr   -----^       （不增加 use_count）
```





## 面试辅导

### 面试考点全景图

| 维度 | 回答要点 | 面试高频追问 |
| --- | --- | --- |
| 核心定义 | `shared_ptr_上` 属于Boost 与资源管理，重点是智能指针和资源包装器提供的所有权表达。 | 它解决了什么问题，边界在哪里？ |
| 关键角色 | 所有者、控制块、观察者、删除器。 | 各角色的职责、依赖方向和生命周期是什么？ |
| 优点 | 通过清晰边界降低耦合，并让复杂度、资源或并发控制可被推理。 | 性能收益是否可量化？ |
| 代价 | 会引入额外抽象、状态管理或运行时/维护成本。 | 何时不应使用？ |
| 适用场景 | 需求满足本节的约束，并且需要明确演进、资源或并发边界时。 | 如何在现有工程中渐进式落地？ |

### 面试官追问链

1. **基础：** 请定义 `shared_ptr_上`，并用本节示例说明它解决的实际问题。
2. **机制：** 说明资源由谁释放、何时释放，以及共享所有权的代价；关键对象、调用顺序或数据流是什么？
3. **深入：** 与原始指针或其他智能指针相比，所有权模型、引用计数、数组支持和迁移成本有什么取舍？
4. **工程：** 出现异常、资源不足、并发竞争或输入非法时如何保证正确性？
5. **优化：** 在基准数据明确后，如何定位瓶颈并以复杂度、延迟或内存指标验证优化？

### 横向对比

| 对比项 | `shared_ptr_上` | 原始指针或其他智能指针 | 选择建议 |
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
// 面试考点：这里说明 `shared_ptr_上` 的职责边界，而不是重复代码字面含义。
// 所有权/并发：明确谁创建、谁释放资源；共享状态必须说明同步策略。
// 复杂度：标注关键路径的时间和空间复杂度，说明优化前提。
// 扩展性：通过稳定接口隔离变化，避免让调用方依赖具体实现。
```

### 可能的面试题

- **可能的面试题：** “请结合 `shared_ptr_上` 说明设计/实现选择，并给出一个失败场景。”
- **简要答案：** 先确定唯一/共享/非拥有语义，再解释控制块、循环引用和异常安全。
