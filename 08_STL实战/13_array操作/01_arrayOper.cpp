// 面试提示：说明此文件的职责边界、资源所有权和关键路径复杂度。
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
//��ָ��,�Ľ������;
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
