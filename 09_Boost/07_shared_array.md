- [一、上次写的删除器有些问题](#一上次写的删除器有些问题)
- [二、shared_array](#二shared_array)
- [三、如何使用shared_array](#三如何使用shared_array)
- [四、shared_array](#四shared_array)

## 一、上次写的删除器有些问题

```cpp
template<class P, class D>
class sp_counted_impl_pd : public sp_counted_base{
public:
    sp_counted_impl_pd(P p, D d) : ptr(p), del(d){}
public:
    void dispose(){
        del(ptr);  //就是这里，将对象用作函数！！！
    }
private:
    P ptr;
    D del;
};
```

**del(ptr)  -> del.operator()(ptr);重载了()的类使用起来就是函数对象。**

**删除器：函数对象和函数都可以充当。**

## 二、shared_array

它和shared_ptr类似，它包装了new[]操作符在堆上分配的动态数组，也是采用了引用计数的机制。

shared_array的接口和功能与shared_ptr几乎是相同的，主要区别：

1. 接受指针p必须是new []的结果
2. 提供operator[]的重载，可以使用下标
3. 系统没有提供*、->的重载
4. 析构函数使用delete  [];

## 三、如何使用shared_array

系统调用：

```cpp
#include<iostream>
#include<boost/smart_ptr.hpp>
using namespace std;
using namespace boost;

int main(void){
    int *p = new int[10];
    shared_array<int> pa(p);  //共享数组

    for(int i = 0; i < 10; i++){
        pa[i] = i+1;  //系统内进行了[]的重载
    }
    for(i = 0; i < 10; i++){
        cout<<pa[i]<<" ";
    }
    cout<<endl;

}
```

## 四、shared_array

模仿的源码如下：

```cpp
#ifndef _SHARED_ARRAY_H_
#define _SHARED_ARRAY_H_

#include"checked_delete.h"

template<class T>
class shared_array{
public:
    typedef checked_array_deleter<T> deleter;
    shared_array(T *p = 0) : px(p), pn(p, deleter()){} //无名对象
    ~shared_array(){
        
    }
public:
    T& operator[](int i)const{
        return px[i];
    }
private:
    T *px;
    shared_count pn;  //必须用到引用计数器对象
};

#endif
///////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CHECKED_DELETE_H_
#define _CHECKED_DELETE_H_

template<class T>
void checked_array_delete(T *x){
    delete []x;
}

template<class T>
struct checked_array_deleter{
public:
    void operator()(T *x)const{
        checked_array_delete(x);        
    }
};

#endif
/////////////////////////////////////////////////////////////////////////////////////////////
#include<iostream>
#include"shared_ptr.h"
#include"shared_array.h"
using namespace std;
/*
template<class T>
void checked_array_delete(T *x){
    delete []x;
}

template<class T>
struct checked_array_deleter{
public:
    void operator()(T *x)const{
        checked_array_delete(x);
    }
};
写好()的重载之后，就是在shared_counted.h中释放空间时将用到。
del(ptr)  -> del.operator()(ptr);重载了()的类使用起来就是函数对象
删除器：函数对象和函数都可以充当。
*/
int main(void){
    int *p = new int[10];
    shared_array<int> pa(p);

    for(int i = 0; i < 10; i++){
        pa[i] = i+1;
    }
    for(i = 0; i < 10; i++){
        cout<<pa[i]<<" ";
    }
    cout<<endl;
}

```

缺点：

1. 重载使用[]时要小心，shared_array不提供数组的索引范围检查
2. 所管理的空间是死的，不能够自动增长。





## 面试辅导

### 面试考点全景图

| 维度 | 回答要点 | 面试高频追问 |
| --- | --- | --- |
| 核心定义 | `shared_array` 属于Boost 与资源管理，重点是智能指针和资源包装器提供的所有权表达。 | 它解决了什么问题，边界在哪里？ |
| 关键角色 | 所有者、控制块、观察者、删除器。 | 各角色的职责、依赖方向和生命周期是什么？ |
| 优点 | 通过清晰边界降低耦合，并让复杂度、资源或并发控制可被推理。 | 性能收益是否可量化？ |
| 代价 | 会引入额外抽象、状态管理或运行时/维护成本。 | 何时不应使用？ |
| 适用场景 | 需求满足本节的约束，并且需要明确演进、资源或并发边界时。 | 如何在现有工程中渐进式落地？ |

### 面试官追问链

1. **基础：** 请定义 `shared_array`，并用本节示例说明它解决的实际问题。
2. **机制：** 说明资源由谁释放、何时释放，以及共享所有权的代价；关键对象、调用顺序或数据流是什么？
3. **深入：** 与原始指针或其他智能指针相比，所有权模型、引用计数、数组支持和迁移成本有什么取舍？
4. **工程：** 出现异常、资源不足、并发竞争或输入非法时如何保证正确性？
5. **优化：** 在基准数据明确后，如何定位瓶颈并以复杂度、延迟或内存指标验证优化？

### 横向对比

| 对比项 | `shared_array` | 原始指针或其他智能指针 | 选择建议 |
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
// 面试考点：这里说明 `shared_array` 的职责边界，而不是重复代码字面含义。
// 所有权/并发：明确谁创建、谁释放资源；共享状态必须说明同步策略。
// 复杂度：标注关键路径的时间和空间复杂度，说明优化前提。
// 扩展性：通过稳定接口隔离变化，避免让调用方依赖具体实现。
```

### 可能的面试题

- **可能的面试题：** “请结合 `shared_array` 说明设计/实现选择，并给出一个失败场景。”
- **简要答案：** 先确定唯一/共享/非拥有语义，再解释控制块、循环引用和异常安全。
