# ThreadLocal
portable and implemention configurable `c++11` like thread local

## WHY
`c++11` **thread_local** is not available for vs2013, apple clang

## Tested Compilers
VS2013, VS2015, gcc>=4.7, clang 3.4, Apple clang

## Build

ThreadLocal.cpp contains test code

- `g++/clang++` `c++11` thread_local: `(clan)g++ -std=c++11 -DTEST_TLS ThreadLocal.cpp`
- `g++/clang++` pthread implemention: `(clan)g++ -std=c++11 -DTEST_TLS ThreadLocal.cpp -DUSE_STD_THREAD_LOCAL=0`
- apple clang (pthread implemention): `clang++ -std=c++11 -DTEST_TLS ThreadLocal.cpp`
- vs2015 `c++11` thread_local: `cl /EHsc -DTEST_TLS ThreadLocal.cpp`
- vs2015 FLS implemention: `cl /EHsc -DTEST_TLS ThreadLocal.cpp -DUSE_STD_THREAD_LOCAL=0`
- vs2013 (FLS implemention): `cl /EHsc -DTEST_TLS ThreadLocal.cpp`
- mingw `c++11` thread_local: `g++ -std=c++11 -DTEST_TLS ThreadLocal.cpp`
- mingw FLS implemention: `g++ -std=c++11 -DTEST_TLS ThreadLocal.cpp -D_WIN32_WINNT=0x0600 -DUSE_STD_THREAD_LOCAL=0`
- mingw pthread implemention: `g++ -std=c++11 -DTEST_TLS ThreadLocal.cpp -DUSE_STD_THREAD_LOCAL=0`