#pragma once

// this is a helper class for easy if statements between these const char*s
// most of the time they should be in the string table, so this is just an int comparison
struct comparable_const_char
{
	const char* str;

	comparable_const_char() : str(nullptr) {}
	comparable_const_char(const char* str) : str(str) {}
	operator const char* () const { return str; }

	bool operator==(const char* s) { return str == s || strcmp(str, s) == 0; }
	bool operator!=(const char* s) { return !operator==(s); }
};

template<>
struct std::hash<comparable_const_char>
{
	std::size_t operator()(const comparable_const_char& s) const noexcept
	{
		return std::hash<std::string>{}(s.str); // does this make a copy?
	}
};