/*
 * Copyright (c) 2016-2017 WangBin <wbsecg1 at gmail.com>/<binwang at pptv.com>
 */
#pragma once
#include <functional>
#include <memory> //default_delete
#include <thread>
#include <iostream>

#ifndef USE_STD_THREAD_LOCAL
#define USE_STD_THREAD_LOCAL 1 // 0: use our own implementation. 1: use c++11 thread_local if possible
#endif

// LSB linuxbase, apple clang for iOS(and macOS if xcode<8) has no thread_local, __thread
// apple: http://asciiwwdc.com/2016/sessions/405#t=354.596
// android libc++ is poor if libc is old: llvm-libc++abi//src/cxa_thread_atexit.cpp
#if defined(__clang__)
//#if defined(__apple_build_version__) /* Clang also masquerades as GCC */
# if __has_feature(cxx_thread_local) // TODO: check ndk r13 r14(3.8.275480 __clang_patchlevel__) becase libc++abi version is hard to check
#   define CC_HAS_THREAD_LOCAL
# endif
#elif defined(_MSC_VER) && _MSC_VER >= 1900
# define CC_HAS_THREAD_LOCAL
#elif defined(__GNUC__) && (__GNUC__*100+__GNUC_MINOR__ >= 408)
# define CC_HAS_THREAD_LOCAL
#endif

#if defined(CC_HAS_THREAD_LOCAL) && USE_STD_THREAD_LOCAL
#define THREAD_LOCAL(T) thread_local T
#else
#define THREAD_LOCAL(T) ThreadLocal<T>
#endif

// Fibers api is preferred for mingw targeting vista or later, and msvc
#if defined(_WIN32) // http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system
#include <windows.h>
#   if defined(_MSC_VER) || (!defined(USE_PTHREAD) && _WIN32_WINNT >= 0x0600) // default use fibers api for mingw targeting store/vista
#       define USE_FLS // vista, winstore
#   endif
#endif
#if !defined(USE_FLS)
#include <pthread.h>
#   ifndef USE_PTHREAD
#       define USE_PTHREAD
#   endif //USE_PTHREAD
#endif

#ifndef FUNCINFO
#if defined(_MSC_VER)
#define FUNCINFO __FUNCSIG__
#else
#define FUNCINFO __PRETTY_FUNCTION__
#endif
#endif

/*!
 * differences between `static ThreadLocal<T> t` and `static thread_local T s`
 * 1. t is an object of type ThreadLocal<T>, the thread data is of type T, while s is an object of T.
 * 2. t is constructed only once, while s is constructed when a new thread starts.
 * 3. thread data of t is constructed only when it's accessed in a new thread, or when t is constructed by assignment. while s (thread data is it's self) is constructed when a new thread starts.
 * \code
 * void f() { static THREAD_LOCAL(int) a = 1; }
 * \endcode
 * int data of value 1 constructed only once (when constructing ThreadLocal using get()).
 * \code
 * void f() { static THREAD_LOCAL(int) a(1); }
 * \endcode
 * int data of value 1 is not constructed because get() is not called.
 * All the diferences above does not affect your program.
 */
template<typename T>
class ThreadLocal
{
public:
    ThreadLocal() : ThreadLocal([]{return new T();}) {}
    ThreadLocal(const T& t) : ThreadLocal([t]{ return new T(t);}) {}
    ThreadLocal(T&& t) : ThreadLocal([t]{ return new T(t);}) {}
    ThreadLocal(std::function<T*()> c, std::function<void(T*)> d = std::default_delete<T>())
    : ctor_(c)
    , dtor_(d) {
#ifdef USE_PTHREAD
        pthread_key_create(&key_, default_exit);
#endif
#ifdef USE_FLS
        index_ = FlsAlloc(default_exit);
        if (index_ == FLS_OUT_OF_INDEXES)
            throw std::system_error(GetLastError(), std::system_category(), "FlsAlloc error");
#endif
        //get(); // set tls data when constructing ThreadLocal object, like operator=(T&&)
    }
    ~ThreadLocal() {
#ifdef USE_PTHREAD
        pthread_key_delete(key_);
#endif
#ifdef USE_FLS
        FlsFree(index_);
#endif
    }
    T* operator&() const { return get();}
    /*!
     * The following operators let ThreadLocal behaves like c++11 thread_local var, except T's member must be accessed by operator->(),
     * e.g. t->member (a workaround is explicitly convert to access: ((T&)t).member)
     */
    operator T&() const { return *get(); }
    ThreadLocal& operator=(const T& v) {
        *get() = v;
        return *this;
    }
    ThreadLocal& operator=(T&& v) {
        *get() = std::forward<T>(v);
        return *this;
    }
private:
    T* get() const {
        void* v = nullptr;
#if defined(USE_PTHREAD)
        v = pthread_getspecific(key_);
#elif defined(USE_FLS)
        v = FlsGetValue(index_);
#else
        return nullptr;
#endif
        if (v)
            return static_cast<Data*>(v)->t;
        Data *d = new Data();
        std::cout << FUNCINFO << " allocate and initialize ThreadLocal data" << std::endl << std::flush;
        d->t = ctor_();
        d->tl = this;
#if defined(USE_PTHREAD)
        pthread_setspecific(key_, d);
#elif defined(USE_FLS)
        FlsSetValue(index_, d);
#endif
        return d->t;
    }
    T* operator->() const { return get(); }
    static void
#ifdef USE_FLS
    WINAPI // WINAPI/__stdcall is required to avoid crash on 32bit target, ignored by x64/arm compiler
#endif
    default_exit(void* v) {
        Data* d = static_cast<Data*>(v);
        if (d && d->tl->dtor_)
            d->tl->dtor_(d->t);
        delete d;
    }

    struct Data {
        Data() { std::cout << FUNCINFO << " thread: " << std::this_thread::get_id() << std::endl; }
        ~Data() { std::cout << FUNCINFO << " thread: " << std::this_thread::get_id() << std::endl; }

        const ThreadLocal* tl;
        T* t;
    };
#ifdef USE_PTHREAD
    pthread_key_t key_;
#endif
#ifdef USE_FLS
    DWORD index_;
#endif
    // static ThreadLocal<T> var, var ctor will be called only once, so must store how var is allocated and initialized, using ctor_
    std::function<T*()> ctor_ = nullptr;
    std::function<void(T*)> dtor_ = nullptr;
};
