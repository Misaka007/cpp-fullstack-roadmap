- [一、AVL树删除](#一avl树删除)
- [二、AVL树删除代码](#二avl树删除代码)
- [三、完整代码+测试代码+运行结果](#三完整代码测试代码运行结果)
  - [3.1 完整代码](#31-完整代码)
  - [3.2 测试代码](#32-测试代码)
  - [3.3 运行结果](#33-运行结果)

## 一、AVL树删除

思路：

1. 首先找到要删除的结点；没找到，直接false返回退出即可；
2. 将其转化为只有一个分支的结点，前面的路径都要入栈；
3. 其父节点(parent)的平衡因子(根据父的左/右=p(要删除的结点)，修改父的bf)，有几种情况：
   1. 父节点的bf=1/-1，代表原先有两个结点，现在剩下一个了，直接退出循环，不用再往上寻找更改bf了；
   2. 父节点的bf=0；
   3. 代表此时的往上更改爷爷结点(在此出栈即可，栈中保存了路径信息)的bf，看情况(bf=2/-2)是否进行旋转，和要进行相应的旋转方式；
4. 判断栈空，进行相应的连接操作；
5. 最后删除这个结点。

相应部分情况：
### 流程图与伪代码：AVL 删除后的回溯修复

这一节先用流程图查看判断与状态更新，再用伪代码将操作拆成可验证的步骤。阅读时先确认边界条件，再观察成功分支如何维护结构不变量。

**原理：** 删除叶子或只含一个孩子的结点后，只有它的祖先子树高度可能降低，所以从实际删除位置的父结点向根回溯即可。若删除的结点有两个孩子，通常先用前驱或后继替换它的值，再删除那个至多只有一个孩子的替代结点。删除与插入的区别是：旋转后子树高度仍可能继续降低，因此修复完当前结点不能总是停止，必须继续检查更高层祖先。

```text
             +-------+
             | START |  <-- 开始操作
             +---+---+
                 |
                 v
        +------------------+
        | 找到待删除结点？ |
        +----+--------+----+
          否 |        | 是  <-- 条件不满足时走返回路径
             v        v
  +------------------+  +------------------+
  | RETURN false |  | 删除结点或以替代结点交换 |
  +--------+---------+  +--------+---------+
           |                     |
           v                     v
       +-------+        +------------------+
       |  END  |        | 向上更新 bf，必要时旋转 |
       +-------+        +--------+---------+
                                  |
                                  v
                         +------------------+
                         | RETURN 成功      |
                         +--------+---------+
                                  |
                                  v
                              +-------+
                              |  END  |
                              +-------+
```

```text
算法 AVLDelete(root, key)
    找到 key 对应结点；若不存在则返回 false
    若结点有两个孩子，用前驱或后继替代后删除替代结点
    从实际删除位置的父结点开始向上回溯
        更新平衡因子
        如果出现 ±2，执行对应旋转
        若当前子树高度不再变化，可停止回溯
    结束回溯
    返回 true
结束算法
```

## 二、AVL树删除代码

```cpp
template<typename Type>
bool AVLTree<Type>::remove(AVLNode<Type> *&t, const Type &x){
    AVLNode<Type> *p = t;
    AVLNode<Type> *parent = NULL;  //父结点
    AVLNode<Type> *q = NULL;  //删除结点的辅助结点
    stack<AVLNode<Type> *> st;

    AVLNode<Type> *ppr; //爷爷结点

    int flag = 0;
    while(p != NULL){
        if(p->data == x){
            break;
        }
        parent = p;
        st.push(parent);
        if(x < p->data){
            p = p->leftChild;
        }else{
            p = p->rightChild;
        }
    } //以上是：查找删除点
    if(p == NULL){  //没有要删除的结点
        return false;
    }
    if(p->leftChild!= NULL && p->rightChild!=NULL){
        parent = p;
        st.push(parent);

        q = p->leftChild;
        while(q->rightChild != NULL){
            parent = q;
            st.push(parent);
            q = q->rightChild;
        }

        p->data = q->data;
        p = q;
    }
    
    if(p->leftChild != NULL){
        q = p->leftChild;
    }else{
        q = p->rightChild;
    }
//以上是：使其要删除的转化为只有一个分支的
    if(parent == NULL){  //删除的是根结点，并且无入栈，代表只有一个分支，并没有寻找
        t = q;  
    }else{
        if(parent->leftChild == p){
            flag = 0;
            parent->leftChild = q;
        }else{
            flag = 1;
            parent->rightChild = q;
        }

        while(!st.empty()){
            parent = st.top();
            st.pop();
            if(parent->leftChild==q){ //对要删除的父节点更改bf;
                parent->bf++;
            }else{
                parent->bf--;
            }
            if(!st.empty()){
                ppr = st.top();
                if(ppr->leftChild == parent){
                    flag = 0;
                }else{
                    flag = 1;
                }
            }
            if(parent->bf==-1 || parent->bf==1 ){
                break; //删除前的平衡因子为0，此时不用再调整其它平衡因子,直接退出循环;
            }

            if(parent->bf == 0){  //原先只有左孩子/右孩子
                q = parent; //往上回溯更改爷爷结点的bf；
            }else{  //此时到达2,已经不平衡了，的进行旋转化的调整
                if(parent->bf < 0){
                    flag = -1;
                    q = parent->leftChild;
                }else{
                    flag = 1;
                    q = parent->rightChild;
                }
                if(q->bf == 0){
                    if(flag == -1){
                        
                    }
                }
                if(parent->bf > 0){
                    q = parent->rightChild;
                    if(q->bf == 0){
                        RotateL(parent);
                    }else if(q->bf > 0){
                        RotateL(parent);
                    }else{
                        RotateRL(parent);
                    }
                }else{
                    q = parent->leftChild;
                    if(q->bf == 0){
                        RotateR(parent);
                    }else if(q->bf < 0){
                        RotateR(parent);
                    }else{
                        RotateLR(parent);
                    }
                }
            }
        }
        if(st.empty()){
            t = parent;  //直接更改root
        }else{
            AVLNode<Type> *tmp = st.top();  //当前的栈顶结点使其的左/右指向parent(是旋转化后的根);
            if(parent->data < tmp->data){  
                tmp->leftChild = parent;
            }else{
                tmp->rightChild = parent;
            }
        }

    }

    delete p;  //删除结点;
    return true;
}
```

## 三、完整代码+测试代码+运行结果

> **文本化验证：** 运行测试后应覆盖正常输入、边界输入和错误路径；核对输出顺序、返回值与关键数据结构状态是否符合预期。

### 3.1 完整代码

```cpp
#ifndef _AVL_TREE_H_
#define _AVL_TREE_H_

#include<iostream>  //引入头文件
#include<stack>    //要用栈保存路径信息
using namespace std;

template<typename Type>
class AVLTree;

template<typename Type>
class AVLNode{   //AVL树的结点
    friend class AVLTree<Type>;
public:
    AVLNode() : data(Type()), leftChild(NULL), rightChild(NULL), bf(0){}
    AVLNode(Type d, AVLNode *left = NULL, AVLNode *right = NULL) 
        : data(d), leftChild(left), rightChild(right), bf(0){}
    ~AVLNode(){}
private:
    Type data;
    AVLNode *leftChild;
    AVLNode *rightChild;
    int bf;  //多了一个平衡因子
};

template<typename Type>
class AVLTree{   //AVL树的类型
public:
    AVLTree() : root(NULL){}
public:
    bool insert(const Type &x){
        return insert(root, x);
    }
    bool remove(const Type &x){
        return remove(root, x);
    }
    void inOrder()const{
        inOrder(root);
    }
protected:
    void inOrder(AVLNode<Type> *t)const{
        if(t != NULL){
            inOrder(t->leftChild);
            cout<<t->data<<" : "<<t->bf<<endl;;
            inOrder(t->rightChild);
        }
    }
    bool insert(AVLNode<Type> *&t, const Type &x); //插入函数
    bool remove(AVLNode<Type> *&t, const Type &x);
    void RotateR(AVLNode<Type> *&ptr){  //右旋
        AVLNode<Type> *subR = ptr;
        ptr = ptr->leftChild;
        subR->leftChild = ptr->rightChild;
        ptr->rightChild = subR;
        ptr->bf = subR->bf = 0;
    }
    void RotateL(AVLNode<Type> *&ptr){  //左旋
        AVLNode<Type> *subL = ptr;
        ptr = subL->rightChild;
        subL->rightChild = ptr->leftChild;
        ptr->leftChild = subL;
        subL->bf = ptr->bf = 0;
    }
    void RotateLR(AVLNode<Type> *&ptr){  //先左后右旋转
        AVLNode<Type> *subR = ptr;
        AVLNode<Type> *subL = ptr->leftChild;
        ptr = subL->rightChild;

        subL->rightChild = ptr->leftChild;
        ptr->leftChild = subL;
        if(ptr->bf <= 0){
            subL->bf = 0;
        }else{
            subL->bf = -1;
        }

        subR->leftChild = ptr->rightChild;
        ptr->rightChild = subR;
        if(ptr->bf == -1){
            subR->bf = 1;
        }else{
            subR->bf = 0;
        }

        ptr->bf = 0;
    }
    void RotateRL(AVLNode<Type> *&ptr){  //先右后左旋转
        AVLNode<Type> *subL = ptr;
        AVLNode<Type> *subR = ptr->rightChild;
        ptr = subR->leftChild;

        subR->leftChild = ptr->rightChild;
        ptr->rightChild = subR;
        if(ptr->bf >=0){
            subR->bf = 0;
        }else{
            subR->bf = 1;
        }

        subL->rightChild = ptr->leftChild;
        ptr->leftChild = subL;
        if(ptr->bf == 1){
            subL->bf = -1;
        }else{
            subL->bf = 0;
        }
        ptr->bf = 0;
    }
private:
    AVLNode<Type> *root;
};

template<typename Type>
bool AVLTree<Type>::insert(AVLNode<Type> *&t, const Type &x){
    AVLNode<Type> *p = t;
    AVLNode<Type> *parent = NULL; // 记录前驱结点，方便连接和调整平衡因子
    stack<AVLNode<Type> *> st; //用栈记录插入的路径，方便调整栈中结点的平衡因子;
    int sign;

    while(p != NULL){
        if(x == p->data){ //要插入的数据和AVL树中的数字相同,则返回失败！
            return false;
        }

        parent = p;
        st.push(parent); //找过的入栈
        if(x < p->data){
            p = p->leftChild;
        }else if(x > p->data){
            p = p->rightChild;
        }
    } // 找插入位置,不用递归，就是为了记录路径信息
    
    p = new AVLNode<Type>(x);
    if(parent == NULL){
        t = p;    //判断是不是第一个结点，进行root的连接;
        return true;
    }

    if(x < parent->data){ //此时通过父节点的数据判断插入的是左还是右
        parent->leftChild = p;
    }else{
        parent->rightChild = p;
    }
    //新插入点的bf为0,关键是栈中的平衡因子的调整
/////////////////////////////////////////////////////// 以上完成插入工作
    while(!st.empty()){  //栈不空，出栈顶元素
        parent = st.top();
        st.pop();

        if(p == parent->leftChild){   //判断插入的是父节点的左/右孩子,
            parent->bf--;           //让其bf++/--;
        }else{
            parent->bf++;
        }

        //以下判断栈中的平衡因子，看是否需要进行旋转调整
        if(parent->bf == 0){  //bf=0，直接跳出循环
            break;
        }
        if(parent->bf==1 || parent->bf==-1){ 
            p = parent;  //此时在向上走，判断bf;
        }else{  //以下的bf为2/-2;利用标志判断左右旋;
            sign = parent->bf > 0 ? 1 : -1;
            if(p->bf == sign){  //符号相同为单旋
                if(sign == 1){  //为1左旋
                    RotateL(parent);  
                }else{
                    RotateR(parent); //右旋
                }
            }else{  //符号不同，为双旋
                if(sign == 1){  
                    RotateRL(parent); //为1右左
                }else{
                    RotateLR(parent);
                }
            }
/*
    以下方法也可以判断左右旋
        else
        {
            if(parent->bf < 0)  //左边
            {
                if(p->bf<0 && p==parent->leftChild)    //    / 只能是左孩子
                {
                    //RotateR(parent);
                }
                else if(p->bf>0 && p == parent->leftChild)  //   <
                {
                    //RotateLR(parent);
                }
            }
            else
            {
                if(p->bf>0 && p==parent->rightChild)   //   \ 
                {
                    //RotateL(parent);
                }
                else if(p->pf<0 && p==parent->rightChild)  //      >
                {
                    //RotateRL(parent);
                }
            }
        }

*/
    break;
        }
    }

    if(st.empty()){  //通过旋转函数，此时parent指向根节点;
        t = parent;  //此时调到栈底了，旋转后将更改root的指向
    }else{
        AVLNode<Type> *tmp = st.top();  //当前的栈顶结点
        if(parent->data < tmp->data){  
            tmp->leftChild = parent;
        }else{
            tmp->rightChild = parent;
        }
    }

    return true;
}

template<typename Type>
bool AVLTree<Type>::remove(AVLNode<Type> *&t, const Type &x){
    AVLNode<Type> *p = t;
    AVLNode<Type> *parent = NULL;  //父结点
    AVLNode<Type> *q = NULL;  //删除结点的辅助结点
    stack<AVLNode<Type> *> st;

    AVLNode<Type> *ppr; //爷爷结点

    int flag = 0;
    while(p != NULL){
        if(p->data == x){
            break;
        }
        parent = p;
        st.push(parent);
        if(x < p->data){
            p = p->leftChild;
        }else{
            p = p->rightChild;
        }
    } //以上是：查找删除点
    if(p == NULL){  //没有要删除的结点
        return false;
    }
    if(p->leftChild!= NULL && p->rightChild!=NULL){
        parent = p;
        st.push(parent);

        q = p->leftChild;
        while(q->rightChild != NULL){
            parent = q;
            st.push(parent);
            q = q->rightChild;
        }

        p->data = q->data;
        p = q;
    }
    
    if(p->leftChild != NULL){
        q = p->leftChild;
    }else{
        q = p->rightChild;
    }
//以上是：使其要删除的转化为只有一个分支的
    if(parent == NULL){  //删除的是根结点，并且无入栈，代表只有一个分支，并没有寻找
        t = q;  
    }else{
        if(parent->leftChild == p){
            flag = 0;
            parent->leftChild = q;
        }else{
            flag = 1;
            parent->rightChild = q;
        }

        while(!st.empty()){
            parent = st.top();
            st.pop();
            if(parent->leftChild==q){ //对要删除的父节点更改bf;
                parent->bf++;
            }else{
                parent->bf--;
            }
            if(!st.empty()){
                ppr = st.top();
                if(ppr->leftChild == parent){
                    flag = 0;
                }else{
                    flag = 1;
                }
            }
            if(parent->bf==-1 || parent->bf==1 ){
                break; //删除前的平衡因子为0，此时不用再调整其它平衡因子
            }

            if(parent->bf == 0){  //原先只有左孩子/右孩子
                q = parent; //往上回溯更改爷爷结点的bf；
            }else{  //此时到达2,已经不平衡了，的进行旋转化的调整
                if(parent->bf < 0){
                    flag = -1;
                    q = parent->leftChild;
                }else{
                    flag = 1;
                    q = parent->rightChild;
                }
                if(q->bf == 0){
                    if(flag == -1){
                        
                    }
                }
                if(parent->bf > 0){
                    q = parent->rightChild;
                    if(q->bf == 0){
                        RotateL(parent);
                    }else if(q->bf > 0){
                        RotateL(parent);
                    }else{
                        RotateRL(parent);
                    }
                }else{
                    q = parent->leftChild;
                    if(q->bf == 0){
                        RotateR(parent);
                    }else if(q->bf < 0){
                        RotateR(parent);
                    }else{
                        RotateLR(parent);
                    }
                }
            }
        }
        if(st.empty()){
            t = parent;  //直接更改root
        }else{
            AVLNode<Type> *tmp = st.top();  //当前的栈顶结点使其的左/右指向parent(是旋转化后的根);
            if(parent->data < tmp->data){  
                tmp->leftChild = parent;
            }else{
                tmp->rightChild = parent;
            }
        }

    }

    delete p;  //删除结点;
    return true;
}
#endif
```

### 3.2 测试代码

```cpp
#include"avlTree.h"

int main(void){
    int ar[] = {16, 3, 7, 11, 9, 26, 18, 14, 15,};
    int n = sizeof(ar) / sizeof(int);
    AVLTree<int> avl;

    for(int i = 0; i < n; i++){
        avl.insert(ar[i]);
    }

    cout<<"删除前: "<<endl;
    avl.inOrder();
    avl.remove(16);
    cout<<"删除后: "<<endl;
    avl.inOrder();
    return 0;
}
```

### 3.3 运行结果

> **文本化验证：** 运行测试后应覆盖正常输入、边界输入和错误路径；核对输出顺序、返回值与关键数据结构状态是否符合预期。





## 面试辅导

### 面试考点全景图

| 维度 | 回答要点 | 面试高频追问 |
| --- | --- | --- |
| 核心定义 | `AVL删除` 属于数据结构与算法，关注数据表示、不变量和操作复杂度之间的取舍。 | 它解决什么问题，前置条件是什么？ |
| 关键角色 | 结点/数组、索引/指针、根/头尾指针、不变量。 | 每个角色的状态、生命周期和责任边界是什么？ |
| 优点 | 通过明确数据、资源或执行边界提高可推理性。 | 复杂度、性能或维护收益如何验证？ |
| 代价 | 会带来状态维护、边界检查或实现复杂度。 | 在什么场景下不适合使用？ |
| 应用场景 | 需要逻辑结构、物理存储、核心操作、复杂度和边界状态的程序或系统任务。 | 如何从现有实现平滑迁移？ |

### 面试官追问链

1. **基础：** 请定义 `AVL删除`，说明它的输入、输出和核心约束。
2. **机制：** 逻辑结构、物理存储、核心操作、复杂度和边界状态；关键状态如何变化？
3. **深入：** 与相邻数据结构或实现方式相比，内存布局、时间复杂度、空间复杂度、迭代稳定性和适用负载有什么不同？
4. **工程：** 失败、边界输入、资源耗尽或并发访问时如何保持正确性？
5. **优化：** 用哪些时间、空间、吞吐或可观测性指标验证优化有效？

### 横向对比

| 对比项 | `AVL删除` | 相邻数据结构或实现方式 | 选择建议 |
| --- | --- | --- | --- |
| 核心目标 | 数据表示、不变量和操作复杂度之间的取舍 | 解决相邻但边界不同的问题 | 按问题约束选择，而不是只按熟悉程度选择。 |
| 主要成本 | 状态维护、边界处理与测试成本 | 成本模型不同，可能偏向性能或抽象能力 | 用实际数据规模和故障模型评估。 |
| 适用条件 | 需要逻辑结构、物理存储、核心操作、复杂度和边界状态 | 需要不同的资源、数据或执行模型 | 优先保证正确性、可观测性和可维护性。 |

### 缺失知识点与工程注意事项

- 面试中要同时说明空结构、单元素、扩容/缩容、删除和重复键等边界条件。
- 回答时应区分标准/系统保证与具体实现细节；未定义行为和未检查错误码不能作为可靠前提。
- 测试至少覆盖正常路径、边界路径和失败路径，并验证资源是否按预期释放或回收。

### 代码注释提示

```c
// 面试考点：说明此处的数据不变量或接口契约。
// 边界条件：明确空输入、容量上限、错误返回和资源释放责任。
// 复杂度：标注关键操作的时间/空间复杂度及其前提。
// 工程约束：共享状态、系统调用或指针访问需要说明同步/错误处理策略。
```

### 可能的面试题

- **问题：** “请说明 `AVL删除` 的核心不变量，并描述一个最容易出错的边界场景。”
- **简要答案：** 先写出不变量，再用插入、删除和查询操作证明不变量在每一步都被维护。
