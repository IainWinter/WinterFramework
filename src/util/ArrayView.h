#pragma once

// could remove if template AsList to take a container type
#include <vector>

// holds a reference to a contiguous array that can be iterated over
// in range based for-loops
template<typename _t>
struct ArrayView
{
private:
    _t* m_begin;
    _t* m_end;

public:
    ArrayView()
        : m_begin (nullptr)
        , m_end   (nullptr)
    {}

    ArrayView(_t* begin, _t* end)
        : m_begin (begin)
        , m_end   (end)
    {}

    template<typename _std_contiguous>
    ArrayView(const _std_contiguous& arr)
        : m_begin ((_t*)arr.data())
        , m_end   ((_t*)arr.data() + arr.size())
    {}

          _t& at(size_t i)       { return m_begin[i]; }
    const _t& at(size_t i) const { return m_begin[i]; }

          _t& operator[](size_t i)       { return at(i); }
    const _t& operator[](size_t i) const { return at(i); }

    size_t size() const { return std::distance(m_begin, m_end); }

    _t* begin() { return m_begin; }
    _t* end()   { return m_end; }

    const _t* begin() const { return m_begin; }
    const _t* end()   const { return m_end; }

          _t* data()       { return m_begin; }
    const _t* data() const { return m_begin; }

    std::vector<_t> AsList() const { return std::vector<_t>(m_begin, m_end); }
};
