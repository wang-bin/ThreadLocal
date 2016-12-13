/*
 * Copyright (c) 2016 WangBin <wbsecg1 at gmail.com>/<binwang at pptv.com>
 */
#include "ThreadLocal.h"

// default is to try c++11 thread_local: cxx ThreadLocal.cpp -DTEST_TLS c++11flags
// mingw desktop use fiber api: g++ -DUSE_STD_THREAD_LOCAL=0 -std=c++11 ThreadLocal.cpp -D_WIN32_WINNT=0x0600  -DTEST_TLS
// mingw desktop use pthread: g++ -DUSE_STD_THREAD_LOCAL=0 -std=c++11 ThreadLocal.cpp -DTEST_TLS
// mingw store use fiber api: g++ -DUSE_STD_THREAD_LOCAL=0 -std=c++11 ThreadLocal.cpp  -DTEST_TLS
// vc use fiber api: cl -DUSE_STD_THREAD_LOCAL=0 ThreadLocal.cpp /MD /EHsc  -DTEST_TLS
// clang use pthread: clang++ -DUSE_STD_THREAD_LOCAL=0 ThreadLocal.cpp -DTEST_TLS

#ifdef TEST_TLS
THREAD_LOCAL(int) a;

void setA(int* x, int v)
{
    *x = v;
}


int main()
{
    a = 123;
    std::thread t([]{
        a = 456;
        std::cout << FUNCINFO << " a: " << (int)a << ", thread " << std::this_thread::get_id() << std::endl;
    });
    t.join();
    std::cout << FUNCINFO << " a: " << (int)a << ", thread " << std::this_thread::get_id() << std::endl;
    setA(&a, 456);
    std::cout << FUNCINFO << " a: " << (int)a << ", thread " << std::this_thread::get_id() << std::endl;
    struct X {
        X(int v = 0) : x(v) {}
        ~X() {
            std::cout << FUNCINFO << ", thread " << std::this_thread::get_id() << std::endl;
        }
        operator int&() {return x;}
        int x;
    };
    THREAD_LOCAL(X) x;
    x = X(1);
    std::thread t1([&]{
        x = 3;
        std::cout << FUNCINFO << " x: " << (X&)x << ", thread " << std::this_thread::get_id() << std::endl;
    });
    t1.join();
    std::cout << FUNCINFO << " x: " << X(x) << ", thread " << std::this_thread::get_id() << std::endl;
    return a;
}
#endif
