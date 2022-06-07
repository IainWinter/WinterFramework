#pragma once

#include <utility>

using u32 = unsigned int;
using u8 = unsigned char;
using f32 = float;

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& pair) const {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};