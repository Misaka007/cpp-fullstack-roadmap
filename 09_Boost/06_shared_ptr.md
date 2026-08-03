- [一、shared_ptr中的px出现原因](#一shared_ptr中的px出现原因)
- [二、解决析构函数](#二解决析构函数)
- [三、拷贝构造和赋值语句](#三拷贝构造和赋值语句)
- [四、shared_ptr的模拟部分](#四shared_ptr的模拟部分)
- [五、删除器](#五删除器)

## 一、shared_ptr中的px出现原因

方便对其数据空间的管理，取值和获取地址将极大的方便我们的操作。

## 二、解决析构函数

避免内存空间的泄漏。new出来的空间都没有释放掉！

释放拥有全靠的是引用计数。

```cpp
~shared_count(){ 
    if(pi){  //判断所指父类是否为空
        pi->release(); //释放new出来的对象和外部new出来的空间
    }
}
////////////////////////////////////////////////////////////////////////
public:
    virtual void dispose() = 0; //纯虚函数
    void release(){  //在sp_counted_base中
        if(--use_count_ == 0){ //判断use_count是否为0
            dispose();  //因为虚函数，所以子类中实现
            delete this; //先调用析构函数，在释放this指向的空间
        }    
    }
///////////////////////////////////////////////////////////////////////
public:
    void dispose(){
        delete px_; //释放外部new出来的空间
    }
```

因为要级联释放空间，所以sp_counted_base的析构函数必须是虚函数，才能先调用子类的析构，最后调用自己的析构函数。

> **文本化验证：** 执行示例后应核对返回值、输出顺序和资源状态；同时覆盖空输入、边界输入与失败路径。

</br>

use_count和unique函数的实现比较简单

## 三、拷贝构造和赋值语句

此时应当相当于浅拷贝，use_count加1即可！模型如下：

```text
p1 ----+
       +--> 控制块 { use_count: 1 -> 2 } --> object
p2 ----+
```

此时应在shared_ptr和shared_count进行浅拷贝，并在shared_count中加入方法。

```cpp
    shared_count(shared_count const &r) : pi(r.pi){
        if(pi){
            pi->add_ref_copy(); //在父类中实现这个方法，只要让++use_count_即可！
        }
    }
```

赋值操作的关键是通过 `swap` 交换控制块，再由临时对象析构完成旧资源的引用计数调整。

**这个赋值语句写的真的很好，既让use_count_加1，又可以让原先的空间符合情况的释放。**

```cpp
    shared_ptr<T>& operator=(shared_ptr<T> const &r){
        if(this != &r){
            this_type(r).swap(*this);//调用拷贝构造，先创建一个无名临时的对象，
        }                         //因为调用了拷贝构造，所以在shared_count中调用方法,
        return *this;             //会让use_count_加1的。
    }
//////////////////////////////////////////////////////////////////////////////////////
    void swap(shared_ptr<T> &other){
        std::swap(px, other.px); //指针的交换
        pn.swap(other.pn);
    }
```

## 四、shared_ptr的模拟部分

```cpp
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include<iostream>
using namespace std;

//#define DISPLAY

#endif
////////////////////////////////////////////////////////////////////////////////////////
#ifndef _SHARED_PTR_H_
#define _SHARED_PTR_H_

#include"shared_count.h"

template<class T>
class shared_ptr{
    typedef shared_ptr<T> this_type;
public:
    shared_ptr(T *p = 0) : px(p), pn(p){
#ifdef DISPLAY
        cout<<"Create shared_ptr object!"<<endl;
#endif
    }
    shared_ptr(shared_ptr<T> const &r) : px(r.px), pn(r.pn){}
    shared_ptr<T>& operator=(shared_ptr<T> const &r){
        if(this != &r){
            this_type(r).swap(*this);//调用拷贝构造，先创建一个无名临时的对象
        }
        return *this;
    }
    ~shared_ptr(){
#ifdef DISPLAY
        cout<<"Free shared_ptr object"<<endl;
#endif
    }
public:
    T& operator*()const{
        return *(get());
    }
    T* operator->()const{
        return get();
    }
    T* get()const{
        return px;
    }
public:
    long use_count()const{
        return pn.use_count();
    }
    bool unique()const{
        return pn.unique();
    }
    void reset(T *p){
        this_type(p).swap(*this);
    }
    void swap(shared_ptr<T> &other){
        std::swap(px, other.px); //指针的交换
        pn.swap(other.pn);
    }
private:
    T *px;
    shared_count pn;
};

#endif
////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _SHARED_COUNT_H_
#define _SHARED_COUNT_H_

#include"config.h"
#include"sp_counted_base.h"
#include"sp_counted_impl_xx.h"

class shared_count{
public:
    template<class T>  //此时类型不定，写模板函数
        shared_count(T *p) : pi(new sp_counted_impl_xx<T>(p)){
#ifdef DISPLAY
        cout<<"Create shared_cout object!"<<endl;
#endif
    }
    shared_count(shared_count const &r) : pi(r.pi){
        if(pi){
            pi->add_ref_copy();
        }
    }
    ~shared_count(){
#ifdef DISPLAY
        cout<<"Free shared_count object"<<endl;
#endif
        if(pi){
            pi->release();
        }
    }
public:
    long use_count()const{
        return pi != 0 ? pi->use_count() : 0;
    }
    bool unique()const{
        return use_count() == 1;
    }
    void swap(shared_count &r){
        sp_counted_base *tmp = r.pi;
        r.pi = pi;
        pi = tmp;
    }
private:
    sp_counted_base *pi;
};

#endif
//////////////////////////////////////////////////////////////////////////////
#ifndef SP_COUNTED_BASE_H_
#define SP_COUNTED_BASE_H_

#include"config.h"

class sp_counted_base{  //抽象类
public:
    sp_counted_base() : use_count_(1){
#ifdef DISPLAY
        cout<<"Create sp_counted_base object"<<endl;
#endif
    }
    virtual ~sp_counted_base(){
#ifdef DISPLAY
        cout<<"Free sp_counted_base object"<<endl;
#endif
    }
public:
    virtual void dispose() = 0; //纯虚函数
    void release(){
        if(--use_count_ == 0){
            dispose();
            delete this;
        }    
    }
public:
    long use_count()const{
        return use_count_;
    }
    void add_ref_copy(){
        ++use_count_;
    }
private:
    long use_count_;
};

#endif
/////////////////////////////////////////////////////////////////////////////////////
#ifndef SP_COUNTED_IMPL_XX_H_
#define SP_COUNTED_IMPL_XX_H_

#include"sp_counted_base.h"

template<class T>
class sp_counted_impl_xx : public sp_counted_base{
public:
    sp_counted_impl_xx(T *p) : px_(p){
#ifdef DISPLAY
        cout<<"Create sp_counted_impl_xx object"<<endl;
#endif
    }
    ~sp_counted_impl_xx(){
#ifdef DISPLAY
        cout<<"Free sp_counted_impl_xx object"<<endl;
#endif
    }
public:
    void dispose(){
        delete px_;
    }
private:
    T *px_;
};

#endif
////////////////////////////////////////////////////////////////////////////////////
#include<iostream>
#include"shared_ptr.h"
using namespace std;

int main(void){
    int *p = new int(10);
    shared_ptr<int> ps(p);

    cout<<ps.use_count()<<endl;
    cout<<ps.unique()<<endl;

    shared_ptr<int> ps1 = ps;
    cout<<ps.use_count()<<endl;
    cout<<ps.unique()<<endl;
    shared_ptr<int> ps2;
    ps2 = ps;
    cout<<ps.use_count()<<endl;
    cout<<ps.unique()<<endl;

    //cout<<*ps<<endl;
    
}

```

以上就是对shared_ptr的部分源码剖析的理解了。

## 五、删除器

删除器d可以是一个函数对象(是一个对象，但是使用起来像函数)，也可以是一个函数指针；

可以根据自己定义的方式去管理(释放)内存空间。有2个特性：函数对象 operator()进行了重载。

删除器的使用，调用系统的：

```cpp
#include<iostream>
#include<boost/smart_ptr.hpp>
using namespace std;
using namespace boost;

void My_Deleter(int *p){ //删除器
    cout<<"HaHa:"<<endl;
    delete p;
}
//靠删除器来管理空间，而不再向之前的调用析构函数。
int main(void){
    int *p = new int(10); //假设p是特殊的资源
    shared_ptr<int> ps(p, My_Deleter);
}
```

回过头来，对自己的空间进行释放，定义自定义的删除器。不采用默认方式释放，而是采用自己的方式释放！

删除器自己模拟部分代码：

```cpp
public:
    template<class Y, class D>
        shared_ptr(Y *p, D d) : px(p), pn(p, d){}//支持传递删除器
/////////////////////////////////////////////////////////////////////////////
    template<class Y, class D>
    shared_count(Y *p, D d) : pi(0){
        typedef Y* P;
        pi = new sp_counted_impl_pd<P, D>(p, d);
    }
///////////////////////////////////////////////////////////////////////////
template<class P, class D>
class sp_counted_impl_pd : public sp_counted_base{
public:
    sp_counted_impl_pd(P p, D d) : ptr(p), del(d){}
public:
    void dispose(){
        delete ptr;
    }
private:
    P ptr;
    D del;
};
//////////////////////////////////////////////////////////////////////////
#include<iostream>
#include"shared_ptr.h"
using namespace std;


void My_Deleter(int *p){ //删除器
    cout<<"HaHa:"<<endl;
    delete p;
}

int main(void){
    int *p = new int(10); 
    shared_ptr<int> ps(p, My_Deleter);
}

```

以上就是删除器实现的主要代码，是在shared_ptr中实现的。





## 补充：`shared_ptr` 控制块与性能边界

```text
shared_ptr p ---+
                +--> control block { strong_count, weak_count, deleter } --> object
weak_ptr w -----+
```

- 控制块通常独立分配；`make_shared` 常将对象和控制块合并分配，减少一次分配，但对象销毁后控制块会因 `weak_ptr` 存在而继续占用该块内存。
- 引用计数操作通常是原子的，但被管理对象本身的读写并不自动线程安全；仍需锁或其他同步策略。
- 不要从同一裸指针构造多个独立 `shared_ptr`，否则会发生重复删除；通过 `enable_shared_from_this` 获取共享所有权。
- 循环引用使用 `weak_ptr` 打破；API 优先按值传递 `shared_ptr` 表示共享所有权，按引用或裸指针/引用表示非拥有访问。

## 面试辅导

### 面试考点全景图

| 维度 | 回答要点 | 面试高频追问 |
| --- | --- | --- |
| 核心定义 | `shared_ptr` 属于Boost 与资源管理，重点是智能指针和资源包装器提供的所有权表达。 | 它解决了什么问题，边界在哪里？ |
| 关键角色 | 所有者、控制块、观察者、删除器。 | 各角色的职责、依赖方向和生命周期是什么？ |
| 优点 | 通过清晰边界降低耦合，并让复杂度、资源或并发控制可被推理。 | 性能收益是否可量化？ |
| 代价 | 会引入额外抽象、状态管理或运行时/维护成本。 | 何时不应使用？ |
| 适用场景 | 需求满足本节的约束，并且需要明确演进、资源或并发边界时。 | 如何在现有工程中渐进式落地？ |

### 面试官追问链

1. **基础：** 请定义 `shared_ptr`，并用本节示例说明它解决的实际问题。
2. **机制：** 说明资源由谁释放、何时释放，以及共享所有权的代价；关键对象、调用顺序或数据流是什么？
3. **深入：** 与原始指针或其他智能指针相比，所有权模型、引用计数、数组支持和迁移成本有什么取舍？
4. **工程：** 出现异常、资源不足、并发竞争或输入非法时如何保证正确性？
5. **优化：** 在基准数据明确后，如何定位瓶颈并以复杂度、延迟或内存指标验证优化？

### 横向对比

| 对比项 | `shared_ptr` | 原始指针或其他智能指针 | 选择建议 |
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
// 面试考点：这里说明 `shared_ptr` 的职责边界，而不是重复代码字面含义。
// 所有权/并发：明确谁创建、谁释放资源；共享状态必须说明同步策略。
// 复杂度：标注关键路径的时间和空间复杂度，说明优化前提。
// 扩展性：通过稳定接口隔离变化，避免让调用方依赖具体实现。
```

### 可能的面试题

- **可能的面试题：** “请结合 `shared_ptr` 说明设计/实现选择，并给出一个失败场景。”
- **简要答案：** 先确定唯一/共享/非拥有语义，再解释控制块、循环引用和异常安全。
