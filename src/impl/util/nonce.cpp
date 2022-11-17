#include "nonce.h"

std::unordered_map<void*, std::string> nonce_cache;

std::string nonce(int length, const char* prefix)
{
	std::stringstream ss;
	ss << prefix;

	for (int i = 0; i < length - 2; i++)
	{
		ss << char(97 + get_rand(26));
	}

	return ss.str();
}

const char* nonce_cached(void* instance, int length, const char* prefix)
{
	auto itr = nonce_cache.find(instance);
	if (itr == nonce_cache.end())
	{
		itr = nonce_cache.emplace(instance, nonce(length, prefix)).first;
	}

	return itr->second.c_str();
}