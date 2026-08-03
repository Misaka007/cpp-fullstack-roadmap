- [一、weak_ptr](#一weak_ptr)
- [二、使用weak_ptr](#二使用weak_ptr)
- [三、weak_ptr源码剖析](#三weak_ptr源码剖析)
- [四、intrusive_ptr的使用](#四intrusive_ptr的使用)

## 一、weak_ptr

1. **weak_ptr是为了配合shared_ptr而引入的智能指针**，它更像是shared_ptr的一个助手，它不具有普通指针的行为，没有重载operator*和->，它的最大作用在于协助shared_ptr工作，像旁观者那样观测资源的使用情况。
2. 2个重要接口：bool expired()const ；// 判断是否过期
lock()函数是弱指针的核心；
3. **获得资源的观测权，但weak_ptr没有共享资源，它的构造不会引起引用计数的增加，它的析构也不会导致引用计数减少，它只是一个静静的观察者。**

## 二、使用weak_ptr

调用系统库的：

```cpp
#include<iostream>
#include<boost/smart_ptr.hpp>
using namespace std;
using namespace boost;

int main(void){
    int *p = new int(10);
    shared_ptr<int> sp(p);
    
    weak_ptr<int> wp(sp);//1、本身对wp的创建与析构是不会增加/减少use_count;
    cout<<wp.use_count()<<endl;//2、显示的是它所观测对象的引用计数器

    if(!wp.expired()){  //没过期(use_count != 0,空间还没释放)
        shared_ptr<int> sp1 = wp.lock();
        //3、wp的lock()函数在有效的前提下可以构造对象(构造的是所观测的对象)。
        //使use_count加1；
    }//失效的话，构建的是一个空对象，引用计数将不会在增加！！！

}
```

## 三、weak_ptr源码剖析

结合shared_ptr和前面的所有，删除器，shared_array、weak_ptr给出模拟的部分代码：

模仿源代码，写出了最重要的函数：

```cpp
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include<iostream>
using namespace std;

//#define DISPLAY

#endif
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef _SHARED_PTR_H_
#define _SHARED_PTR_H_

#include"shared_count.h"

template<class T>
class weak_ptr;

template<class T>
class shared_ptr{
    friend class weak_ptr<T>;
    typedef shared_ptr<T> this_type;
public:
    template<class Y, class D>
        shared_ptr(Y *p, D d) : px(p), pn(p, d){}//支持传递删除器
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
public:
    template<class Y>
    shared_ptr(weak_ptr<Y> const &r) : pn(r.pn){
        px = r.px;
    }

private:
    T *px;
    shared_count pn;
};

#endif
/////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _SHARED_COUNT_H_
#define _SHARED_COUNT_H_

#include"config.h"
#include"sp_counted_base.h"
#include"sp_counted_impl_xx.h"


class shared_count{
    friend class weak_count;
public:
    template<class T>  //此时类型不定，写模板函数
        shared_count(T *p) : pi(new sp_counted_impl_xx<T>(p)){
#ifdef DISPLAY
        cout<<"Create shared_cout object!"<<endl;
#endif
    }
    template<class Y, class D>
    shared_count(Y *p, D d) : pi(0){
        typedef Y* P;
        pi = new sp_counted_impl_pd<P, D>(p, d);
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
public:
    explicit shared_count(weak_count const &r);
private:
    sp_counted_base *pi;
};
/////////////////////////////////////////////////////
template<class P, class D>
class sp_counted_impl_pd : public sp_counted_base{
public:
    sp_counted_impl_pd(P p, D d) : ptr(p), del(d){}
public:
    void dispose(){
        del(ptr);
    }
private:
    P ptr;
    D del;
};
/////////////////////////////////////////////////////////////////////
class weak_count{
    friend class shared_count;
public:
    weak_count(shared_count const &r) : pi(r.pi){
        if(pi != 0){
            pi->weak_add_ref();
        }
    }
    ~weak_count(){
        if(pi){
            pi->weak_release();
        }
    }
public:
    long use_count()const{
        return pi != 0 ? pi->use_count() : 0;
    }
private:
    sp_counted_base *pi;
};

shared_count::shared_count(weak_count const &r) : pi(r.pi){
    if(pi){
        pi->add_ref_lock();
    }
}
#endif
//////////////////////////////////////////////////////////////////////////////
#ifndef SP_COUNTED_BASE_H_
#define SP_COUNTED_BASE_H_

#include"config.h"

class sp_counted_base{  //抽象类
public:
    sp_counted_base() : use_count_(1), weak_count_(1){
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
    void weak_add_ref(){
        ++weak_count_;
    }
    virtual void destroy(){
        delete this;
    }
    void weak_release(){
        if(--weak_count_ == 0){
            destroy();
        }
    }
    bool add_ref_lock(){
        if(use_count == 0){
            return false;
        }
        ++use_count_;
        return true;
    }
private:
    long use_count_;
    long weak_count_;
};

#endif
/////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _WEAK_PTR_H_
#define _WEAK_PTR_H_

#include"shared_ptr.h"

template<class T>
class shared_ptr;

class shared_count;
template<class T>
class weak_ptr{
    friend class shared_ptr<T>;
    friend class shared_count;
public:
    template<class Y>
    weak_ptr(shared_ptr<Y> const &r) : px(r.px), pn(r.pn){}
    ~weak_ptr(){}
public:
    long use_count()const{
        pn.use_count();
    }
    bool expired()const{
        return pn.use_count() == 0;
    }
    shared_ptr<T> lock()const{
        return shared_ptr<T>(*this);
    }
private:
    T *px;
    weak_count pn;
};

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////////
#include<iostream>
#include"shared_ptr.h"
#include"shared_array.h"
#include"weak_ptr.h"
using namespace std;

int main(void){
    int *p = new int(10);

    shared_ptr<int> sp(p);
    weak_ptr<int> wp(sp);
    if(!wp.expired()){
        shared_ptr<int> sp1 = wp.lock();//返回真实的对象
        cout<<sp.use_count()<<endl;
    }
    cout<<sp.use_count()<<endl;

}


```

这是这个智能指针的最主要的剖析，关键理清脉络，条理清晰一些，就可以分析出来！

## 四、intrusive_ptr的使用

适用情景：

- 对内存的占用要求非常严格，不能有引用计数的内存开销；
- **这时的代码中已经有了引用计数机制管理的对象；**

重点掌握如何使用：

- 两个方法必须重写(增加/减少引用计数)
- 必须自己编写管理引用计数的类
- 要封装到一个类中，在继承(把自己的类侵入到智能指针中进行管理)。

具体使用的一个类子如下：

```cpp
#include<iostream>
#include<boost/smart_ptr.hpp>
using namespace std;
using namespace boost;
 
//1、实现2个重要的函数
template<class T>
void intrusive_ptr_add_ref(T *t){ //增加引用计数
    t->add_ref_copy();
}
template<class T>
void intrusive_ptr_release(T *t){  //减少引用计数
    t->release();
}

//2、提供管理对象
class Counter{
public:
    Counter() : use_count_(0){
    
    }
    ~Counter(){
    
    }
public:
    void add_ref_copy(){
        ++use_count_;
    }
    void release(){
        if(--use_count_ == 0){
            delete this;
        }
    }
private:
    long use_count_;
};
//3、定义自己的对象
class Test;
ostream& operator<<(ostream &out, const Test &t);
class Test : public Counter{
    friend     ostream& operator<<(ostream &out, const Test &t);
public:
    Test(int d = 0) : data(d){}
    ~Test(){}
private:
    int data;
};

ostream& operator<<(ostream &out, const Test &t){
    out<<t.data;
    return out;
}

int main(void){
    Test *pt = new Test(10);
    intrusive_ptr<Test> ip(pt);
    cout<<*ip<<endl;

}
```

> **文本化验证：** 执行示例后应核对返回值、输出顺序和资源状态；同时覆盖空输入、边界输入与失败路径。

## 补充：观察所有权与侵入式计数

```text
shared_ptr ----> 控制块 ----> object
weak_ptr   ----^        \-> weak_count

intrusive_ptr ----> object { 内嵌引用计数 }
```

- `weak_ptr` 用于观察 `shared_ptr` 管理的对象；使用前通过 `lock()` 获取临时 `shared_ptr`，避免 check-then-use 竞态。
- 循环引用中至少一条边必须改为 `weak_ptr`，否则控制块计数永远无法降到零。
- `intrusive_ptr` 将引用计数放入对象，减少独立控制块分配，但要求类型可修改且生命周期协议由类型实现。

## 面试辅导

### 面试考点全景图

| 维度 | 回答要点 | 面试高频追问 |
| --- | --- | --- |
| 核心定义 | `weak_ptr与intrusive_ptr` 属于Boost 与资源管理，重点是智能指针和资源包装器提供的所有权表达。 | 它解决了什么问题，边界在哪里？ |
| 关键角色 | 所有者、控制块、观察者、删除器。 | 各角色的职责、依赖方向和生命周期是什么？ |
| 优点 | 通过清晰边界降低耦合，并让复杂度、资源或并发控制可被推理。 | 性能收益是否可量化？ |
| 代价 | 会引入额外抽象、状态管理或运行时/维护成本。 | 何时不应使用？ |
| 适用场景 | 需求满足本节的约束，并且需要明确演进、资源或并发边界时。 | 如何在现有工程中渐进式落地？ |

### 面试官追问链

1. **基础：** 请定义 `weak_ptr与intrusive_ptr`，并用本节示例说明它解决的实际问题。
2. **机制：** 说明资源由谁释放、何时释放，以及共享所有权的代价；关键对象、调用顺序或数据流是什么？
3. **深入：** 与原始指针或其他智能指针相比，所有权模型、引用计数、数组支持和迁移成本有什么取舍？
4. **工程：** 出现异常、资源不足、并发竞争或输入非法时如何保证正确性？
5. **优化：** 在基准数据明确后，如何定位瓶颈并以复杂度、延迟或内存指标验证优化？

### 横向对比

| 对比项 | `weak_ptr与intrusive_ptr` | 原始指针或其他智能指针 | 选择建议 |
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
// 面试考点：这里说明 `weak_ptr与intrusive_ptr` 的职责边界，而不是重复代码字面含义。
// 所有权/并发：明确谁创建、谁释放资源；共享状态必须说明同步策略。
// 复杂度：标注关键路径的时间和空间复杂度，说明优化前提。
// 扩展性：通过稳定接口隔离变化，避免让调用方依赖具体实现。
```

### 可能的面试题

- **可能的面试题：** “请结合 `weak_ptr与intrusive_ptr` 说明设计/实现选择，并给出一个失败场景。”
- **简要答案：** 先确定唯一/共享/非拥有语义，再解释控制块、循环引用和异常安全。
