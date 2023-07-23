#pragma once

// This file should hold some helper classes
// and functions for memory management

// A fixed array which has a lifetime using RAII
template<typename _t>
class v2Array 
{
public:
	v2Array() {
		m_size = 0;
		m_data = nullptr;
	}

	v2Array(int size) {
		m_size = size;
		m_data = new _t[m_size];
	}

	~v2Array() {
		delete[] m_data;
	}

	v2Array(const v2Array& other) {
		copy_from(other);
	}

	v2Array(v2Array&& other) noexcept {
		move_from(std::forward<v2Array>(other));
	}

	v2Array& operator=(const v2Array& other) {
		copy_from(other);
		return *this;
	}

	v2Array& operator=(v2Array&& other) noexcept {
		move_from(std::forward<v2Array>(other));
		return *this;
	}

	_t& operator[](int index) {
		return m_data[index];
	}

	const _t& operator[](int index) const {
		return m_data[index];
	}

	int size() const {
		return m_size;
	}

private:
	void copy_from(const v2Array& other) {
		m_size = other.size;
		m_data = new _t[m_size];
		for (int i = 0; i < m_size; i++)
			m_data[i] = other[i];
	}

	void move_from(v2Array&& other) {
		m_size = other.m_size;
		m_data = other.m_data;

		other.m_size = 0;
		other.m_data = nullptr;
	}

private:
	_t* m_data;
	int m_size;
};