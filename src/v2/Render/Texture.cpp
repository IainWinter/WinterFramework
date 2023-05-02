#include "v2/Render/Texture.h"

#include "v2/Render/platform_host.h"
#include "v2/Render/platform_gl.h"

//
//	Host Texture
//

TextureView HostTexture::View()
{
	return m_view;
}

TextureViewConst HostTexture::View() const
{
	return TextureViewConst(m_view);
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
	const TextureLayout& layout = copy.View().GetLayout();
	
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

void DeviceTexture::CopyToDevice(const TextureViewConst& view)
{
	if (m_handle.HasData())
		_texture_device_realloc(&m_handle, view);
	else
		m_handle = _texture_device_alloc(view);
}

void DeviceTexture::CopyToHost(HostTexture* host)
{
	if (host->View().GetLayout() != m_handle.GetLayout())
		host->Resize(m_handle.GetLayout());

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
	: m_filter (fPixel)
{}

DeviceTexture::DeviceTexture(const TextureLayout& layout)
	: m_filter (fPixel)
{
	m_handle = _texture_device_alloc(layout);
}

DeviceTexture::DeviceTexture(const TextureViewConst& host)
	: m_filter (fPixel)
{
	m_handle = _texture_device_alloc(host);
}

DeviceTexture::DeviceTexture(const TextureHandle& handle)
	: m_filter (fPixel)
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
	m_handle = _texture_device_alloc(copy.View().GetLayout());
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

bool SharedTexture::HasData() const
{
	return m_host.View().HasData() || m_device.View().HasData();
}

void SharedTexture::Resize(const TextureLayout& newLayout)
{
	m_outdated = false;
	m_host.Resize(newLayout);
	m_device.CopyToDevice(m_host.View());
}

TextureView SharedTexture::View()
{
	m_outdated = true;
	return m_host.View();
}

TextureViewConst SharedTexture::View() const
{
	return m_host.View();
}

int SharedTexture::GetHandle() const
{
	return m_device.View().GetHandle();
}

void SharedTexture::SendToDevice()
{
	m_device.CopyToDevice(m_host.View());
}

void SharedTexture::SendToHost()
{
	m_device.CopyToHost(&m_host);
}

void SharedTexture::FreeHost()
{
	m_host = {};
}

void SharedTexture::FreeDevice()
{
	m_device = {};
}

SharedTexture::SharedTexture()
	: m_outdated (false)
{}

SharedTexture::SharedTexture(const TextureLayout& layout, TextureAccess access)
	: m_outdated (false)
{
	bool createHost = access == aHost || access == aHostDevice;
	bool createDevice = access == aDevice || access == aHostDevice;

	if (createHost)
		m_host = HostTexture(layout);
	
	if(createDevice)
		m_device = DeviceTexture(layout);
}

SharedTexture::SharedTexture(const TextureView& view, TextureAccess access)
	: m_outdated (false)
	, m_host	 (view)
{
	_sync(access);
}

SharedTexture::SharedTexture(const TextureHandle& handle, TextureAccess access)
	: m_outdated (false)
	, m_device   (handle)
{
	_sync(access);
}

SharedTexture::SharedTexture(const TextureView& view, const TextureHandle& handle)
	: m_outdated (false)
	, m_host	 (view)
	, m_device   (handle)
{
	if (view.GetLayout() != handle.GetLayout())
	{
		throw nullptr; // invalid layouts
	}
}

SharedTexture::SharedTexture(const SharedTexture& copy)
{
	_copy_in(copy);
}

SharedTexture::SharedTexture(SharedTexture&& move) noexcept
{
	_move_in(std::forward<SharedTexture>(move));
}

SharedTexture& SharedTexture::operator=(const SharedTexture& copy)
{
	_copy_in(copy);
	return *this;
}

SharedTexture& SharedTexture::operator=(SharedTexture&& move) noexcept
{
	_move_in(std::forward<SharedTexture>(move));
	return *this;
}

void SharedTexture::_copy_in(const SharedTexture& copy)
{
	m_outdated = true;
	m_host = copy.m_host;
	m_device = copy.m_device;
}

void SharedTexture::_move_in(SharedTexture&& move)
{
	m_outdated = move.m_outdated;
	m_host = std::exchange(move.m_host, {});
	m_device = std::exchange(move.m_device, {});
}

void SharedTexture::_sync(TextureAccess access)
{
	bool noHost = !m_host.View().HasData();
	bool noDevice = !m_device.View().HasData();

	switch (access)
	{
		// if the host has not been created, copy device memory
		// assume device exists if host doesn't
		case aHost:
			if (noHost) SendToHost();
			FreeDevice();
			break;

		// if the device has not been created, copy host memory
		// assume host exists if device doesn't
		case aDevice:
			if (noDevice) SendToDevice();
			FreeHost();
			break;

		// sync memory between host and device, but don't free
		case aHostDevice:
			if (noHost) SendToHost();
			if (noDevice) SendToDevice();
			break;
	}
}

//
//	Texture
//

Texture_New::Texture_New(SharedTexture&& move)
	: instance (mkr<SharedTexture>(std::move(move)))
{}

Texture_New::Texture_New(const a<SharedTexture>& inst)
	: instance (inst)
{}

Texture_New wCreateTexture(const TextureLayout& layout, TextureAccess access)
{
	return Texture_New(mkr<SharedTexture>(layout, access));
}

// put in another file

#include "io/ImageFromDisk.h"

Texture_New wCreateTexture(const char* filepath, TextureAccess access)
{
	TextureView view = wCreateTexture(filepath);
	return Texture_New(mkr<SharedTexture>(view, access));
}

TextureView wCreateTexture(const char* filepath)
{
	RawImageData raw = io_LoadImageFromFile(filepath);

	TextureLayout layout;
	layout.width = raw.width;
	layout.height = raw.height;
	layout.depth = 1;
	layout.format = (TextureFormat)raw.channels;

	return TextureView((u8*)raw.buffer, layout);
}
