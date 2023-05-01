#pragma once

#include "v2/Render/RenderOptions.h"
#include "util/Color.h"
#include <type_traits>

//	Defines a buffer in host RAM with a defined layout from TextureLayout
//  Does not own memory
//
template<bool _is_const>
struct TextureView_t
{
	using buffer_t = typename std::conditional<_is_const, const u8*, u8*>::type;

	TextureView_t() 
		: m_buffer        (nullptr)
		, m_layout        ()
		, m_bytesPerIndex (0)
	{}

	TextureView_t(const TextureLayout& layout)
		: m_buffer        (nullptr)
		, m_layout        (layout)
		, m_bytesPerIndex (layout.NumberOfBytesPerPixel())
	{}
	
	TextureView_t(buffer_t buffer, const TextureLayout& layout)
		: m_buffer        (buffer)
		, m_layout        (layout)
		, m_bytesPerIndex (layout.NumberOfBytesPerPixel())
	{}

	TextureView_t(const TextureView_t<false>& nonConst)
		: m_buffer        (nonConst.GetBytes())
		, m_layout        (nonConst.GetLayout())
		, m_bytesPerIndex (nonConst.GetLayout().NumberOfBytesPerPixel())
	{}

	bool HasData() const { return m_buffer != nullptr; }
	const TextureLayout& GetLayout() const { return m_layout; }

	buffer_t GetBytes() const { return m_buffer; }

	// Return a Color from an index. Layout defines which accessors in Color are valid
	// For example, an fRG texture will have Color::r and Color::g valid, but Color::b and Color::a will be the next pixel's data
	const Color& At(int index1D) const 
	{
		return *_access_raw(index1D);
	}

	Color& At(int index1D)
	{
        static_assert(!_is_const, "Texture view is const");
        
		return *_access_raw(index1D);
	}

    TextureView_t& Set(int index1D, const Color& color)
	{
        static_assert(!_is_const, "Texture view is const");
        
		At(index1D) = color;
		return *this;
	}

private:
	Color* _access_raw(int index1D) const
	{
		return (Color*)(m_buffer + index1D * m_bytesPerIndex);
	}

private:
	buffer_t m_buffer;
	TextureLayout m_layout;

	int m_bytesPerIndex = 0;
};

using TextureView	   = TextureView_t<false>;
using TextureViewConst = TextureView_t<true>;

//  Holds a reference to a device handle of a texture
//  Does not own handle
//
class TextureHandle
{
public:
	TextureHandle()
		: m_handle (0)
		, m_type   (0)
	{}
	
	TextureHandle(u32 handle, u32 type, const TextureLayout& layout)
		: m_handle (handle)
		, m_type   (type)
		, m_layout (layout)
	{}

	bool HasData() const { return m_handle != 0; }
	const TextureLayout& GetLayout() const { return m_layout; }

	u32 GetHandle() const { return m_handle; }
	u32 GetTarget() const { return m_type; }
	
private:
	u32 m_handle;
	u32 m_type;
	TextureLayout m_layout;
};
