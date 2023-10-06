#include <iostream>
using namespace std;

class Singleton {
private:
    static Singleton* instance;
    int data;
    Singleton() : data(0) {
    }

public:
    void setData(int val){
        data = val;
    }

    int getData(){
        return data;
    }

    static Singleton* getInstance(){
        return instance;
    }
};

Singleton* Singleton::instance = new Singleton();

int main(){
    Singleton* obj1 = Singleton::getInstance();
    Singleton* obj2 = Singleton::getInstance();

    obj1->setData(42);
    std::cout << "Data from obj1: " << obj1->getData() << std::endl; // 输出 42
    std::cout << "Data from obj2: " << obj2->getData() << std::endl; // 输出 42

    return 0;
}