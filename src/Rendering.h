#pragma once

// if sdl is a wrapper around os calls, then hiding it is stupid
// keep it simple!

#include "Event.h"
#include "Common.h"
#include <string>
#include <cmath>
#include <tuple>
#include <array>

// renderer ties this to opengl
// should split this into an ext and seperate the imgui and window
// but you cannot mix and match so its a replacement of the renderer...
// this is why hiding the impl is nice, but whatever...

// all this could be hidden from the public interface
// but for now Ill leave it

// imgui is going to be the UI library for everything in the game
// so hard commit to tieing it up with the renderer

// could hide this in a cpp file
// probally the best thing to do would be to put all the _XXXXX functions into a cpp
// that way opengl is actually hidden

#include "util/error_check.h" // gives gl
#include "SDL2/SDL_surface.h"
#include "stb/load_image.h"

// I want to create the simplest graphics api that hides as much as possible away
// I only need simple Texture/Mesh/Shader, I dont even need materials
// user should be able to understand where the memory is without needing to know the specifics of each type
//   I like how cuda does it with host and device
// Each thing could be interfaces with a graphics objeect type, might make things more? or less confusing
// This should just be a wrapper, I dont want to cover everything, so expose underlying library

// I was using RAII, but that causes headaches with all the move constructors
// So here is how this works

// Constructor  ( create host memory / load from files )
// SendToDevice ( create device memory and copy over) [free host if static]
// Cleanup      ( destroy host and device memory if they exists)

// this allows these classes to be passed to functions without ref
// bc they are quite small, only store handles / pointers

// doesnt support reading from device, but just follow the pattern to do that. Need SendToHost, _InitOnHost, _UpdateOnHost
// or just require non static and have only SendToHost and UpdateOnHost
// now it does :)

// you cannot however, SendToDevice(), FreeHost(), SendToHost()
// Sending to host requires it is never freeed, and therefore not static

#ifndef STATIC_HOST
#	define DYNAMIC_HOST 0
#	define STATIC_HOST 1
#	define INHERIT_HOST 2
//#	define PICK_INHERIT isStatic == INHERIT_HOST ? IsStatic() : isStatic
#endif


struct IDeviceObject
{
	IDeviceObject(
		bool is_static
	)
		: m_static   (is_static)
		, m_outdated (true)
	{}

	virtual ~IDeviceObject() = default;

	void FreeHost()
	{
		assert_on_host();
		_FreeHost();
	}

	void FreeDevice()
	{
		assert_on_device();
		_FreeDevice();
		m_outdated = true;
	}

	void SendToDevice()
	{
		assert_on_host(); // always require at least something on host to be copied to device, no device only init

		if (OnDevice())
		{
			assert_not_static();
			_UpdateOnDevice();
		}

		else
		{
			_InitOnDevice();
			if (m_static) FreeHost();
		}

		m_outdated = false;
	}

	void SendToHost()
	{
		assert_on_host(); // need host memory
		assert_not_static();
		_UpdateFromDevice();
	}

	void Cleanup()
	{
		if (OnDevice()) _FreeDevice();
		if (OnHost())   _FreeHost();
	}

	bool IsStatic() const
	{
		return m_static;
	}

	bool Outdated() const
	{
		return m_outdated;
	}

	void MarkForUpdate()
	{
		m_outdated = true;
	}

// interface

public:
	virtual bool OnHost() const = 0; 
	virtual bool OnDevice() const = 0;
	virtual int  DeviceHandle() const = 0;      // underlying opengl handle, useful for custom calls

protected:
	virtual void _FreeHost() = 0;               // delete host memory
	virtual void _FreeDevice() = 0;             // delete device memory
	virtual void _InitOnDevice() = 0;           // create device memory
	virtual void _UpdateOnDevice() = 0;         // update device memory
	virtual void _UpdateFromDevice() = 0;       // update host memory

private:
	bool m_static;
	bool m_outdated;

public:
	void assert_on_host()    const { assert(OnHost()   && "Device object has no data on host"); }
	void assert_on_device()  const { assert(OnDevice() && "Device object has no data on device"); }
	void assert_is_static()  const { assert( m_static  && "Device object is static"); }
	void assert_not_static() const { assert(!m_static  && "Device object is not static"); }

// move & copy

protected:
	void copy_base(const IDeviceObject* move) { m_static = move->m_static; m_outdated = move->m_outdated; }
};

// textures are very simple
// just two arrays, one on the cpu, one on the device
// can pass by value without much worry
struct Texture : IDeviceObject
{
public:

	// valid to cast an int representing a number of 8 bit channels to this enum to get r/rg/rgb/rgba
	enum Usage
	{
		uR = 1,   // 8 bit
		uRG,      // 16 bit
		uRGB,     // 24 bit
		uRGBA,    // 32 bit

		uDEPTH,   // 32 bit? unclear
		uSTENCIL, // 1 bit?

		// for passing data between gpu and cpu

		uINT_32,  // 128 bit
		uFLOAT_32 // 128 bit
	};
private:
	u8*          m_host            = nullptr; // deafult construction
	GLuint       m_device          = 0u;

	int          m_width           = 0;
	int          m_height          = 0; 
	int          m_channels        = 0;       // number of components
	int          m_bytesPerChannel = 0;       // bytes per component
	Usage        m_usage           = uR;

// public texture specific functions

public:
	int Width()           const { return m_width; } 
	int Height()          const { return m_height; } 
	int Channels()        const { return m_channels; }
	int BytesPerChannel() const { return m_bytesPerChannel; }
	Usage UsageType()     const { return m_usage; }
	u8* Pixels()          const { return (u8*)m_host; }
	int BufferSize()      const { return Width() * Height() * Channels() * BytesPerChannel(); }
	int Length()          const { return Width() * Height(); }

	vec2 Dimensions()     const { return vec2(Width(), Height()); }

	// reads from the host
	// only rgba up to Channels() belong to the pixel at (x, y)
	// only r -> Channels() = 1
	// only rg -> Channels() = 2
	// only rgb -> Channels() = 3
	// full rgba -> Channels() = 4

	// assumed non const At will be written to

	      Color& At(int x, int y)       { return At(Index32(x, y)); }
	const Color& At(int x, int y) const { return At(Index32(x, y)); }

		  Color& At(int index32)       { assert_on_host(); MarkForUpdate(); return *(Color*)(Pixels() + Index(index32)); }
	const Color& At(int index32) const { assert_on_host();                  return *(Color*)(Pixels() + Index(index32)); }

	int Index32(int x, int y) const { return x + y * m_width; }
	int Index  (int x, int y) const { return Index32(x, y) * m_channels * m_bytesPerChannel; }
	int Index  (int index32)  const { return index32 * m_channels * m_bytesPerChannel; }

	template<typename _t> const _t* At(int x, int y) const { return (const _t*)&At(x, y).as_u32; }
	template<typename _t>       _t* At(int x, int y)       { return (      _t*)&At(x, y).as_u32; }

	void ClearHost(Color color = Color(0, 0, 0, 0))
	{
		assert_on_host();
		memset(m_host, color.as_u32, BufferSize());

		MarkForUpdate();
	}

	void Resize(int width, int height)
	{
		assert_on_host();
		assert_not_static();

		if (m_width == width && m_height == height) return;

		m_width = width;
		m_height = height;

		void* newMemory = realloc(m_host, BufferSize());
		assert(newMemory && "failed to resize texture");
		m_host = (u8*)newMemory;

		MarkForUpdate();
	}

// interface

public:	
	bool OnHost()       const override { return m_host   != nullptr; }
	bool OnDevice()     const override { return m_device != 0u;      }
	int  DeviceHandle() const override { return m_device; } 
protected:
	void _FreeHost()
	{
		free(m_host);
		m_host = nullptr;
	}

	void _FreeDevice() override
	{
		gl(glDeleteTextures(1, &m_device));
		m_device = 0;
	}

	void _InitOnDevice() override
	{
		gl(glGenTextures(1, &m_device));
		gl(glBindTexture(GL_TEXTURE_2D, m_device));
		gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		gl(glTexImage2D(GL_TEXTURE_2D, 0, gl_iformat(), Width(), Height(), 0, gl_format(), gl_type(), Pixels()));
	}

	void _UpdateOnDevice() override
	{
		glBindTexture(GL_TEXTURE_2D, m_device);

		int w, h;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

		if (w != m_width || h != m_height)
		{
			gl(glTexImage2D(GL_TEXTURE_2D, 0, gl_iformat(), Width(), Height(), 0, gl_format(), gl_type(), Pixels()));
		}

		else
		{
			gl(glTextureSubImage2D(m_device, 0, 0, 0, Width(), Height(), gl_format(), gl_type(), Pixels()));
		}
	}

	void _UpdateFromDevice() override
	{
		// really slow find fix
		// is it the data or the point in time this function is getting called?

		gl(glGetTextureImage(m_device, 0, gl_format(), gl_type(), BufferSize(), Pixels()));
	}

// construction

public:
	Texture()
		: IDeviceObject (true)
	{}

	Texture(
		const std::string& path,
		bool isStatic = true
	)
		: IDeviceObject (isStatic)
	{
		auto [pixels, width, height, channels] = load_image(path);
		assert(channels > 0 && channels <= 4 && "Invalid RGBA channel count created by stb");
		init_texture_host_memory(pixels, width, height, (Usage)channels);
	}

	// creates pixel memory
	Texture(
		int width, int height, Usage usage,
		bool isStatic = true
	)
		: IDeviceObject (isStatic)
	{
		void* pixels = malloc(width * height * num_channels(usage) * bytes_per_channel(usage));
		init_texture_host_memory(pixels, width, height, usage);
	}

	Texture(
		int width, int height, Usage usage,
		void* pixels
	)
		: IDeviceObject (false) // false dont delete
	{
		init_texture_host_memory(pixels, width, height, usage);
	}

	~Texture() { Cleanup(); }

	Texture           (      Texture&& move) noexcept : IDeviceObject(move.IsStatic()) { move_into(std::move(move)); }
	Texture           (const Texture&  copy)          : IDeviceObject(copy.IsStatic()) { copy_into(copy); }
	Texture& operator=(      Texture&& move) noexcept                                  { return move_into(std::move(move)); }
	Texture& operator=(const Texture&  copy)                                           { return copy_into(copy); }

// move & copy

private:
	Texture& move_into(Texture&& move) noexcept
	{
		copy_base(&move);

		m_width           = move.m_width;
		m_height          = move.m_height;
		m_channels        = move.m_channels;
		m_bytesPerChannel = move.m_bytesPerChannel;
		m_usage           = move.m_usage;

		m_host            = move.m_host;
		m_device          = move.m_device;

		move.m_host = nullptr;
		move.m_device = 0;

		return *this;
	}

	Texture& copy_into(const Texture& copy)
	{
		copy_base(&copy);

		m_width           = copy.m_width;
		m_height          = copy.m_height;
		m_channels        = copy.m_channels;
		m_bytesPerChannel = copy.m_bytesPerChannel;
		m_usage           = copy.m_usage;

		m_device          = 0;

		if (copy.OnHost())
		{
			m_host = (u8*)malloc(BufferSize());
			memcpy(m_host, copy.m_host, BufferSize());
		}

		return *this;
	}

// helpers

private:
	void init_texture_host_memory(void* pixels, int w, int h, Usage usage)
	{
		assert(pixels && "Sprite failed to load");

		m_host = (u8*)pixels;
		m_width = w;
		m_height = h;
		m_usage = usage;
		m_channels = num_channels(usage);
		m_bytesPerChannel = bytes_per_channel(usage);
	}

	GLenum gl_format() const // doesnt need to be here
	{
		switch (m_usage)			
		{
			case uR:        return GL_RED;
			case uRG:       return GL_RG;
			case uRGB:      return GL_RGB;
			case uRGBA:     return GL_RGBA;
			case uDEPTH:    return GL_DEPTH_COMPONENT;
			case uSTENCIL:  return GL_STENCIL_INDEX;
			case uINT_32:   return GL_RGBA_INTEGER;
			case uFLOAT_32: return GL_RGBA; // this doesnt have to specify float???
		}

		assert(false);
		return -1;
	}

	GLenum gl_iformat() const
	{
		switch (m_usage)
		{
			case uR:        return GL_R8;
			case uRG:       return GL_RG8;
			case uRGB:      return GL_RGB8;
			case uRGBA:     return GL_RGBA8;
			case uDEPTH:    return GL_DEPTH_COMPONENT32;
			case uSTENCIL:  return GL_STENCIL_INDEX8;
			case uINT_32:   return GL_RGBA32I;
			case uFLOAT_32: return GL_RGBA32F;
		}

		assert(false);
		return -1;
	}

	GLenum gl_type() const
	{
		switch (m_usage)
		{
			case uR:        return GL_UNSIGNED_BYTE;
			case uRG:       return GL_UNSIGNED_BYTE;
			case uRGB:      return GL_UNSIGNED_BYTE;
			case uRGBA:     return GL_UNSIGNED_BYTE;
			case uDEPTH:    return GL_FLOAT;
			case uSTENCIL:  return GL_UNSIGNED_BYTE;
			case uINT_32:   return GL_INT;
			case uFLOAT_32: return GL_FLOAT;
		}

		assert(false);
		return -1;
	}

	int num_channels(Usage usage) const
	{
		switch (usage)
		{
			case uR:         return 1;
			case uRG:        return 2;
			case uRGB:       return 3;
			case uRGBA:      return 4;
			case uDEPTH:     return 1;
			case uSTENCIL:   return 1;
			case uINT_32:    return 4;
			case uFLOAT_32:  return 4;
		}

		assert(false);
		return -1;
	}

	int bytes_per_channel(Usage usage) const
	{
		switch (usage)
		{
			case uR:         return sizeof(u8);
			case uRG:        return sizeof(u8);
			case uRGB:       return sizeof(u8);
			case uRGBA:      return sizeof(u8);
			case uDEPTH:     return sizeof(f32);
			case uSTENCIL:   return sizeof(u8);
			case uINT_32:    return sizeof(u32);
			case uFLOAT_32:  return sizeof(f32);
		}

		assert(false);
		return -1;
	}
};

// target is a collection of textures that can get drawn onto
// functions much like mesh with buffers

struct Target : IDeviceObject
{
public:
	enum AttachmentName
	{
		aColor,
		aColor1,
		aColor2,
		aColor3,
		aColor4,
		aColor5,
		aDepth,
		aStencil
	};
private:
	using _attachments = std::unordered_map<AttachmentName, r<Texture>>;

	_attachments m_attachments;        // deafult construction
	GLuint       m_device       = 0;

	int          m_width        = 0;
	int          m_height       = 0;

	// public target specific functions

public:
	int NumberOfAttachments() const { return m_attachments.size(); }
	int Width()               const { return m_width; }
	int Height()              const { return m_height; }

	      r<Texture>& Get(AttachmentName name)       { return m_attachments.at(name); }
	const r<Texture>& Get(AttachmentName name) const { return m_attachments.at(name); }

	// instances a buffer
	void Add(AttachmentName name, const r<Texture>& texture)
	{
		assert(m_attachments.find(name) == m_attachments.end() && "Attachment already exists in target");
		// should put asserts for depth and spencil for what rgb values they need
		m_attachments.emplace(name, texture);
	}

	// creates an empty texture that is not static
	r<Texture> Add(AttachmentName name, int width, int height, Texture::Usage usage, bool isStatic = true)
	{
		r<Texture> texture = mkr<Texture>(width, height, usage, isStatic);
		Add(name, texture);

		return texture;
	}

	void Resize(int width, int height)
	{
		if (m_width == width && m_height == height) return;

		for (auto& [_, texture] : m_attachments) texture->Resize(width, height);
		m_width = width;
		m_height = height;

		MarkForUpdate();
	}

	void Use()
	{
		if (!OnDevice() || Outdated()) SendToDevice();
		gl(glBindFramebuffer(GL_FRAMEBUFFER, m_device));
	}

	static void UseDefault()
	{
		gl(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	static void Clear(Color color)
	{
		gl(glClearColor(color.rf(), color.gf(), color.bf(), color.af()));
		gl(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	// interface

public:

	// same concept as mesh, see comment there

	bool OnHost()       const override { return m_attachments.size() != 0; }
	bool OnDevice()     const override { return m_device != 0u; }
	int  DeviceHandle() const override { return m_device; }
protected:
	void _FreeHost() override
	{
		for (auto& [_, texture] : m_attachments) if (texture->OnHost() && texture->IsStatic()) texture->FreeHost();
	}

	void _FreeDevice() override
	{
		for (auto& [_, texture] : m_attachments) if (texture->OnDevice()) texture->FreeDevice();
		gl(glDeleteFramebuffers(1, &m_device));
		m_device = 0;
	}

	void _InitOnDevice() override
	{
		gl(glGenFramebuffers(1, &m_device));
		gl(glBindFramebuffer(GL_FRAMEBUFFER, m_device));

		std::vector<GLenum> toDraw;

		for (auto& [name, texture] : m_attachments)
		{
			if (name >= aColor) toDraw.push_back(gl_attachment(name));
			if (!texture->OnDevice()) texture->SendToDevice();
			gl(glFramebufferTexture(GL_FRAMEBUFFER, gl_attachment(name), texture->DeviceHandle(), 0));
		}

		gl(glDrawBuffers(toDraw.size(), toDraw.data()));

		GLint err = gl(glCheckFramebufferStatus(GL_FRAMEBUFFER));
		if (err != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("failed to create framebuffer\n");
		}
	}

	void _UpdateOnDevice() override
	{
		for (auto& [_, texture] : m_attachments) if (texture->OnHost()) texture->SendToDevice();
	}

	void _UpdateFromDevice() override
	{
		for (auto& [_, texture] : m_attachments) if (texture->OnDevice()) texture->SendToHost();
	}

	// construction

public:
	Target(
		bool isStatic = true
	)
		: IDeviceObject(isStatic)
	{}

	// add instancing constructor

	~Target() { Cleanup(); }

	Target           (      Target&& move) noexcept : IDeviceObject(move.IsStatic()) { move_into(std::move(move)); }
	Target           (const Target&  copy)          : IDeviceObject(copy.IsStatic()) { copy_into(copy); }
	Target& operator=(      Target&& move) noexcept                                  { return move_into(std::move(move)); }
	Target& operator=(const Target&  copy)                                           { return copy_into(copy); }

// move & copy

private:
	Target& move_into(Target&& move) noexcept
	{
		copy_base(&move);

		m_width           = move.m_width;
		m_height          = move.m_height;
		m_attachments     = std::move(move.m_attachments);
		m_device          = 0;

		move.m_device = 0;

		return *this;
	}

	// makes instances of buffers
	// not sure if this is the right behaviour

	Target& copy_into(const Target& copy)
	{
		copy_base(&copy);

		m_width           = copy.m_width;
		m_height          = copy.m_height;
		m_device          = 0;

		// copy the buffers, do not instance

		for (const auto& [name, texture] : m_attachments)
		{
			m_attachments.emplace(name, mkr<Texture>(*texture));
		}

		return *this;
	}

// helpers

private:
	GLenum gl_attachment(AttachmentName name) const // doesnt need to be here
	{
		switch (name)
		{
			case AttachmentName::aDepth:   return GL_DEPTH_ATTACHMENT;
			case AttachmentName::aStencil: return GL_STENCIL_ATTACHMENT;
			case AttachmentName::aColor:   return GL_COLOR_ATTACHMENT0;
			case AttachmentName::aColor1:  return GL_COLOR_ATTACHMENT1;
			case AttachmentName::aColor2:  return GL_COLOR_ATTACHMENT2;
			case AttachmentName::aColor3:  return GL_COLOR_ATTACHMENT3;
			case AttachmentName::aColor4:  return GL_COLOR_ATTACHMENT4;
			case AttachmentName::aColor5:  return GL_COLOR_ATTACHMENT5;
		}

		assert(false);
		return -1;
	}
};

// meshes are annoying
// can have many many differnt setups
// I am going to make the desision to spit each buffer into a seperate array so they can be appended
// this is useful for a GenerateNormals function, which would add some buffers

// so goal is to be able to add a buffer ONLY by typename
// if vert attribs are a map, not an array, then I will store the device buffers as such

// I dont like this thought because functions cannot know what other functions will change
// so two could try and add the same type of buffer without knowing
// and there are no names that work for everything, so aAttrib1, 2, 3, 4, 5 are needed
// maybe should just use strings?
// or have an order

// hmm maybe create a the buffer obj that the mesh seems to be a collection
// and then the mesh is just a map of these
// MeshSource is a map of attribs to shared_ptrs of buffers
// then a Mesh is just an instance of this, can swap out attribs as it likes

// add std::enable_shared_from_this
// add better support for length 0, right now it is a little unclear with malloc / realloc of size 0, should just set nullptr

struct Buffer : IDeviceObject
{
public:
	enum ElementType
	{
		_none, _u8, _u32, _f32
	};
private:
	void*        m_host     = nullptr; // deafult construction
	GLuint       m_device   = 0u;

	int          m_length   = 0;
	int          m_repeat   = 0;
	ElementType  m_type     = _none;

// public mesh specific functions

public:
	      void* Data()                   { return m_host; }
	const void* Data()             const { return m_host; }
	int         Length()           const { return m_length; }
	int         Repeat()           const { return m_repeat; }
	ElementType Type()             const { return m_type; }
	int         BytesPerElement()  const { return Repeat() * element_type_size(Type()); }
	int         Bytes()            const { return Length() * BytesPerElement(); }

	template<typename _t>       _t& Get(int i)       { assert_on_host(); return *(      _t*)m_host + i * BytesPerElement(); }
	template<typename _t> const _t& Get(int i) const { assert_on_host(); return *(const _t*)m_host + i * BytesPerElement(); }

	template<typename _t>
	void Set(const std::vector<_t>& data)
	{
		Set(data.size(), data.data());
	}

	// length is number of elements
	void Set(int length, const void* data)
	{
		// see below
		if (m_length > 0) assert_on_host();

		int size = length * BytesPerElement();
		
		if (m_length != length)
		{
			m_host = realloc(m_host, size);
			m_length = length;
		}

		memcpy(m_host, data, size);

		MarkForUpdate();
	}

	// length is number of elements
	// offset is the number of elements from the start
	// offset must be positive and adjacent or inside of the buffer
	// if the length + offset is larger than the buffer, it is realloced and appended to
	// data is left uninitalized if there are gaps between offsets
	void Append(int length, const void* data, int offset)
	{
		assert(offset >= 0 && "offset must be positive");

		// this is weird, could recreate on host if its not there...
		if (m_length > 0) assert_on_host();

		assert(offset >= 0 && offset <= Length() && "offset must be positive and adjacent or inside of the buffer");
		int size = length * BytesPerElement();
		int osize = size + offset * BytesPerElement();
		if (osize > Bytes())
		{
			m_host = realloc(m_host, osize);
			m_length = length + offset;
		}

		assert_on_host(); // see above, could put it here, tihs allows you to resize but not copy into...
		memcpy((char*)m_host + offset, data, size);

		MarkForUpdate();
	}

// interface

public:
	bool OnHost()       const override { return m_host   != nullptr; }
	bool OnDevice()     const override { return m_device != 0u;      }
	int  DeviceHandle() const override { return m_device; }
protected:
	void _FreeHost() override
	{
		free(m_host);
		m_host = nullptr;
	}

	void _FreeDevice() override
	{
		gl(glDeleteBuffers(1, &m_device));
		m_device = 0;
	}

	void _InitOnDevice() override
	{
		gl(glGenBuffers(1, &m_device));
		gl(glBindBuffer(GL_ARRAY_BUFFER, m_device)); // user can bind to what they want after using ::DeviceHandle
		gl(glNamedBufferData(m_device, Bytes(), Data(), gl_static()));
	}

	void _UpdateOnDevice() override
	{
		// on realloc, does old buffer need to be destroied?

		int size = 0;
		gl(glGetNamedBufferParameteriv(m_device, GL_BUFFER_SIZE, &size));
		if (Bytes() != size) { gl(glNamedBufferData(m_device, Bytes(), Data(), gl_static())); }
		else                 { gl(glNamedBufferSubData(m_device, 0, Bytes(), Data())); }
	}

	void _UpdateFromDevice() override
	{
		gl(glGetNamedBufferSubData(m_device, 0, Bytes(), Data()));
	}

// construction

public:
	Buffer()
		: IDeviceObject (true)
	{}

	Buffer(
		int length, int repeat, ElementType type,
		bool isStatic = true
	)
		: IDeviceObject (isStatic)
		, m_length      (length)
		, m_repeat      (repeat)
		, m_type        (type)
	{
		m_host = malloc(length * repeat * element_type_size(type));
	}

	~Buffer() { Cleanup(); }

	Buffer           (      Buffer&& move) noexcept : IDeviceObject(move.IsStatic()) { move_into(std::move(move)); }
	Buffer           (const Buffer&  copy)          : IDeviceObject(copy.IsStatic()) { copy_into(copy); }
	Buffer& operator=(      Buffer&& move) noexcept                                  { return move_into(std::move(move)); }
	Buffer& operator=(const Buffer&  copy)                                           { return copy_into(copy); }

// move & copy

private:
	Buffer& move_into(Buffer&& move) noexcept
	{
		copy_base(&move);

		m_length		  = move.m_length;
		m_repeat		  = move.m_repeat;
		m_type            = move.m_type;

		m_host  		  = move.m_host;
		m_device		  = move.m_device;

		move.m_host   = 0;
		move.m_device = 0;

		return *this;
	}

	// makes instances of buffers
	// not sure if this is the right behaviour

	Buffer& copy_into(const Buffer& copy)
	{
		copy_base(&copy);

		m_length          = copy.m_length;
		m_repeat          = copy.m_repeat;
		m_type            = copy.m_type;

		m_device          = 0;

		if (copy.OnHost())
		{
			m_host = (u8*)malloc(Bytes());
			memcpy(m_host, copy.m_host, Bytes());
		}

		return *this;
	}

// helpers

private:
	int element_type_size(ElementType type) const // doesnt need to be here
	{
		switch (type)
		{
			case _u8:  return sizeof(u8);
			case _u32: return sizeof(u32);
			case _f32: return sizeof(f32);
		}

		assert(false && "invalid buffer element type");
		return 0;
	}

	GLenum gl_static() const
	{
		return IsStatic() ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
	}
};

// tihs allows you to get the number of elements and element type from some glm types

template<typename _t>
std::pair<int, Buffer::ElementType> get_element_type_info()
{
	Buffer::ElementType type;
	int repeat = 1; // deafult

	constexpr bool isFloat = std::is_same<_t, float>::value;
	constexpr bool isByte  = std::is_same<_t,  char>::value || std::is_same<_t, unsigned char>::value; // or bool?
	constexpr bool isInt   = std::is_same<_t,   int>::value || std::is_same<_t, unsigned  int>::value;

	if constexpr (!isFloat && !isByte && !isInt)
	{
		// assume has a value_type and length() like glm

		constexpr bool _isFloat = std::is_same<typename _t::value_type, float>::value;
		constexpr bool _isByte  = std::is_same<typename _t::value_type,  char>::value || std::is_same<typename _t::value_type, unsigned char>::value; // or bool?
		constexpr bool _isInt   = std::is_same<typename _t::value_type,   int>::value || std::is_same<typename _t::value_type, unsigned  int>::value;

		if constexpr (_isFloat) { type = Buffer::_f32; }
		if constexpr (_isByte)  { type = Buffer::_u8;  }
		if constexpr (_isInt)   { type = Buffer::_u32; }

		repeat = sizeof(_t) / sizeof(_t::value_type);// _t::length();
	}

	if constexpr (isFloat) { type = Buffer::_f32; }
	if constexpr (isByte)  { type = Buffer::_u8;  }
	if constexpr (isInt)   { type = Buffer::_u32; }

	//assert(repeat <= 4 && "vec4 is max data allowed in one VA attrib");
	// attribs now auto expand to handle this

	return { repeat, type };
}

// mesh data is a list of buffer objects bound together
// by a vertex array
// attribs in shaders need to be in the order of AttribName

struct Mesh : IDeviceObject
{
public:
	enum AttribName
	{
		aPosition,
		aTextureCoord,

		aNormal,
		aTangent,
		aBiTangent,
		
		aColor,

		aCustom_a1,
		aCustom_a2,
		aCustom_a3,
		aCustom_a4,

		aCustom_b1,
		aCustom_b2,
		aCustom_b3,
		aCustom_b4,

		aCustom_c1,
		aCustom_c2,
		aCustom_c3,
		aCustom_c4,

		aIndexBuffer // the index buffer
	};

	enum Topology
	{
		tTriangles,
		tLines,
		tLoops
	};

	struct BufferInfo
	{
		int instancedStride;
		int offset;            // this is used if the buffer is larger than a vec4
		int repeat;            // so each buffer knows how many repeats (buffer with 5 repeats) first attrib has 4, next has 1
	};
private:
	using _buffers = std::unordered_map<AttribName, r<Buffer>>;
	using _info    = std::unordered_map<AttribName, BufferInfo>; // could change to a struct if there is other info
																 // right now this is just the instanced stride
	_buffers     m_buffers;            // deafult construction
	_info        m_info;
	GLuint       m_device   = 0;
	int          m_hasInstance = 0;
public:
	Topology     topology   = tTriangles;

// public mesh specific functions

public:
	int  NumberOfBuffers()     const { return m_buffers.size(); }
	bool HasInstancedBuffers() const { return m_hasInstance > 0; }

	      r<Buffer>&  Get    (AttribName name)       { MarkForUpdate(); return m_buffers.at(name); }
	      BufferInfo& GetInfo(AttribName name)       { MarkForUpdate(); return m_info.at(name); }
	const r<Buffer>&  Get    (AttribName name) const {                  return m_buffers.at(name); }
	const BufferInfo& GetInfo(AttribName name) const {                  return m_info.at(name); }

	Mesh& SetTopology(Topology topology)
	{
		this->topology = topology;
		return *this;
	}

	// set the instanced stride of a buffer
	// if set to 0, instancing is disabled
	// if any buffers are instanced, DrawInstanced is required to draw
	// an assert will fire if this is not done
	Mesh& SetInst(AttribName name, int instancedStride) 
	{
		if (instancedStride == 0) m_hasInstance -= GetInfo(name).instancedStride;
		else                      m_hasInstance += instancedStride;

		GetInfo(name).instancedStride = instancedStride; 
		
		return *this; 
	}

	// set the byte offset of an attribute
	// a single buffer can be bound to many attribs, this allows use of differnt parts of the data
	// for each attrib
	// max size for each attrib element is a vec4, so for a mat4, offsets are requires to use a single buffer
	Mesh& SetOffset(AttribName name, int offset) 
	{
		GetInfo(name).offset = offset;
		return *this; 
	}

	// instances a buffer
	// if buffer->Repeat() returns more than 4, the attribs past 'name' are also linked to this buffer
	// and their infos are set to reflect the offset inside each buffer element
	Mesh& Add(AttribName name, int instancedStride, const r<Buffer>& buffer)
	{
		assert(m_buffers.find(name) == m_buffers.end() && "Buffer already exists in mesh");
		assert(name != aIndexBuffer || (buffer->Type() == Buffer::_u32 && buffer->Repeat() == 1) && "index buffer must be of type 'int' with a repeat of 1.");
	
		// if repeat is larger than 4, then use the next attribs
		// add the same buffer ref, but change the offset and repeat of buffer info

		int repeat = buffer->Repeat();
		
		for (int i = 0; i < repeat; i += 4) // will always run once
		{
			BufferInfo info;
			info.instancedStride = instancedStride;
			info.offset = i * sizeof(f32);            // assuming that each element is a single p float, and split is every 4th
			info.repeat = min(4, repeat);

			m_buffers.emplace(AttribName( (int)name + i/4 ), buffer);
			m_info   .emplace(AttribName( (int)name + i/4 ), info);

			m_hasInstance += instancedStride;
		}
	
		MarkForUpdate();
		return *this;
	}

	// constructs a buffer and sets its data
	template<typename _t>
	Mesh& Add(AttribName name, int instancedStride, int isStatic, const std::vector<_t>& data)
	{
		auto [repeat, type] = get_element_type_info<_t>();
		r<Buffer> buffer = mkr<Buffer>(data.size(), repeat, type, isStatic == INHERIT_HOST ? IsStatic() : isStatic);
		buffer->Set(data.size(), data.data());
		return Add(name, instancedStride, buffer);
	}

// shorthand for simple configs (no instancing)

	template<typename _t>
	Mesh& Add(AttribName name, const std::vector<_t>& data)
	{
		return Add<_t>(name, 0, INHERIT_HOST, data);
	}

	template<typename _t>
	Mesh& Add(AttribName name, const r<Buffer>& buffer)
	{
		return Add<_t>(name, 0, buffer);
	}

	// setups a buffer without data
	template<typename _t>
	Mesh& Setup(AttribName name, int instancedStride = 0, int isStatic = INHERIT_HOST)
	{
		Add<_t>(name, instancedStride, isStatic, {});
		return *this;
	}

	// gl() calls cause nullptr exception

	void Draw(Topology drawType = Topology::tTriangles)
	{
		assert(!HasInstancedBuffers() && "mesh has instanced buffers, need to call DrawInstanced");
		bool hasIndex = SendBindAndReturnHasIndex();
		if (hasIndex) { /*gl(*/glDrawElements(gl_drawtype(drawType),    m_buffers.at(aIndexBuffer)->Length(), GL_UNSIGNED_INT, nullptr)/*)*/; }
		else          { /*gl(*/glDrawArrays  (gl_drawtype(drawType), 0, m_buffers.at(aPosition)   ->Length())/*)*/; }
	}

	void DrawInstanced(int numberOfInstances, Topology drawType = Topology::tTriangles)
	{
		assert(HasInstancedBuffers() && "mesh has no instanced buffers, need to call Draw");
		bool hasIndex = SendBindAndReturnHasIndex();
		if (hasIndex) { /*gl(*/glDrawElementsInstanced(gl_drawtype(drawType),    m_buffers.at(aIndexBuffer)->Length(), GL_UNSIGNED_INT, nullptr, numberOfInstances)/*)*/; }
		else          { /*gl(*/glDrawArraysInstanced  (gl_drawtype(drawType), 0, m_buffers.at(aPosition)   ->Length(),                           numberOfInstances)/*)*/; }
	}

private:

	// helper for Draw functions
	bool SendBindAndReturnHasIndex()
	{
		if (!OnDevice() || Outdated()) SendToDevice();
		gl(glBindVertexArray(m_device));
		return m_buffers.find(aIndexBuffer) != m_buffers.end();
	}

// interface

public:

	// All these functions will skip buffers
	// that dont meet assertion requirements
	
	// OnHost gives no information about underlying buffers
	// only if it contains any buffers
	// this is to meet interface: SendToDevice frees static hosts
	// then onhost would return false for when the mesh would try and call free host on itself
	// little hackey
	// might want to return true always just to hammer the point home that this func isnt what its ment to be

	bool OnHost()       const override { return m_buffers.size() != 0; }
	bool OnDevice()     const override { return m_device != 0u; }
	int  DeviceHandle() const override { return m_device; } 
protected:
	void _FreeHost() override
	{
		for (auto& [_, buffer] : m_buffers) if (buffer->OnHost() && buffer->IsStatic()) buffer->FreeHost();
	}

	void _FreeDevice() override
	{
		for (auto& [_, buffer] : m_buffers) if (buffer->OnDevice()) buffer->FreeDevice();
		gl(glDeleteVertexArrays(1, &m_device));
		m_device = 0;
	}

	void _InitOnDevice() override
	{
		gl(glGenVertexArrays(1, &m_device));
		gl(glBindVertexArray(m_device));

		for (auto& [attrib, buffer] : m_buffers)
		{
 			if (buffer->OnHost() && !buffer->OnDevice()) 
			{
				buffer->SendToDevice();
			}
			
			if (attrib == aIndexBuffer)
			{
				gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->DeviceHandle())); // set for vba
				continue;
			}

			gl(glBindBuffer(GL_ARRAY_BUFFER, buffer->DeviceHandle()));

			BufferInfo& info = GetInfo(attrib);

			gl(glEnableVertexAttribArray(attrib));
			gl(glVertexAttribPointer(attrib, info.repeat, gl_format(buffer->Type()), GL_FALSE, buffer->BytesPerElement(), (void*)info.offset));
			gl(glVertexAttribDivisor(attrib, info.instancedStride));
		}
	}

	void _UpdateOnDevice() override
	{
		for (auto& [_, buffer] : m_buffers) if (buffer->OnHost()) buffer->SendToDevice();
	}

	void _UpdateFromDevice() override
	{
		for (auto& [_, buffer] : m_buffers) if (buffer->OnDevice()) buffer->SendToHost();
	}

// construction

public:
	Mesh(
		bool isStatic = true
	)
		: IDeviceObject (isStatic)
	{}

	// add instancing constructor
	
	~Mesh() { Cleanup(); }

	Mesh           (      Mesh&& move) noexcept : IDeviceObject(move.IsStatic()) { move_into(std::move(move)); }
	Mesh           (const Mesh&  copy)          : IDeviceObject(copy.IsStatic()) { copy_into(copy); }
	Mesh& operator=(      Mesh&& move) noexcept                                  { return move_into(std::move(move)); }
	Mesh& operator=(const Mesh&  copy)                                           { return copy_into(copy); }

	Mesh Copy()
	{
		return *this;
	}

// move & copy

private:
	Mesh& move_into(Mesh&& move) noexcept
	{
		copy_base(&move);

		topology          = move.topology;
		m_hasInstance     = move.m_hasInstance;
		m_buffers         = std::move(move.m_buffers);
		m_info            = std::move(move.m_info);
		m_device          = 0;

		move.m_device = 0;

		return *this;
	}

	// makes instances of buffers
	// not sure if this is the right behaviour

	Mesh& copy_into(const Mesh& copy)
	{
		copy_base(&copy);

		topology          = copy.topology;
		m_hasInstance     = copy.m_hasInstance;
		m_info            = copy.m_info;
		m_device          = 0;

		std::unordered_map<r<Buffer>, std::vector<AttribName>> singleBuffers;
		
		// actually copy the data of the buffers, not just the references
		// instancing can happen somewhere else, not in the copy constructor, this should make an independent copy
		for (const auto& [name, buffer] : copy.m_buffers)
		{
			singleBuffers[buffer].push_back(name);
		}

		for (const auto& [buffer, names] : singleBuffers)
		{
			buffer->assert_on_host(); // needs to have some data on host for this to make sense
			r<Buffer> copyBuffer = mkr<Buffer>(*buffer);

			for (const AttribName& name : names)
			{
				m_buffers.emplace(name, copyBuffer);
			}
		}

		return *this;
	}

// helpers

private:
	GLenum gl_format(Buffer::ElementType type) const // doesnt need to be here
	{
		switch (type)
		{
			case Buffer::_u8:  return GL_UNSIGNED_BYTE;
			case Buffer::_u32: return GL_INT;
			case Buffer::_f32: return GL_FLOAT;
		}

		assert(false);
		return -1;
	}
	
	GLenum gl_static() const
	{
		return IsStatic() ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
	}

	GLenum gl_drawtype(Topology drawType) const
	{
		switch (drawType)
		{
			case Topology::tTriangles: return GL_TRIANGLES;
			case Topology::tLines:     return GL_LINES;
			case Topology::tLoops:     return GL_LINE_LOOP;
		}

		assert(false);
		return -1;
	}
};

// this turned out to be very similar to a mesh hmmm

struct ShaderProgram : IDeviceObject
{
public:
	enum ShaderName
	{
		sVertex,
		sFragment,
		sGeometry,
		sCompute
	};
private:
	using _buffers = std::unordered_map<ShaderName, Buffer>; // buffer for string storage (host only)
	
	_buffers     m_buffers;            // deafult construction
	GLuint       m_device   = 0;

	int          m_slot     = 0;       // number of active texture slots

// public mesh specific functions

public:
	int NumberOfShaders()       const { return m_buffers.size(); }
	int NumberOfBoundTextures() const { return m_slot; }

	// appends source onto a shader
	void Add(ShaderName name, const char* str)
	{
		bool new_text = m_buffers.find(name) == m_buffers.end();
		if (new_text) m_buffers.emplace(name, Buffer(1, 1, Buffer::_u8));
		Buffer& buffer = m_buffers.at(name);
		buffer.Append(strlen(str) + 1, str, buffer.Length() - 1); // +1 copies null and -1 removes it
	}

	void Add(ShaderName name, const std::string& str)
	{
		Add(name, str.c_str());
	}

	void Use()
	{
		if (!OnDevice()) SendToDevice();
		gl(glUseProgram(m_device));
	}

	// these could be const if texture didnt get sent here
	// think of another way to do this...

	void Set(const std::string& name, const   int& x) { gl(glUniform1iv       (gl_location(name), 1,            (  int*)  &x)); }
	void Set(const std::string& name, const   u32& x) { gl(glUniform1uiv      (gl_location(name), 1,            (  u32*)  &x)); }
	void Set(const std::string& name, const   f32& x) { gl(glUniform1fv       (gl_location(name), 1,            (float*)  &x)); }
	void Set(const std::string& name, const fvec1& x) { gl(glUniform1fv       (gl_location(name), 1,            (float*)  &x)); }
	void Set(const std::string& name, const fvec2& x) { gl(glUniform2fv       (gl_location(name), 1,            (float*)  &x)); }
	void Set(const std::string& name, const fvec3& x) { gl(glUniform3fv       (gl_location(name), 1,            (float*)  &x)); }
	void Set(const std::string& name, const fvec4& x) { gl(glUniform4fv       (gl_location(name), 1,            (float*)  &x)); }
	void Set(const std::string& name, const ivec1& x) { gl(glUniform1iv       (gl_location(name), 1,            (  int*)  &x)); }
	void Set(const std::string& name, const ivec2& x) { gl(glUniform2iv       (gl_location(name), 1,            (  int*)  &x)); }
	void Set(const std::string& name, const ivec3& x) { gl(glUniform3iv       (gl_location(name), 1,            (  int*)  &x)); }
	void Set(const std::string& name, const ivec4& x) { gl(glUniform4iv       (gl_location(name), 1,            (  int*)  &x)); }
	void Set(const std::string& name, const fmat2& x) { gl(glUniformMatrix2fv (gl_location(name), 1,  GL_FALSE, (float*)  &x)); }
	void Set(const std::string& name, const fmat3& x) { gl(glUniformMatrix3fv (gl_location(name), 1,  GL_FALSE, (float*)  &x)); }
	void Set(const std::string& name, const fmat4& x) { gl(glUniformMatrix4fv (gl_location(name), 1,  GL_FALSE, (float*)  &x)); }

	void Set(const std::string& name, Texture& texture)
	{
		if (!texture.OnDevice() || texture.Outdated()) texture.SendToDevice();
		gl(glBindTexture(GL_TEXTURE_2D, texture.DeviceHandle())); // need texture usage
		gl(glActiveTexture(gl_slot()));
		gl(glUniform1i(gl_location(name), m_slot));
	}

// interface

public:

	bool OnHost()       const override { return m_buffers.size() != 0; }
	bool OnDevice()     const override { return m_device != 0u; }
	int  DeviceHandle() const override { return m_device; } 
protected:
	void _FreeHost() override
	{
		for (auto& [_, buffer] : m_buffers) if (buffer.OnHost()) buffer.FreeHost();
	}

	void _FreeDevice() override
	{
		gl(glDeleteProgram(m_device));
		m_device = 0;
	}

	void _InitOnDevice() override
	{
		m_device = gl(glCreateProgram());

		for (auto& [name, buffer] : m_buffers)
		{
			const char* source = (char*)buffer.Data(); // only reason this is a var is for odd error when it's an arg

			GLuint shader = gl(glCreateShader(gl_type(name)));
			glShaderSource(shader, 1, &source, nullptr);
			gl(glCompileShader(shader));

			// error check
			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
				glDeleteShader(shader); // if soft error this can be removed
				printf("Failed to compile shader: %s\n", (char*)infoLog.data());
				assert(false && "Failed to compile shader"); // maybe soft error
			}

			gl(glAttachShader(m_device, shader));
			gl(glDeleteShader(shader));
		}

		gl(glLinkProgram(m_device));
	}

	void _UpdateOnDevice() override
	{
		gl(glDeleteProgram(m_device));
		_InitOnDevice();
	}

	void _UpdateFromDevice() override
	{
		assert(false && "Updating from device has no impl for shaders");
		// maybe copy the source back? I dont think this will ever be useful
	}

// construction

public:
	ShaderProgram(
		bool isStatic = true
	)
		: IDeviceObject (isStatic)
	{}

	~ShaderProgram() { Cleanup(); }

	ShaderProgram           (      ShaderProgram&& move) noexcept : IDeviceObject(move.IsStatic()) { move_into(std::move(move)); }
	ShaderProgram           (const ShaderProgram&  copy)          : IDeviceObject(copy.IsStatic()) { copy_into(copy); }
	ShaderProgram& operator=(      ShaderProgram&& move) noexcept                                  { return move_into(std::move(move)); }
	ShaderProgram& operator=(const ShaderProgram&  copy)                                           { return copy_into(copy); }

// move & copy

private:
	ShaderProgram& move_into(ShaderProgram&& move)
	{
		copy_base(&move);

		m_slot            = move.m_slot;
		m_buffers         = std::move(move.m_buffers);
		m_device          = 0;

		move.m_device = 0;

		return *this;
	}

	// makes instances of buffers
	// not sure if this is the right behaviour

	ShaderProgram& copy_into(const ShaderProgram& copy)
	{
		copy_base(&copy);

		m_slot            = copy.m_slot;
		m_buffers         = copy.m_buffers;
		m_device          = 0;

		return *this;
	}

// helpers

private:
	GLenum gl_type(ShaderName type) const // doesnt need to be here
	{
		switch (type)			
		{
			case ShaderName::sVertex:   return GL_VERTEX_SHADER;
			case ShaderName::sFragment: return GL_FRAGMENT_SHADER;
			case ShaderName::sGeometry: return GL_GEOMETRY_SHADER;
			case ShaderName::sCompute:  return GL_COMPUTE_SHADER;
		}

		assert(false);
		return -1;
	}

	GLint gl_slot() const
	{
		return GL_TEXTURE0 + m_slot;
	}

	GLint gl_location(const std::string& name) const
	{
		GLint location = gl(glGetUniformLocation(m_device, name.c_str()));
		return location;
	}
};

// Need a simple material system
// needs particles that change a uniform every frame
//

// each mesh needs a material
// a model could be a list of meshes and materials

// a sprite renderer takes a 

// 

//
// end device objects s
//



//struct Material
//{
//private:
//	struct Prop
//	{
//		std::string name;
//		virtual void SetOnDevice(ShaderProgram* program) = 0;
//	};
//
//	template<typename _t>
//	struct PropValue : Prop
//	{
//		_t value;
//		void SetOnDevice(ShaderProgram* program)
//		{
//			program->Set(name, value);
//		}
//	};
//
//	std::unordered_map<std::string, Prop*> m_properties;
//
//public:
//	template<typename _t>
//	void Set(const std::string& name, const _t& value)
//	{
//		auto itr = m_properties.find(name);
//		if (itr == m_properties.end())
//		{
//			m_properties[name] = new PropValue();
//		}
//
//		m_properties[name].value = value;
//	}
//
//	void SendToDevice(ShaderProgram* program)
//	{
//		for (auto& [name, prop] : m_properties)
//		{
//			prop->SetOnDevice(program);
//		}
//	}
//};
