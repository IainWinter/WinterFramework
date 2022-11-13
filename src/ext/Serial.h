#pragma once

#include "entt/meta/meta.hpp"
#include "entt/meta/factory.hpp"
#include "entt/meta/container.hpp"

#include <string>
#include <istream>
#include <ostream>

using namespace entt::literals;

class SerialWriter
{
private:
    std::ostream& out;
    bool binary;

public:
    SerialWriter(std::ostream& out, bool binary);

    template<typename _t>
    void put(const _t& c)
    {
        if (binary && !std::is_same<_t, char>::value)
        {
            const char* b = (const char*)&c;
            const char* e = b + sizeof(_t);

            while(b != e)
            { 
                out << *b;
                b++;
            }
        }

        else
        {
            out << c;
        }
    }

    virtual void Write(entt::meta_any instance) = 0;
};

class SerialReader
{
private:
    std::istream& in;
    bool binary;

public:
    SerialReader(std::istream& in, bool binary);

    template<typename _t>
    _t pop()
    {
        _t c = _t();

        if (binary && !std::is_same<_t, char>::value)
        {
            char* b = (char*)&c;
            char* e = b + sizeof(_t);

            while (b != e)
            { 
                in >> *b;
                b++;
            }
        }

        else
        {
            in >> c;
        }

        return c;
    }

    virtual void Read(entt::meta_any& instance) = 0;
};

class BinaryWriter : public SerialWriter
{
public:
    BinaryWriter(std::ostream& out);

    void Write(entt::meta_any instance) override;
};

class JsonWriter : public SerialWriter
{
public:
    JsonWriter(std::ostream& out);

    void Write(entt::meta_any instance) override;
};

class BinaryReader : public SerialReader
{
public:
    BinaryReader(std::istream& in);

    void Read(entt::meta_any& instance) override;
};

//
//
//class SerialWriter
//{
//private:
//    std::ostream& out;
//    bool binary;
//    
//public:
//    SerialWriter(std::ostream& out, bool binary);
//    
//    template<typename _t>
//    void put(const _t& c)
//    {
//        if (binary && !std::is_same<_t, char>::value)
//        {
//            WriteBytes((const char*)&c, sizeof(_t));
//        }
//
//        else
//        {
//            out << c;
//        }
//    }
//    
//    virtual void put_class_begin() {}
//    virtual void put_array_begin() {}
//    virtual void put_bytes_begin() {}
//    virtual void put_class_end()   {}
//    virtual void put_array_end()   {}
//    virtual void put_bytes_end()   {}
//    virtual void put_class_delim() {}
//    virtual void put_array_delim() {}
//    
//    virtual void WriteName(const char* name) {}
//    virtual void WriteLength(size_t length) {}
//
//    void WriteClass(entt::meta_any instance);
//    void WriteArray(entt::meta_any instance);
//    void WriteBytes(const char* bytes, size_t length);
//    
//    // Write any class or array
//    void Write(entt::meta_any instance, size_t length_override = -1);
//};
//
//class SerialReader
//{
//private:
//    std::istream& in;
//    bool binary;
//    int place = 0;
//
//public:
//    SerialReader(std::istream& in, bool binary);
//    
//    template<typename _t>
//    _t pop()
//    {
//        _t c;
//
//        if (binary && !std::is_same<_t, char>::value)
//        {
//            ReadBytes((char*)&c);
//        }
//
//        else if constexpr (std::is_integral<_t>::value || std::is_floating_point<_t>::value) // should just test for << op
//        {
//            in >> c;
//
//            place += sizeof(_t);
//        }
//
//        return c;
//    }
//        
//    virtual void pop_class_begin() {}
//    virtual void pop_array_begin() {}
//    virtual void pop_bytes_begin() {}
//    virtual void pop_class_end()   {}
//    virtual void pop_array_end()   {}
//    virtual void pop_bytes_end()   {}
//    virtual void pop_class_delim() {}
//    virtual void pop_array_delim() {}
//
//    virtual void ReadName(const char* name) {}
//    virtual size_t ReadLength() { return 0; }
//
//    void ReadClass(entt::meta_any& instance);
//    void ReadArray(entt::meta_any& instance);
//
//    // you own char*, delete with delete[]
//    char* ReadBytes();
//    
//    // Read any class or array
//    void ReadAny(entt::meta_any& instance, size_t length_override = -1);
//
//    template<typename _t>
//    _t Read()
//    {
//        entt::meta_any any = _t();
//        ReadAny(any);
//        return any.cast<_t>();
//    }
//};
//
//class JsonWriter : public SerialWriter
//{
//public:
//    JsonWriter(std::ostream& out)
//        : SerialWriter(out, false)
//    {}
//
//    void put_class_begin() override { put('{'); }
//    void put_array_begin() override { put('['); }
//    void put_bytes_begin() override { put('"'); }
//    void put_class_end()   override { put('}'); }
//    void put_array_end()   override { put(']'); }
//    void put_bytes_end()   override { put('"'); }
//    void put_class_delim() override { put(','); }
//    void put_array_delim() override { put(','); }
//
//    void WriteName(const char* name) override
//    {
//        WriteBytes(name, strlen(name));
//        put(':');
//    }
//
//    void WriteLength(size_t length) override
//    {
//        // do nothing
//    }
//};
//
//class BinaryWriter : public SerialWriter
//{
//public:
//    BinaryWriter(std::ostream& out)
//        : SerialWriter(out, true)
//    {}
//
//    void WriteLength(size_t length) override
//    {
//        put(length);
//    }
//};
//
//class BinaryReader : public SerialReader
//{
//public:
//    BinaryReader(std::istream& in)
//        : SerialReader(in, true)
//    {}
//
//    size_t ReadLength() override
//    {
//        return Read<size_t>();
//    }
//};
//

//template<typename _t> const _t& ref(const void* p) { return *(const _t*)p; }
//template<typename _t>       _t& ref(      void* p) { return *(      _t*)p; }
//
//void RegisterStdTypes();
//
//void WriteString(SerialWriter* writer, const void* data);
//void  ReadString(SerialReader* reader, void* data);