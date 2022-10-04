#pragma once

#include <vector>

template<typename _r, typename _t>
struct range_vector
{
private:
	struct range
	{
		_r length;
		_t value;
	};

	std::vector<range> m_vec;

public:

	void insert(const _r& length, const _t& value)
	{
		m_vec.push_back({ length, value });
	}

	_t& at(const _r& x)
	{
		_r c = _r(0);

		for (range& i : m_vec)
		{
			if (x < c + i.length)
			{
				return i.value;
			}

			c += i.length;
		}

		// could throw out of range, but returning the back is closer to the problems this solves

		return m_vec.back().value;
	}

	auto begin() { return m_vec.begin(); }
	auto end()   { return m_vec.end(); }
	auto size()  { return m_vec.size(); }
	auto front() { return m_vec.front(); }
	auto back()  { return m_vec.back(); }
};