#pragma once

#include <vector>
#include <utility>
#include <tuple>
#include <type_traits>

template<typename _t>
_t get_rand(const std::vector<_t>& x)
{
    return x.size() == 0 ? _t() : x[get_rand((int)x.size())];
}

template<typename _t, typename _int_t>
void pop_erase(std::vector<_t>& list, _int_t* index)
{
    list.at((size_t)*index) = std::move(list.back());
    list.pop_back();
    *index -= 1;
}

template<typename _t, typename... _i>
std::vector<_t> list(const _t& first, const _i&... others)
{
    return std::vector<_t>{ first, others... };
}

template<typename _t, typename _f>
bool contains(const _t& list, const _f& value)
{
    return std::find(list.begin(), list.end(), value) != list.end();
}

template<typename _t>
float* f(_t& vec)
{
    return (float*)&vec;
}

template<auto val>
using constant = std::integral_constant<decltype(val), val>;

template<typename... _t>
using t = std::tuple<_t...>;

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& pair) const {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};

namespace std
{
    template <class T1, class T2>
    struct hash<pair<T1, T2>> {
        std::size_t operator() (const std::pair<T1, T2>& pair) const {
            return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
        }
    };

    template <>
    struct hash<ivec2> {
        size_t operator()(const ivec2& x) const {
            return (size_t(x.x) << 32) + size_t(x.y);
        }
    };
}


template <typename _map>
bool map_compare(const _map& lhs, const _map& rhs)
{
    // https://stackoverflow.com/a/8473603/6772365

    // No predicate needed because there is operator== for pairs already.
    return lhs.size() == rhs.size()
        && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}
