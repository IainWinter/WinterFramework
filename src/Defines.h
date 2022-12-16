#pragma once

#include <utility>

#define wPI 3.1415926535f
#define w2PI wPI * 2.f

#define S1(x) #x
#define S2(x) S1(x)
#define __LOCATION __FILE__ " : " S2(__LINE__)

using u32 = unsigned int;
using u8  = unsigned char;
using f32 = float;

using Order = void*;

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& pair) const {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};