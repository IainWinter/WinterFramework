#pragma once

// Adds a string name to a class
template<typename _t>
class Named
{
    const char* m_name = "unnamed";
    
public:
    _t* SetName(const char* name)
    {
        m_name = name;
        return (_t*)this;
    }
    
    const char* GetName() const
    {
        return m_name;
    }
};
