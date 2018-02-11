/*
 * Copyright (c) 2016-2018 WangBin <wbsecg1 at gmail.com>
 */
#include "ThreadLocal.h"
#include <iostream>
#include <string>
#include <thread>
using namespace std;
// TODO: compare results and exit(result)
// default is to try c++11 thread_local: cxx test.cpp c++11flags
// mingw desktop use fiber api: g++ -DUSE_STD_THREAD_LOCAL=0 -std=c++11 test.cpp -D_WIN32_WINNT=0x0600
// mingw desktop use pthread: g++ -DUSE_STD_THREAD_LOCAL=0 -std=c++11 test.cpp
// mingw store use fiber api: g++ -DUSE_STD_THREAD_LOCAL=0 -std=c++11 test.cpp
// vc use fiber api: cl -DUSE_STD_THREAD_LOCAL=0 test.cpp /MD /EHsc
// clang use pthread: clang++ -DUSE_STD_THREAD_LOCAL=0 test.cpp

struct X {
    X() {
        std::cout << FUNCINFO <<this << ": " << x << ", thread " << this_thread::get_id() << std::endl;
    }
    X(const string& v) : x(v) {
       std::cout << FUNCINFO <<this << ": " << x << ", thread " << this_thread::get_id() << std::endl;
    }
    X(string&& v) : x(std::move(v)) {
        std::cout << FUNCINFO <<this << ": " << x << ", thread " << this_thread::get_id() << std::endl;
    }
    X(const X& y) : x(y.x) {
        std::cout << FUNCINFO <<this << ": " << x << ", thread " << this_thread::get_id() << std::endl;        
    }
    X(X&& y) : x(std::move(y.x)) {
        std::cout << FUNCINFO <<this << ": " << x << ", thread " << this_thread::get_id() << std::endl;
    }
    X& operator=(const X& y) {
        x = y.x;
        std::cout << FUNCINFO <<this << ": " << x << ", thread " << this_thread::get_id() << std::endl;        
        return *this;
    }
    X& operator=(X&& y) {
        x = std::move(y.x);
        std::cout << FUNCINFO <<this << ": " << x << ", thread " << this_thread::get_id() << std::endl;        
        return *this;
    }
    ~X() {
       std::cout << FUNCINFO <<this << ": " << x << ", thread " << this_thread::get_id() << std::endl;
    }
    operator string&() {return x;}
    string x;
};

void setA(int* x, int v)
{
    *x = v;
}

void test_assign_move()
{
    std::cout << FUNCINFO << std::endl;
    static THREAD_LOCAL(X) x = X("move assignment T");
}

void test_assign_copy()
{
    std::cout << FUNCINFO << std::endl;
    X y = X("copy assignemnt T");
    static THREAD_LOCAL(X) x = y;
}

void test_ctor_move()
{
    std::cout << FUNCINFO << std::endl;
    static THREAD_LOCAL(X) x(X("move ctor T"));
}

void test_ctor_copy()
{
    std::cout << FUNCINFO << std::endl;
    X y("copy ctor T");
    static THREAD_LOCAL(X) x(y);
}

void test_ptr()
{
    std::cout << FUNCINFO << std::endl;
    static THREAD_LOCAL(X*) x(new X("T* delete test"));
    X* y = x;
    delete y;
}

void test_operator_T()
{
    std::cout << FUNCINFO << std::endl;
    static THREAD_LOCAL(string) x(string("operator T"));
    std::cout << (string&)x << std::endl;
}

void test_operator_addressof()
{
    std::cout << FUNCINFO << std::endl;
    static THREAD_LOCAL(X) x(X("operator&() on THREADL_LOCAL like c++ thread_local test"));
    X *y = &x;
    *y = X("operator&() on THREAD_LOCAL like c++ thread_local test ok");
}

// TODO: array of ptr new T[N]
int main()
{
    //thread(test_operator_addressof).join();
    static THREAD_LOCAL(X) x(X("init in main"));
    /*thread([&]{
        x = X("set in thread");
        std::cout << FUNCINFO << ": " << (string&)(X&)x << ", thread " << this_thread::get_id() << std::endl;
    }).join();*/

    printf("@%d\n", __LINE__);
    static THREAD_LOCAL(X) y;// = std::move(x);
    std::cout << FUNCINFO << " y: " << (string&)(X&)y << ", thread " << this_thread::get_id() << std::endl;
    printf("@%d\n", __LINE__);
    thread([]{
        std::cout << FUNCINFO << " x: " << (string&)(X&)x << ", thread " << this_thread::get_id() << std::endl;
    }).join();
    thread([]{
        std::cout << FUNCINFO << " y: " << (string&)(X&)y << ", thread " << this_thread::get_id() << std::endl;
    }).join();
    //y = x;
    printf("@%d\n", __LINE__);
    //x = std::move(y);
    printf("@%d\n", __LINE__);

    static THREAD_LOCAL(string) xx("789");
    /*thread([&]{
        x = X("set in thread");
        std::cout << FUNCINFO << ": " << (string&)(X&)x << ", thread " << this_thread::get_id() << std::endl;
    }).join();*/

    printf("@%d\n", __LINE__);
    static THREAD_LOCAL(string) yy(std::move(xx));
    std::cout << FUNCINFO << " yy: " << (string&)yy << ", thread " << this_thread::get_id() << std::endl;
    printf("@%d\n", __LINE__);
    thread([]{
        std::cout << FUNCINFO << " xx: " << (string&)xx << ", thread " << this_thread::get_id() << std::endl;
    }).join();
    thread([]{
        std::cout << FUNCINFO << " yy: " << (string&)yy << ", thread " << this_thread::get_id() << std::endl;
    }).join();
    return 0;
    thread(test_operator_T).join();
    thread(test_ptr).join();
    thread(test_ctor_move).join();
    thread(test_ctor_copy).join();
    thread(test_assign_move).join();
    thread(test_assign_copy).join();
    return 0;
}
