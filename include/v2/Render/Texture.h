#pragma once

#include "v2/Render/TextureView.h"
#include "v2/Render/Access.h"

//  Takes ownership of a TextureView
//
class HostTexture
{
public:
	// Reallocate texture in RAM, if the number of bytes is the same
	// the texture will not be reallocated
	void Resize(const TextureLayout& newLayout);

	TextureView View() const;

	// Take ownership of the memory
	TextureView Take();

public:
	HostTexture() = default;
	
	// Allocate memory for a layout
	HostTexture(const TextureLayout& layout);
	
	// Take ownership of memory from a view
	HostTexture(const TextureView& view);

	// Free memory
	~HostTexture();

public:
	HostTexture(const HostTexture& copy);
	HostTexture(HostTexture&& move) noexcept;

	HostTexture& operator=(const HostTexture& copy);
	HostTexture& operator=(HostTexture&& move) noexcept;

private:
	void _copy_in(const HostTexture& copy);
	void _move_in(HostTexture&& move);

private:
	TextureView m_view;
};

//	Takes ownership of a TextureHandle
//
class DeviceTexture
{
public:
	// Reallocate texture in VRAM, does not retain old pixel information
	// Use UpdatePixels to copy in new host data
	void Resize(const TextureLayout& newLayout);
	
	TextureHandle View() const;

	// Take ownership of handle
	TextureHandle Take();
	
	// Copy a host texture into this device texture.
	// If the dimensions are not the same, Resize will be called
	void CopyToDevice(const TextureView& view);

	// Copy the device texture into a specified host texture.
	// If the layouts are not the same, the host will be realloced
	void CopyToHost(HostTexture* host);

	void SetFilter(TextureFilter filter);
	TextureFilter GetFilter();

public:
	// Does not allocate anything on the device
	DeviceTexture();
	
	// Allocate memory on the device for a layout
	DeviceTexture(const TextureLayout& layout);
	
	// Allocate memory on the device for a layout, and fill it with data from the host
	DeviceTexture(const TextureView& host);

	// Take ownership of a TextureHandle
	DeviceTexture(const TextureHandle& handle);

	// Destroy the texture on the device
	~DeviceTexture();

public:
	DeviceTexture(const DeviceTexture& copy);
	DeviceTexture(DeviceTexture&& move) noexcept;

	DeviceTexture& operator=(const DeviceTexture& copy);
	DeviceTexture& operator=(DeviceTexture&& move) noexcept;

private:
	void _copy_in(const DeviceTexture& copy);
	void _move_in(DeviceTexture&& move);

private:
	TextureHandle m_handle;
	
	// settings that don't effect layout of data
	TextureFilter m_filter;
};

//	Texture with a host and device side
//
class v2Texture
{
public:
	bool HasData() const;

	// Resize the texture in VRAM and host RAM
	void Resize(const TextureLayout& newLayout);

	void SendToDevice();
	void SendToHost();
	void FreeHost();
	void FreeDevice();

public:
	v2Texture();

	// Create host and or device textures depending on access
	v2Texture(const TextureLayout& layout, Access access);

	// Take ownership of a TextureView
	// Create a device texture depending on access
	v2Texture(const TextureView& view, Access access);

	// Take ownership of a TextureHandle
	// Create a host texture depending on access
	v2Texture(const TextureHandle& handle, Access access);

	// Take ownership of a TextureView and TextureHandle
	// their layouts must be identical
	v2Texture(const TextureView& view, const TextureHandle& handle);

public:
	v2Texture(const v2Texture& copy);
	v2Texture(v2Texture&& move) noexcept;

	v2Texture& operator=(const v2Texture& copy);
	v2Texture& operator=(v2Texture&& move) noexcept;

private:
	void _copy_in(const v2Texture& copy);
	void _move_in(v2Texture&& move);

	void _sync(Access access);

	// read only accessors
public:
	TextureView host;
	TextureHandle device;

private:
	HostTexture m_host;
	DeviceTexture m_device;
};

//
//	Loading Textures
//

v2Texture wTextureCreate(const char* filepath, Access access);

// this leaks memory
TextureView wTextureLoadView(const char* filepath);
