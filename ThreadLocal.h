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

// windows thread_local requires public ctor
/// Windows use fibers api if possible. You can add -DUSE_PTHREAD for winrt+mingw to use pthread for winrt, or -D_WIN32_WINNT=0x0600 for mingw to use fibers
// TODO: TlsAlloc() support for XP. Use RegisterSingleObject. currently must use vs2015 thread_local to support xp.
// LSB linuxbase, apple clang for iOS(and macOS if xcode<8) has no thread_local, __thread
// apple: http://asciiwwdc.com/2016/sessions/405#t=354.596
#if defined(__clang__)
//#if defined(__apple_build_version__) /* Clang also masquerades as GCC */
# if __has_feature(cxx_thread_local) // TODO: check ndk r13 r14(3.8.275480 __clang_patchlevel__)
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
# if defined(_WIN32) // http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system
#   include <windows.h>
#   if defined(_MSC_VER) || (!defined(USE_PTHREAD) && _WIN32_WINNT >= 0x0600) // default use fibers api for mingw targeting store/vista
#       define USE_FLS // vista, winstore
#   endif
# endif
# if !defined(USE_FLS)
#   include <pthread.h>
#   ifndef USE_PTHREAD
#       define USE_PTHREAD
#   endif //USE_PTHREAD
# endif
#endif

#ifndef FUNCINFO
#if defined(_MSC_VER)
#define FUNCINFO __FUNCSIG__
#else
#define FUNCINFO __PRETTY_FUNCTION__
#endif
#endif

template<typename T>
class ThreadLocal
{
public:
    ThreadLocal() : ThreadLocal(std::function<T*()>([]{return new T();})) {}
    // To support assignment in declaration may be not supported (ios/android clang) if inherit ctor is used.
    // FIXME: what if T == std::function<T*()>?
    ThreadLocal(const T& t) : ThreadLocal(std::function<T*()>([=]{ return new T(t);})) {}
    // TODO: move constructor?
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
    }
    ~ThreadLocal() {
#ifdef USE_PTHREAD
        pthread_key_delete(key_);
#endif
#ifdef USE_FLS
        FlsFree(index_);
#endif
    }
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
    T* operator&() const { return get();}
    T* operator->() const { return get(); }
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
