#pragma once

#include "Event.h"
#include "Common.h"
#include <string>
#include <cmath>
#include <tuple>
#include <array>

#include "util/error_check.h" // gives gl
#include "SDL2/SDL_surface.h"

// I want to create the simplest graphics api that hides as much as possible away
// I only need simple Texture/Mesh/Shader, I dont even need materials
// user should be able to understand where the memory is without needing to know the specifics of each type
//   I like how cuda does it with host and device
// Each thing could be interfaces with a graphics objeect type, might make things more? or less confusing
// This should just be a wrapper, I dont want to cover everything, so expose underlying library

// Constructor  ( create host memory / load from files )
// SendToDevice ( create device memory and copy over) [free host if static]
// Cleanup      ( destroy host and device memory if they exists)

// you cannot: SendToDevice(), FreeHost(), SendToHost()
// Sending to host requires it is never freeed, and therefore not static

// ended up using RAII, using shared pointers for most things is really what you want to instance

#ifndef STATIC_HOST
#	define DYNAMIC_HOST 0
#	define STATIC_HOST 1
#	define INHERIT_HOST 2
#	define PICK_INHERIT isStatic == INHERIT_HOST ? IsStatic() : isStatic
#endif

struct IDeviceObject
{
	IDeviceObject(bool isStatic);

	void FreeHost();
	void FreeDevice();
	void SendToDevice();
	void SendToHost();
	void Cleanup();

	bool IsStatic() const;
	bool Outdated() const;

	void MarkForUpdate();

// interface

public:
	virtual ~IDeviceObject() = default;

	virtual bool OnHost()       const = 0; 
	virtual bool OnDevice()     const = 0;
	virtual int  DeviceHandle() const = 0;      // underlying opengl handle, useful for custom calls

protected:
	virtual void _FreeHost()         = 0;       // delete host memory
	virtual void _FreeDevice()       = 0;       // delete device memory
	virtual void _InitOnDevice()     = 0;       // create device memory
	virtual void _UpdateOnDevice()   = 0;       // update device memory
	virtual void _UpdateFromDevice() = 0;       // update host memory

private:
	bool m_static;
	bool m_outdated;

public:
	void assert_on_host()    const;
	void assert_on_device()  const;
	void assert_is_static()  const;
	void assert_not_static() const;

// move & copy

protected:
	void copy_base(const IDeviceObject* move);
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
	int   Width()           const;
	int   Height()          const;
	int   Channels()        const;
	int   BytesPerChannel() const;
	Usage UsageType()       const;
	u8*   Pixels()          const;
	int   BufferSize()      const;
	int   Length()          const;
	vec2  Dimensions()      const;

	// reads from the host
	// only rgba up to Channels() belong to the pixel at (x, y)
	// only r -> Channels() = 1
	// only rg -> Channels() = 2
	// only rgb -> Channels() = 3
	// full rgba -> Channels() = 4

	// assumed non const At will be written to so marks the texture as outdated

	      Color& At(int x, int y);
	const Color& At(int x, int y) const;

	// !! need to add assert for invalid index !!!

		  Color& At(int index32);
	const Color& At(int index32) const;

	int Index32(int x, int y) const;
	int Index  (int x, int y) const;
	int Index  (int index32)  const;

	template<typename _t> const _t* At(int x, int y) const { return (const _t*)&At(x, y).as_u32; }
	template<typename _t>       _t* At(int x, int y)       { return (      _t*)&At(x, y).as_u32; }

	void ClearHost(Color color = Color(0, 0, 0, 0));
	void Resize(int width, int height);

// interface

public:	
	bool OnHost()       const override;
	bool OnDevice()     const override;
	int  DeviceHandle() const override;
protected:
	void _FreeHost()         override;
	void _FreeDevice()       override;
	void _InitOnDevice()     override;
	void _UpdateOnDevice()   override;
	void _UpdateFromDevice() override;

// construction

public:
	// by default is static
	Texture();
	Texture(const std::string& path, bool isStatic = true);
	Texture(int width, int height, Usage usage, bool isStatic = true);
	Texture(int width, int height, Usage usage, void* pixels);
	~Texture();

	Texture(Texture&& move) noexcept;
	Texture(const Texture& copy);
	Texture& operator=(Texture&& move) noexcept;
	Texture& operator=(const Texture& copy);

// move & copy

private:
	Texture& move_into(Texture&& move) noexcept;
	Texture& copy_into(const Texture& copy);

// helpers

private:
	void init_texture_host_memory(void* pixels, int w, int h, Usage usage);
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
	int NumberOfAttachments() const;
	int Width()               const;
	int Height()              const;

	      r<Texture>& Get(AttachmentName name);
	const r<Texture>& Get(AttachmentName name) const;

	// instances a texture
	void Add(AttachmentName name, const r<Texture>& texture);

	// creates an empty texture
	r<Texture> Add(AttachmentName name, int width, int height, Texture::Usage usage, int isStatic = INHERIT_HOST);

	void Resize(int width, int height);

	void Use();

	static void UseDefault();
	static void Clear(Color color);

// interface

public:
	bool OnHost()       const override;
	bool OnDevice()     const override;
	int  DeviceHandle() const override;
protected:
	void _FreeHost()         override;
	void _FreeDevice()       override;
	void _InitOnDevice()     override;
	void _UpdateOnDevice()   override;
	void _UpdateFromDevice() override;

// construction

public:
	Target(bool isStatic = true);
	~Target();

	Target(Target&& move) noexcept;
	Target(const Target& copy);
	Target& operator=(Target&& move) noexcept;
	Target& operator=(const Target& copy);

// move & copy

private:
	Target& move_into(Target&& move) noexcept;
	Target& copy_into(const Target& copy);
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
	using _data = std::vector<char>;

	_data        m_host;
	GLuint       m_device   = 0u;

	int          m_length   = 0; // need length after host is freeed
	int          m_repeat   = 0;
	ElementType  m_type     = _none;

// public buffer specific functions

public:
	      void* Data();
	const void* Data()             const;
	int         Length()           const;
	int         Repeat()           const;
	ElementType Type()             const;
	int         BytesPerElement()  const;
	int         Bytes()            const;

	template<typename _t>       _t& Get(int i)       { assert_on_host(); return *(      _t*)m_host.at(i * BytesPerElement()); }
	template<typename _t> const _t& Get(int i) const { assert_on_host(); return *(const _t*)m_host.at(i * BytesPerElement()); }

	void SetBytes  (int byteCount, const void* data);
	void PushBytes (int byteCount, const void* data);
	void EraseBytes(int byteIndex, int byteCount);
	void PopBytes  (int byteCount);

	void Set  (int elementCount, const void* elements);
	void Push (int elementCount, const void* elements);
	void Erase(int elementIndex, int elementCount = 1);
	void Pop  (int elementCount = 1);

	template<typename _t> void Set (const std::vector<_t>& data) { SetBytes ((int)(data.size() * sizeof(_t)), data.data()); }
	template<typename _t> void Push(const std::vector<_t>& data) { PushBytes((int)(data.size() * sizeof(_t)), data.data()); }

// interface

public:
	bool OnHost()       const override;
	bool OnDevice()     const override;
	int  DeviceHandle() const override;
protected:
	void _FreeHost()         override;
	void _FreeDevice()       override;
	void _InitOnDevice()     override;
	void _UpdateOnDevice()   override;
	void _UpdateFromDevice() override;

// construction

public:
	Buffer(bool isStatic = true);
	Buffer(int length, int repeat, ElementType type, bool isStatic = true);
	~Buffer();

	Buffer(Buffer&& move) noexcept;
	Buffer(const Buffer& copy);
	Buffer& operator=(Buffer&& move) noexcept;
	Buffer& operator=(const Buffer& copy);

// move & copy

private:
	Buffer& move_into(Buffer&& move) noexcept;
	Buffer& copy_into(const Buffer& copy);
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

		aCustom_a1, aCustom_a2, aCustom_a3, aCustom_a4,
		aCustom_b1, aCustom_b2, aCustom_b3, aCustom_b4,
		aCustom_c1, aCustom_c2, aCustom_c3, aCustom_c4,

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
	int  NumberOfBuffers()     const;
	bool HasInstancedBuffers() const;

	      r<Buffer>&  Get    (AttribName name);
		  BufferInfo& GetInfo(AttribName name);
	const r<Buffer>&  Get    (AttribName name) const;
	const BufferInfo& GetInfo(AttribName name) const;

	Mesh& SetTopology(Topology topology);

	// set the instanced stride of a buffer
	// if set to 0, instancing is disabled
	// if any buffers are instanced, DrawInstanced is required to draw
	// an assert will fire if this is not done
	Mesh& SetInst(AttribName name, int instancedStride);

	// set the byte offset of an attribute
	// a single buffer can be bound to many attribs, this allows use of differnt parts of the data
	// for each attrib
	// max size for each attrib element is a vec4, so for a mat4, offsets are requires to use a single buffer
	Mesh& SetOffset(AttribName name, int offset);

	// instances a buffer
	// if buffer->Repeat() returns more than 4, the attribs past 'name' are also linked to this buffer
	// and their infos are set to reflect the offset inside each buffer element
	Mesh& Add(AttribName name, int instancedStride, int forceRepeat, const r<Buffer>& buffer);

	// constructs a buffer and sets its data
	template<typename _t>
	Mesh& Add(AttribName name, int instancedStride, int isStatic, const std::vector<_t>& data)
	{
		auto [repeat, type] = get_element_type_info<_t>();
		r<Buffer> buffer = mkr<Buffer>(data.size(), repeat, type, PICK_INHERIT);
		buffer->Set((int)data.size(), data.data());
		return Add(name, instancedStride, buffer->Repeat(), buffer);
	}

// shorthand for simple configs (no instancing)

	template<typename _t>
	Mesh& Add(AttribName name, const std::vector<_t>& data)
	{
		return Add<_t>(name, 0, INHERIT_HOST, data);
	}

	// setups a buffer without data
	template<typename _t>
	Mesh& Add(AttribName name, int instancedStride = 0, int isStatic = INHERIT_HOST)
	{
		Add<_t>(name, instancedStride, isStatic, {});
		return *this;
	}

	Mesh& Add(AttribName name, const r<Buffer>& buffer);

	void Draw(Topology drawType = Topology::tTriangles);
	void DrawInstanced(int numberOfInstances, Topology drawType = Topology::tTriangles);

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

	bool OnHost()       const override;
	bool OnDevice()     const override;
	int  DeviceHandle() const override;
protected:
	void _FreeHost()         override;
	void _FreeDevice()       override;
	void _InitOnDevice()     override;
	void _UpdateOnDevice()   override;
	void _UpdateFromDevice() override;

// construction

public:
	Mesh(bool isStatic = true);
	~Mesh();

	Mesh(Mesh&& move) noexcept;
	Mesh(const Mesh& copy);
	Mesh& operator=(Mesh&& move) noexcept;
	Mesh& operator=(const Mesh& copy);

	Mesh Copy();

// move & copy

private:
	Mesh& move_into(Mesh&& move) noexcept;
	Mesh& copy_into(const Mesh& copy);

// helpers

private:
	bool SendBindAndReturnHasIndex();
};

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
	using _buffers = std::unordered_map<ShaderName, std::string>;
	
	_buffers     m_buffers;            // deafult construction
	GLuint       m_device   = 0;

	int          m_slot     = 0;       // number of active texture slots

public:
	int NumberOfShaders()       const;
	int NumberOfBoundTextures() const;

	// append source onto a shader

	void Add(ShaderName name, const char* str);
	void Add(ShaderName name, const std::string& str);

	void Use();

	void Set(const std::string& name, const   int& x);
	void Set(const std::string& name, const   u32& x);
	void Set(const std::string& name, const   f32& x);
	void Set(const std::string& name, const fvec1& x);
	void Set(const std::string& name, const fvec2& x);
	void Set(const std::string& name, const fvec3& x);
	void Set(const std::string& name, const fvec4& x);
	void Set(const std::string& name, const ivec1& x);
	void Set(const std::string& name, const ivec2& x);
	void Set(const std::string& name, const ivec3& x);
	void Set(const std::string& name, const ivec4& x);
	void Set(const std::string& name, const fmat2& x);
	void Set(const std::string& name, const fmat3& x);
	void Set(const std::string& name, const fmat4& x);

	// auto sends texture to device
	void Set(const std::string& name, Texture& texture);

// interface

public:
	bool OnHost()       const override;
	bool OnDevice()     const override;
	int  DeviceHandle() const override;
protected:
	void _FreeHost()         override;
	void _FreeDevice()       override;
	void _InitOnDevice()     override;
	void _UpdateOnDevice()   override;
	void _UpdateFromDevice() override;

// construction

public:
	ShaderProgram(bool isStatic = true);
	~ShaderProgram();

	ShaderProgram(ShaderProgram&& move) noexcept;
	ShaderProgram(const ShaderProgram& copy);
	ShaderProgram& operator=(ShaderProgram&& move) noexcept;
	ShaderProgram& operator=(const ShaderProgram& copy);

// move & copy

private:
	ShaderProgram& move_into(ShaderProgram&& move);
	ShaderProgram& copy_into(const ShaderProgram& copy);

// helpers

private:
	GLint gl_location(const std::string& name) const;
};

// Translations

GLenum gl_format              (Texture::Usage usage);
GLenum gl_iformat             (Texture::Usage usage);
GLenum gl_type                (Texture::Usage usage);
int    gl_num_channels        (Texture::Usage usage);
int    gl_bytes_per_channel   (Texture::Usage usage);
GLenum gl_attachment          (Target::AttachmentName name);
int    gl_element_type_size   (Buffer::ElementType type);
GLenum gl_buffer_draw         (bool isStatic);
GLenum gl_format              (Buffer::ElementType type);
GLenum gl_drawtype            (Mesh::Topology drawType);
GLenum gl_shader_type         (ShaderProgram::ShaderName type);
GLint  gl_program_texture_slot(int slot);