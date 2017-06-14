/*
 * Copyright (c) 2016 WangBin <wbsecg1 at gmail.com>/<binwang at pptv.com>
 */
#include "ThreadLocal.h"
#include <iostream>
#include <string>
#include <thread>
using namespace std;
// TODO: compare results and exit(result)
// default is to try c++11 thread_local: cxx ThreadLocal.cpp c++11flags
// mingw desktop use fiber api: g++ -DUSE_STD_THREAD_LOCAL=0 -std=c++11 ThreadLocal.cpp -D_WIN32_WINNT=0x0600 
// mingw desktop use pthread: g++ -DUSE_STD_THREAD_LOCAL=0 -std=c++11 ThreadLocal.cpp
// mingw store use fiber api: g++ -DUSE_STD_THREAD_LOCAL=0 -std=c++11 ThreadLocal.cpp 
// vc use fiber api: cl -DUSE_STD_THREAD_LOCAL=0 ThreadLocal.cpp /MD /EHsc 
// clang use pthread: clang++ -DUSE_STD_THREAD_LOCAL=0 ThreadLocal.cpp

struct X {
    X(const string& v = string()) : x(v) {
       std::cout << FUNCINFO <<this << ": " << x << ", thread " << this_thread::get_id() << std::endl;
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

void test_assign()
{
    static THREAD_LOCAL(X) x = X("assing T");
}

void test_ctor()
{
    static THREAD_LOCAL(X) x(X("ctor T"));
}

void test_ptr()
{
    static THREAD_LOCAL(X*) x(new X("T* delete test"));
    X* y = x;
    delete y;
}

void test_operator_T()
{
    static THREAD_LOCAL(string) x(string("operator T"));
    std::cout << (string&)x << std::endl;
}

void test_operator_addressof()
{
    static THREAD_LOCAL(X) x(X("operator&() on THREADL_LOCAL like c++ thread_local test"));
    X *y = &x;
    *y = X("operator&() on THREAD_LOCAL like c++ thread_local test ok");
}

int main()
{
    thread(test_operator_addressof).join();
    static THREAD_LOCAL(X) x(X("init in main"));
    thread([&]{
        x = X("set in thread");
        std::cout << FUNCINFO << ": " << (string&)(X&)x << ", thread " << this_thread::get_id() << std::endl;
    }).join();

    thread(test_operator_T).join();
    thread(test_ptr).join();
    thread(test_ctor).join();
    thread(test_ctor).join();
    thread(test_assign).join();
    thread(test_assign).join();
    return 0;
}
