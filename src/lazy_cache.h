#pragma once
#include <windows.h>

namespace waskin {
// Function-local static guards use implicit TLS, which XP cannot initialize
// reliably for LoadLibrary DLLs. Keep lazy construction explicitly synchronized.
template<class T> class LazyCache {
    volatile LONG state_{}; // 0: empty, 1: constructing, 2: published
    T* value_{};
public:
    constexpr LazyCache()=default;
    LazyCache(const LazyCache&)=delete;
    LazyCache& operator=(const LazyCache&)=delete;
    ~LazyCache(){delete value_;}
    template<class Factory> const T& Get(Factory factory) {
        for(;;) {
            const LONG state=InterlockedCompareExchange(&state_,1,0);
            if(state==2)return *value_;
            if(state==0) {
                try {
                    value_=new T(factory());
                    InterlockedExchange(&state_,2);
                    return *value_;
                }catch(...){InterlockedExchange(&state_,0);throw;}
            }
            Sleep(1);
        }
    }
};
}
