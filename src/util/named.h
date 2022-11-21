#pragma once

#include <string>

// Adds a string name to a class
template<typename _t>
class Named
{
    std::string m_name = "unnamed";
    
public:
    _t* SetName(const std::string& name)
    {
        m_name = name;
        return (_t*)this;
    }
    
    const char* GetName() const
    {
        return m_name.c_str();
    }
};
