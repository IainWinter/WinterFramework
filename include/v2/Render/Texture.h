#pragma once

#include "v2/Render/TextureView.h"
#include "ext/AssetItem.h"

//  Takes ownership of a TextureView
//
class HostTexture
{
public:
	// Reallocate texture in RAM, if the number of bytes is the same
	// the texture will not be reallocated
	void Resize(const TextureLayout& newLayout);

	TextureView View();
	TextureViewConst View() const;

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

//	Defines a buffer in device VRAM
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
	void CopyToDevice(const TextureViewConst& view);

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
	DeviceTexture(const TextureViewConst& host);

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
class SharedTexture
{
public:
	bool HasData() const;

	// Resize the texture in VRAM and host RAM
	void Resize(const TextureLayout& newLayout);

	TextureView View();
	TextureViewConst View() const;

	int GetHandle() const;

	void SendToDevice();
	void SendToHost();
	void FreeHost();
	void FreeDevice();

public:
	SharedTexture();
	SharedTexture(const TextureLayout& layout, TextureAccess access);

	// Take ownership of a TextureView
	// Create a host texture depending on the TextureAccess
	SharedTexture(const TextureView& view, TextureAccess access);

	// Take ownership of a TextureHandle
	// Create a host texture depending on the TextureAccess
	SharedTexture(const TextureHandle& handle, TextureAccess access);

	// Take ownership of a TextureView and TextureHandle
	// their layouts must be identical
	SharedTexture(const TextureView& view, const TextureHandle& handle);

public:
	SharedTexture(const SharedTexture& copy);
	SharedTexture(SharedTexture&& move) noexcept;

	SharedTexture& operator=(const SharedTexture& copy);
	SharedTexture& operator=(SharedTexture&& move) noexcept;

private:
	void _copy_in(const SharedTexture& copy);
	void _move_in(SharedTexture&& move);

	void _sync(TextureAccess access);

private:
	HostTexture m_host;
	DeviceTexture m_device;

	// A flag that is set whenever a non-const host operation is called.
	// When set, the device should be updated before using this texture in a shader
	bool m_outdated;
};

//	A wrapper around an asset reference to a SharedTexture.
//	Create through wCreateTexture to allow for custom loaders
//
class Texture_New
{
public:
	a<SharedTexture> instance;

public:
	Texture_New() = default;

	Texture_New(const a<SharedTexture>& inst);
	Texture_New(SharedTexture&& move);
};

Texture_New wCreateTexture(const TextureLayout& layout, TextureAccess access);
Texture_New wCreateTexture(const char* filepath, TextureAccess access);