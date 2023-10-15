#include <iostream>
using namespace std;

class A {
public:
    virtual void foo() {
        cout << "A:: foo() is caled" << endl;
    }
};

class B : public A {
public:
    void foo() {
        cout << "B:: foo() is caled" << endl;
    }
};

int main() {
    A* a = new B(); // a可以访问到B类的虚表指针,进而访问B的虚表,从而访问到B的虚函数
    a->foo();
    return 0;
}