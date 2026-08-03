- [一、array类](#一array类)
- [二、string类](#二string类)

## 一、array类

```cpp
#include<iostream>
using namespace std;

class Array{
    public:
        Array(int count);
        Array(const Array &t);
        ~Array();
    public:
        void setData(int i, int data);
        int getData(int i);
        int length();
    private:
        int len;
        int *p;
};

Array::Array(int count){
    len = count;
    p = new int[len];
}
//有指针,的进行深拷贝;
Array::Array(const Array &t){
    len = t.len;
    p = new int[len];
    for(int i = 0; i < t.len; i++){
        p[i] = t.p[i];
    }

}
Array::~Array(){
    if(p){
        delete []p;
        p = NULL;
    }

}
void Array::setData(int i, int data){
    p[i] = data;
}
int Array::getData(int i){
    return p[i];
}
int Array::length(){
    return len;
}


int main(void){
    Array array(10);
    int i;

    for(i = 0; i < array.length(); i++){     
        array.setData(i, i);
    }

    for(i = 0; i < array.length(); i++){
        cout<<array.getData(i)<<" ";
    }
    cout<<endl;

    Array array1 = array;
    for(i = 0; i < array1.length(); i++){
        cout<<array1.getData(i)<<" ";
    }
    cout<<endl;


    return 0;
}
```

> **文本化验证：** 执行示例后应核对返回值、输出顺序和资源状态；同时覆盖空输入、边界输入与失败路径。

## 二、string类

```cpp
#include<iostream>
#include<stdio.h>
#include<string.h>
using namespace std;

class MyString{
    public:
        friend ostream& operator<<(ostream &out, const MyString &s1);
        friend istream& operator>>(istream &in, MyString &s2);
        MyString(int len = 0){ //默认参数看我们是否自己开辟大小的空间;
            if(len != 0){ 
                m_len = len;
                m_p = new char[m_len+1];
                memset(m_p, 0, m_len);
            }else{

                m_len = 0;
                m_p = new char[m_len+1];    
                strcpy(m_p, "");
            }   
        }   
        MyString(const char *p){
            if(p == NULL){
                m_len = 0;
                m_p = new char[m_len+1];    
                strcpy(m_p, "");
            }else{
                m_len = strlen(p);
                m_p = new char[m_len+1];
                strcpy(m_p, p);
            }
        }
        MyString(const MyString &s){
            m_len = s.m_len;
            m_p = new char[m_len+1];
            strcpy(m_p, s.m_p);
        }
        MyString& operator=(const MyString &t){
            if(m_p){
                delete []m_p;
                m_p = NULL;
                m_len = 0;
            }

            m_len = t.m_len;
            m_p = new char[m_len+1];
            strcpy(m_p, t.m_p);

            return *this;
        }
        ~MyString(){
            if(m_p) {
                delete []m_p;         
                m_p = NULL;
                m_len = 0;
            }
        }
    public:
        MyString operator=(const char *p){
            if(m_p){
                delete []m_p;
                m_p = NULL;
                m_len = 0;
            }
            if(p == NULL){
                m_len = 0;
                m_p = new char[m_len+1];
                strcpy(m_p, "");
            }else{
                m_len = strlen(p);
                m_p = new char[m_len+1];
                strcpy(m_p, p);
            }

            return *this;
        }
        char& operator[](int index){
            return m_p[index];
        }     
        bool operator==(const char *p)const{  //判断与字符串是否相等,看长度和里面的内容是否相等!!!
            if(p == NULL){
                if(m_len == 0){
                    return true;
                }else{
                    return false;
                }
            }else{
                if(m_len == strlen(p)){
                    return !strcmp(m_p, p);
                }else{
                    return false;
                }
            }
        }
        bool operator==(const MyString &s)const{
            if(m_len != s.m_len){
                return false;
            }

            return !strcmp(m_p, s.m_p);
        }

        bool operator!=(const char *p)const{
            return !(*this == p);
        }   
        bool operator!=(const MyString &s)const{
            return !(*this == s);
        }
        int operator<(const char *p)const{
            return strcmp(m_p, p);
        }
        int operator<(const MyString &s)const{
            return strcmp(m_p, s.m_p);
        }

        int operator>(const char *p)const{
            return strcmp(p, m_p);
        }
        int operator>(const MyString &s)const{
            return strcmp(s.m_p, m_p);
        }
        //怎么样把类的指针露出来.

    public:
        char *c_str(){     
            return m_p;
        }
        const char *c_str2(){
            return m_p;
        }
        int length(){
            return m_len;
        }
    private:
        int m_len;
        char *m_p;
};


ostream& operator<<(ostream &out, const MyString &s1){
    out<<s1.m_p;

    return out;
}

istream& operator>>(istream &in, MyString &s2){
    in>>s2.m_p;

    return in;
}
int main(void){    
    MyString s1;
    MyString s2("s2");
    MyString s3 = s2;
    MyString s4 = "s444444444444";

    s4 = "s22222222222";

    s4 = s2;

    s4[1] = '3';
    printf("%c\n", s4[1]); //测试[]改变值了吗?
    
    cout<<s4<<endl;

    if(s2 == "s2"){
        cout<<"相等"<<endl;
    }else{
        cout<<"不相等"<<endl;
    }

    s3 = "aaa";
    
    int flag = (s3 < "bbb");
    if(flag < 0){
        cout<<"s3小于bbb"<<endl;  
    }else{
        cout<<"s3大于bbb"<<endl;
    }

    s3 = "adasf";
    strcpy(s3.c_str(), "sga");
    cout<<s3<<endl;

    MyString s9(100);//默认输入要开辟字符串的空间大小;
    cout<<"请输入一个数字 :";
    cin>>s9;
    cout<<s9<<endl;

    return 0;
}
```

运行结果:





## 面试辅导

### 面试考点全景图

| 维度 | 回答要点 | 面试高频追问 |
| --- | --- | --- |
| 核心定义 | `array与string` 属于C++ 基础，重点是语言语义、对象生命周期和资源所有权。 | 它解决了什么问题，边界在哪里？ |
| 关键角色 | 引用/指针、值语义/对象语义、RAII。 | 各角色的职责、依赖方向和生命周期是什么？ |
| 优点 | 通过清晰边界降低耦合，并让复杂度、资源或并发控制可被推理。 | 性能收益是否可量化？ |
| 代价 | 会引入额外抽象、状态管理或运行时/维护成本。 | 何时不应使用？ |
| 适用场景 | 需求满足本节的约束，并且需要明确演进、资源或并发边界时。 | 如何在现有工程中渐进式落地？ |

### 面试官追问链

1. **基础：** 请定义 `array与string`，并用本节示例说明它解决的实际问题。
2. **机制：** 定义语义并说明编译期与运行时成本；关键对象、调用顺序或数据流是什么？
3. **深入：** 与相近语言机制相比，是否改变对象所有权、生命周期或调用绑定方式有什么取舍？
4. **工程：** 出现异常、资源不足、并发竞争或输入非法时如何保证正确性？
5. **优化：** 在基准数据明确后，如何定位瓶颈并以复杂度、延迟或内存指标验证优化？

### 横向对比

| 对比项 | `array与string` | 相近语言机制 | 选择建议 |
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
// 面试考点：这里说明 `array与string` 的职责边界，而不是重复代码字面含义。
// 所有权/并发：明确谁创建、谁释放资源；共享状态必须说明同步策略。
// 复杂度：标注关键路径的时间和空间复杂度，说明优化前提。
// 扩展性：通过稳定接口隔离变化，避免让调用方依赖具体实现。
```

### 可能的面试题

- **可能的面试题：** “请结合 `array与string` 说明设计/实现选择，并给出一个失败场景。”
- **简要答案：** 先解释语义，再说明对象何时构造、复制、移动和销毁，最后给出异常安全或性能取舍。
