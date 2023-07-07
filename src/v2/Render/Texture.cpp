#include "v2/Render/Texture.h"

#include "v2/Render/platform_host.h"
#include "v2/Render/platform_gl.h"

//
//	Host Texture
//

TextureView HostTexture::View() const
{
	return m_view;
}

TextureView HostTexture::Take()
{
	TextureView view = m_view;
	m_view = {};
	
	return view;
}

void HostTexture::Resize(const TextureLayout& newLayout)
{
	_texture_host_realloc(&m_view, newLayout);
}

HostTexture::HostTexture(const TextureLayout& layout)
{
	m_view = _texture_host_alloc(layout);
}

HostTexture::HostTexture(const TextureView& view)
{
	m_view = view;
}

HostTexture::~HostTexture()
{
	_texture_host_free(&m_view);
}

HostTexture::HostTexture(const HostTexture& copy)
{
	_copy_in(copy);
}

HostTexture::HostTexture(HostTexture&& move) noexcept
{
	_move_in(std::forward<HostTexture>(move));
}

HostTexture& HostTexture::operator=(const HostTexture& copy)
{
	_copy_in(copy);
	return *this;
}

HostTexture& HostTexture::operator=(HostTexture&& move) noexcept
{
	_move_in(std::forward<HostTexture>(move));
	return *this;
}

void HostTexture::_copy_in(const HostTexture& copy)
{
	TextureLayout layout = copy.View().layout;
	
	_texture_host_free(&m_view);
	m_view = _texture_host_alloc(layout);
	_texture_host_copy(&m_view, layout);
}

void HostTexture::_move_in(HostTexture&& move)
{
	_texture_host_free(&m_view);
	m_view = std::exchange(move.m_view, {});
}

//
//	Device Texture
//

void DeviceTexture::Resize(const TextureLayout& newLayout)
{
	TextureView empty(nullptr, newLayout);
	_texture_device_realloc(&m_handle, empty);
}

TextureHandle DeviceTexture::View() const
{
	return m_handle;
}

TextureHandle DeviceTexture::Take()
{
	TextureHandle handle = m_handle;
	m_handle = {};

	return handle;
}

void DeviceTexture::CopyToDevice(const TextureView& view)
{
	if (m_handle.handle)
		_texture_device_realloc(&m_handle, view);
	else
		m_handle = _texture_device_alloc(view);
}

void DeviceTexture::CopyToHost(HostTexture* host)
{
	if (host->View().layout != m_handle.layout)
		host->Resize(m_handle.layout);

	_texture_device_copy_to_host(m_handle, host->View());
}

void DeviceTexture::SetFilter(TextureFilter filter)
{
	m_filter = filter;
	_texture_device_filter_set(m_handle, m_filter);
}

TextureFilter DeviceTexture::GetFilter()
{
	// assumes this is the same on the GPU and this flag
	return m_filter;
}

DeviceTexture::DeviceTexture()
	: m_filter (TextureFilterPixel)
{}

DeviceTexture::DeviceTexture(const TextureLayout& layout)
	: m_filter (TextureFilterPixel)
{
	m_handle = _texture_device_alloc(layout);
}

DeviceTexture::DeviceTexture(const TextureView& host)
	: m_filter (TextureFilterPixel)
{
	m_handle = _texture_device_alloc(host);
}

DeviceTexture::DeviceTexture(const TextureHandle& handle)
	: m_filter (TextureFilterPixel)
	, m_handle (handle)
{}

DeviceTexture::~DeviceTexture()
{
	_texture_device_free(&m_handle);
}

DeviceTexture::DeviceTexture(const DeviceTexture& copy)
{
	_copy_in(copy);
}

DeviceTexture::DeviceTexture(DeviceTexture&& move) noexcept
{
	_move_in(std::forward<DeviceTexture>(move));
}

DeviceTexture& DeviceTexture::operator=(const DeviceTexture& copy)
{
	_copy_in(copy);
	return *this;
}

DeviceTexture& DeviceTexture::operator=(DeviceTexture&& move) noexcept
{
	_move_in(std::forward<DeviceTexture>(move));
	return *this;
}

void DeviceTexture::_copy_in(const DeviceTexture& copy)
{
	_texture_device_free(&m_handle);
	
	// memory is lost
	// could copy memory but that needs opengl 4.3+, so should just use normal FBO process
	m_handle = _texture_device_alloc(copy.View().layout);
	m_filter = copy.m_filter;
}

void DeviceTexture::_move_in(DeviceTexture&& move)
{
	_texture_device_free(&m_handle);

	m_handle = std::exchange(move.m_handle, {});
	m_filter = std::exchange(move.m_filter, {});
}

//
//	Shared Texture
//

bool v2Texture::HasData() const
{
	return m_host.View().buffer || m_device.View().handle;
}

void v2Texture::Resize(const TextureLayout& newLayout)
{
	m_host.Resize(newLayout);
	m_device.CopyToDevice(m_host.View());
	device = m_device.View();
}

void v2Texture::SendToDevice()
{
	m_device.CopyToDevice(m_host.View());
	device = m_device.View();
}

void v2Texture::SendToHost()
{
	m_device.CopyToHost(&m_host);
	host = m_host.View();
}

void v2Texture::FreeHost()
{
	m_host = {};
}

void v2Texture::FreeDevice()
{
	m_device = {};
}

v2Texture::v2Texture()
{}

v2Texture::v2Texture(const TextureLayout& layout, Access access)
{
	bool createHost = access == AccessHost || access == AccessHostDevice;
	bool createDevice = access == AccessDevice || access == AccessHostDevice;

	if (createHost)
		m_host = HostTexture(layout);
	
	if(createDevice)
		m_device = DeviceTexture(layout);
}

v2Texture::v2Texture(const TextureView& view, Access access)
	: m_host (view)
{
	_sync(access);
}

v2Texture::v2Texture(const TextureHandle& handle, Access access)
	: m_device (handle)
{
	_sync(access);
}

v2Texture::v2Texture(const TextureView& view, const TextureHandle& handle)
	: m_host	 (view)
	, m_device   (handle)
	, host       (view)
	, device     (handle)
{
	if (view.layout != handle.layout)
		throw nullptr; // invalid layouts
}

v2Texture::v2Texture(const v2Texture& copy)
{
	_copy_in(copy);
}

v2Texture::v2Texture(v2Texture&& move) noexcept
{
	_move_in(std::forward<v2Texture>(move));
}

v2Texture& v2Texture::operator=(const v2Texture& copy)
{
	_copy_in(copy);
	return *this;
}

v2Texture& v2Texture::operator=(v2Texture&& move) noexcept
{
	_move_in(std::forward<v2Texture>(move));
	return *this;
}

void v2Texture::_copy_in(const v2Texture& copy)
{
	m_host = copy.m_host;
	m_device = copy.m_device;

	host = m_host.View();
	device = m_device.View();
}

void v2Texture::_move_in(v2Texture&& move)
{
	m_host = std::exchange(move.m_host, {});
	m_device = std::exchange(move.m_device, {});

	host = m_host.View();
	device = m_device.View();
}

void v2Texture::_sync(Access access)
{
	bool noHost = !m_host.View().buffer;
	bool noDevice = !m_device.View().handle;

	switch (access)
	{
		// if the host has not been created, copy device memory
		// assume device exists if host doesn't
		case AccessHost:
			if (noHost) SendToHost();
			FreeDevice();
			break;

		// if the device has not been created, copy host memory
		// assume host exists if device doesn't
		case AccessDevice:
			if (noDevice) SendToDevice();
			FreeHost();
			break;

		// sync memory between host and device, but don't free
		case AccessHostDevice:
			if (noHost) SendToHost();
			if (noDevice) SendToDevice();
			break;
	}
}

//
//	Texture
//

// put in another file

#include "io/ImageFromDisk.h"

v2Texture wTextureCreate(const char* filepath, Access access)
{
	TextureView view = v2LoadTextureFromFile(filepath);
	return v2Texture(view, access);
}


TextureView v2LoadTextureFromFile(const char* filepath)
{
    RawImageData raw = io_LoadImageFromFile(filepath);

    TextureLayout layout;
    layout.width = raw.width;
    layout.height = raw.height;
    layout.depth = 1;
    layout.format = (TextureFormat)raw.channels;

    return TextureView((u8*)raw.buffer, layout);
}

void v2FreeTexture(TextureView* view)
{
    _texture_host_free(view);
}
