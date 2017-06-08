/*
 * Copyright (c) 2016 WangBin <wbsecg1 at gmail.com>/<binwang at pptv.com>
 */
#include "ThreadLocal.h"

// default is to try c++11 thread_local: cxx ThreadLocal.cpp c++11flags
// mingw desktop use fiber api: g++ -DUSE_STD_THREAD_LOCAL=0 -std=c++11 ThreadLocal.cpp -D_WIN32_WINNT=0x0600 
// mingw desktop use pthread: g++ -DUSE_STD_THREAD_LOCAL=0 -std=c++11 ThreadLocal.cpp
// mingw store use fiber api: g++ -DUSE_STD_THREAD_LOCAL=0 -std=c++11 ThreadLocal.cpp 
// vc use fiber api: cl -DUSE_STD_THREAD_LOCAL=0 ThreadLocal.cpp /MD /EHsc 
// clang use pthread: clang++ -DUSE_STD_THREAD_LOCAL=0 ThreadLocal.cpp

THREAD_LOCAL(int) a = 11; // construct from T

void setA(int* x, int v)
{
    *x = v;
}


int main()
{
    std::cout << FUNCINFO << " initial a: " << (int)a << ", thread " << std::this_thread::get_id() << std::endl;
    a = 123; // assignment
    std::thread t([]{
        setA(&a, 456); // operator&()
        std::cout << FUNCINFO << " a: " << (int)a << ", thread " << std::this_thread::get_id() << std::endl;
    });
    t.join();
    std::cout << FUNCINFO << " a: " << (int)a << ", thread " << std::this_thread::get_id() << std::endl;
    struct X {
        X(int v = 0) : x(v) {std::cout << this  << " x: " << x<< std::endl;}
        ~X() {
            std::cout << FUNCINFO <<this << "  x: " << x << ", thread " << std::this_thread::get_id() << std::endl;
        }
        operator int&() {return x;}
        int x;
    };
    static THREAD_LOCAL(X) x;
    x = X(1);
    static THREAD_LOCAL(X*) y = new X(333);
    X* yy = y;
    delete yy;
    std::thread t1([&]{
        x = 3;
        std::cout << FUNCINFO << " x: " << (X&)x << ", thread " << std::this_thread::get_id() << std::endl;
    });
    t1.join();
    std::cout << FUNCINFO << " x: " << X(x) << ", thread " << std::this_thread::get_id() << std::endl;
    return a;
}
