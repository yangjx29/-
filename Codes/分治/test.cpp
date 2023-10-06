// #include <iostream>

// typedef void(FT)(int*, int*);
// FT x, y, *z, u;

// int main() {
//     int a = 1, b = 2;
//     x(&a, &b);
//     y(&a, &b);
//     z = y;  // 这里x是一个函数名,但是编译器会把函数名转换为函数指针变量,所以也可以显式写成z = &x;
//     z(&a, &b);
//     // u = x;//这里出错了这里u是一个函数名称,是一个指针常量,类似于数组名称,是不能够被赋值的
//     return 0;
// }

// void x(int* a, int* b) {
//     *a = *a + *b;
//     printf("%d\n", *a);
// }

// void y(int* a, int* b) {
//     *a = *a * (*b);
//     printf("%d\n", *a);
// }

#include <iostream>
int inc(int a) {
    return (++a);
}

int multi(int *a, int *b, int *c) {
    return (*c = *a * *b);
}

typedef int (*FUNC1)(int);
typedef int(FUNC2)(int *, int *, int *);
typedef int (*FUNC3)(int *, int *, int *);

void show(FUNC3 fun, int arg1, int *arg2)
// void   show(FUNC2 fun,int   arg1,   int*arg2)//这里做这样的声明也一样是对的
{
    FUNC1 p = inc;
    FUNC2 *q1 = multi;  // 这表示 q1 只能指向接受三个整数指针参数并返回整数的函数，因为它的类型是 int (int *, int *, int *)。
    FUNC3 q2 = multi;   // 它直接表示指向接受三个整数指针参数并返回整数的函数的指针。因此，q2 的类型是 int (*)(int *, int *, int *)。
    int temp = (*p)(arg1);
    //(*fun)(&temp, &arg1, arg2); 这是fun的类型是FUNC2的写法
    fun(&temp, &arg1, arg2);  // 这是fun的类型是FUNC3的写法
    // fun(&temp,&arg1,   arg2); //这里的两种调用方法和上面的恶两种声明方法可以任意组合，,原因是函数名和函数指针常量的隐式转换
    printf("%d\n", *arg2);
}

int main() {
    int a;
    show(multi, 1, &a);
    printf("a = %d\n", a);
    return 0;
}