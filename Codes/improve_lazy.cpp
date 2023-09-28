#include <iostream>
#include <mutex>

class Singleton {
public:
    // 获取单例实例的静态方法
    static Singleton& getInstance() {
        // 使用局部静态变量确保只创建一次
        static Singleton instance;
        return instance;
    }

    void setData(int val) {
        data = val;
    }

    int getData() {
        return data;
    }

private:
    // 私有构造函数，防止外部实例化
    Singleton() {
        // 初始化单例对象的操作
    }

    int data;

    // 禁止拷贝构造和赋值操作
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};

int main() {
    // 获取单例实例
    Singleton& instance1 = Singleton::getInstance();
    Singleton& instance2 = Singleton::getInstance();

    // 验证是否为同一实例
    std::cout << (&instance1 == &instance2 ? "Same instance" : "Different instances") << std::endl;
    instance1.setData(66);
    std::cout << "instance1:" << instance1.getData() << std::endl;
    std::cout << "instance2:" << instance2.getData() << std::endl;
    return 0;
}
