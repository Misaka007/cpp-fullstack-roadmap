- [一、scoped_array](#一scoped_array)
- [二、此类特点如下](#二此类特点如下)
- [三、怎么使用scoped_array](#三怎么使用scoped_array)
- [四、scoped_array源码的实现](#四scoped_array源码的实现)

## 一、scoped_array

是专门对数组空间进行管理的。包装了new[]在堆上分配的动态数组；

scoped_array弥补了标准库中没有指向数组的智能指针的缺憾。

## 二、此类特点如下

1. 构造函数接受的指针p必须是new[]的结果，不能是new；
2. 没有*、->操作符的重载(库中不提供这些的重载，但是我们可以自己写)，因为scoped_array所持有的不是一个普通的指针；
3. **析构则必须用delete [];**
4. **提供operator[]的重载，可以像普通数组一样进行下标访问元素；**
5. 没有begin()、end()等类似容器的迭代器操作函数；

**scoped_array与scoped_ptr有相同的设计思想，也是局部智能指针，不能拷贝和赋值；**

## 三、怎么使用scoped_array

```cpp
#include<iostream>
#include<boost/smart_ptr.hpp> //内部实现好的，直接调用系统的。
using namespace std;
using namespace boost;  //这个命名空间必须要有。

int main(void){
    int *p = new int[10];  //申请数组空间
    scoped_array<int> ps(p); //交与智能指针管理

    for(int i = 0; i < 10; i++){
        ps[i] = i+1;  //可以进行下标操作
    }
    for(i = 0; i < 10; i++){
        cout<<ps[i]<<" ";
    }
    cout<<endl;
}
//拷贝构造和赋值都不可以。
```

## 四、scoped_array源码的实现

```cpp
#include<iostream>
using namespace std;

template<class T>
class scoped_array{
public:
    explicit scoped_array(T *p = 0) : px(p){} //预防隐式调用
    ~scoped_array(){
        delete []px;
    }
public:
    typedef scoped_array<T> this_type;
    void reset(T *p = 0){ //重置方法
        this_type.swap(*this);//无名临时对象
    }
    void swap(scoped_array &b){
        T *tmp = b.px;
        b.px = px;
        px = tmp;
    }
    T* get()const{
        return px;
    }
    T& operator[](int i)const{ //下标越界没有检测
        //return *(px+i);
        return px[i];
    }
    T& operator*()const{
        return px[0];
    }
    T* operator+(int i)const{
        return px+i;
    }

private:
    T *px;
    scoped_array(scoped_array const &);//放到私有中，外界无法调用
    scoped_array& operator=(scoped_array const &);
    
    void operator==(scoped_array const &)const;
    void operator!=(scoped_array const &)const;
};

int main(void){
    int *p = new int[10];
    scoped_array<int> ps(p);

    *ps = 2;

    for(int i = 0; i < 10; i++){
        ps[i] = i+1;
    }
    *(ps + 3) = 100; //利用 + ，*的运算符的重载即可以实现。
    for(i = 0; i < 10; i++){
        cout<<ps[i]<<" ";
    }
    cout<<endl;
}
```

库中没有提供*和+的重载。

scoped_array缺点：不能动态增长，没有迭代器支持，不能搭配STL算法，是纯粹的裸接口，不推荐使用。





## 面试辅导

### 面试考点全景图

| 维度 | 回答要点 | 面试高频追问 |
| --- | --- | --- |
| 核心定义 | `scoped_array` 属于Boost 与资源管理，重点是智能指针和资源包装器提供的所有权表达。 | 它解决了什么问题，边界在哪里？ |
| 关键角色 | 所有者、控制块、观察者、删除器。 | 各角色的职责、依赖方向和生命周期是什么？ |
| 优点 | 通过清晰边界降低耦合，并让复杂度、资源或并发控制可被推理。 | 性能收益是否可量化？ |
| 代价 | 会引入额外抽象、状态管理或运行时/维护成本。 | 何时不应使用？ |
| 适用场景 | 需求满足本节的约束，并且需要明确演进、资源或并发边界时。 | 如何在现有工程中渐进式落地？ |

### 面试官追问链

1. **基础：** 请定义 `scoped_array`，并用本节示例说明它解决的实际问题。
2. **机制：** 说明资源由谁释放、何时释放，以及共享所有权的代价；关键对象、调用顺序或数据流是什么？
3. **深入：** 与原始指针或其他智能指针相比，所有权模型、引用计数、数组支持和迁移成本有什么取舍？
4. **工程：** 出现异常、资源不足、并发竞争或输入非法时如何保证正确性？
5. **优化：** 在基准数据明确后，如何定位瓶颈并以复杂度、延迟或内存指标验证优化？

### 横向对比

| 对比项 | `scoped_array` | 原始指针或其他智能指针 | 选择建议 |
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
// 面试考点：这里说明 `scoped_array` 的职责边界，而不是重复代码字面含义。
// 所有权/并发：明确谁创建、谁释放资源；共享状态必须说明同步策略。
// 复杂度：标注关键路径的时间和空间复杂度，说明优化前提。
// 扩展性：通过稳定接口隔离变化，避免让调用方依赖具体实现。
```

### 可能的面试题

- **可能的面试题：** “请结合 `scoped_array` 说明设计/实现选择，并给出一个失败场景。”
- **简要答案：** 先确定唯一/共享/非拥有语义，再解释控制块、循环引用和异常安全。
