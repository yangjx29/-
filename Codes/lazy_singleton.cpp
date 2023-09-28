#include <iostream>
using namespace std;

class Singleton {
private:
    // 静态成员变量，用于保存唯一实例
    static Singleton* instance;
    int data;  // 单例对象的数据

    // 构造函数私有，放置外部创建对象
    Singleton() : data(0) {
    }

public:
    // 静态成员函数,用于获取单例实例
    static Singleton* getInstance() {
        if (instance == nullptr) {
            instance = new Singleton();  // 第一次访问时创建
        }
        return instance;
    }

    void setData(int val) {
        data = val;
    }

    int getData() const {
        return data;
    }
};

// 静态成员变量需要在类外初始化
Singleton* Singleton::instance = nullptr;

int main() {
    Singleton* s1 = Singleton::getInstance();
    Singleton* s2 = Singleton::getInstance();
    s1->setData(525);
    cout << "s1:" << s1->getData() << endl;
    cout << "s2:" << s1->getData() << endl;
    return 0;
}