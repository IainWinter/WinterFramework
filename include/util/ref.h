#pragma once

#include <memory>

template<typename _t> 
using r = std::shared_ptr<_t>;

template<typename _t>
using wr = std::weak_ptr<_t>;

template<typename _t, typename... _args> 
r<_t> mkr(_args&&... args)
{ 
	return std::make_shared<_t>(std::forward<_args>(args)...); 
}

template<typename _t>
r<_t> ref(_t&& move)
{
	return std::make_shared<_t>(std::forward<_t>(move));
}

template<typename _t>
r<_t> ref(const _t& copy)
{
	return std::make_shared<_t>(copy);
}