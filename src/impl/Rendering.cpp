#include "Rendering.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/load_image.h"

IDeviceObject::IDeviceObject(
	bool isStatic
)
	: m_static   (isStatic)
	, m_outdated (true)
{}

void IDeviceObject::FreeHost()
{
	assert_on_host();
	_FreeHost();
}

void IDeviceObject::FreeDevice()
{
	assert_on_device();
	_FreeDevice();
	m_outdated = true;
}

void IDeviceObject::SendToDevice()
{
	assert_on_host(); // always require at least something on host to be copied to device

	if (OnDevice()) // if on device and outdated
	{
		assert_not_static();
		if (m_outdated) _UpdateOnDevice();
	}

	else // if no device then init
	{
		_InitOnDevice();
		if (m_static) FreeHost();
	}

	m_outdated = false;
}

void IDeviceObject::SendToHost()
{
	assert_on_host(); // need host memory
	assert_not_static();
	_UpdateFromDevice();
}

void IDeviceObject::Cleanup()
{
	if (OnDevice()) _FreeDevice();	
	if (OnHost())   _FreeHost();
}

bool IDeviceObject::IsStatic() const
{
	return m_static;
}

bool IDeviceObject::Outdated() const
{
	return m_outdated;
}

void IDeviceObject::MarkForUpdate()
{
	m_outdated = true;
}

void IDeviceObject::assert_on_host()    const { assert(OnHost()   && "Device object has no data on host"); }
void IDeviceObject::assert_on_device()  const { assert(OnDevice() && "Device object has no data on device"); }
void IDeviceObject::assert_is_static()  const { assert( m_static  && "Device object is static"); }
void IDeviceObject::assert_not_static() const { assert(!m_static  && "Device object is not static"); }

void IDeviceObject::copy_base(const IDeviceObject* copy)
{
	m_static   = copy->m_static;
	m_outdated = copy->m_outdated; // this might want to always be true when copying, move should keep it the same
}

int             Texture::Width()           const { return m_width; } 
int             Texture::Height()          const { return m_height; } 
int             Texture::Channels()        const { return m_channels; }
int             Texture::BytesPerChannel() const { return m_bytesPerChannel; }
Texture::Usage  Texture::UsageType()       const { return m_usage; }
Texture::Filter Texture::FilterType()      const { return m_filter;}
u8*             Texture::Pixels()          const { return (u8*)m_host; }
int             Texture::BufferSize()      const { return Width() * Height() * Channels() * BytesPerChannel(); }
int             Texture::Length()          const { return Width() * Height(); }
vec2            Texture::Dimensions()      const { return vec2(Width(), Height()); }
float           Texture::Aspect()          const { return Height() / (float)Width(); }

Texture& Texture::SetFilter(Filter filter)
{
	m_filter = filter;

	if (OnDevice())
	{
		gl(glBindTexture(GL_TEXTURE_2D, m_device));
		_SetDeviceFilter();
	}

	return *this;
}

      Color& Texture::At(int x, int y)       { return At(Index32(x, y)); }
const Color& Texture::At(int x, int y) const { return At(Index32(x, y)); }

Color& Texture::At(int index32)
{
	assert_on_host();
	assert_valid_index(index32);
	MarkForUpdate();
	return *(Color*)(Pixels() + Index(index32));
}

const Color& Texture::At(int index32) const
{
	assert_on_host();
	assert_valid_index(index32);
	return *(Color*)(Pixels() + Index(index32));
}

void Texture::Set(int x, int y, const Color& color)
{
	Set(Index32(x, y), color);
}

void Texture::Set(int index32, const Color& color)
{
	Color& c = At(index32);

	if (m_channels > 0) c.r = color.r;
	if (m_channels > 1) c.g = color.g;
	if (m_channels > 2) c.b = color.b;
	if (m_channels > 3) c.a = color.a;
}

int Texture::Index32(int x, int y) const { return IsInBounds(x, y) ? x + y * m_width : -1; }
int Texture::Index  (int x, int y) const { return IsInBounds(x, y) ? Index32(x, y) * m_channels * m_bytesPerChannel : -1; }
int Texture::Index  (int index32)  const { return index32 * m_channels * m_bytesPerChannel; }

void Texture::ClearHost(Color color)
{
	assert_on_host();
	memset(m_host, color.as_u32, BufferSize());

	MarkForUpdate();
}

void Texture::Resize(int width, int height)
{
	assert_not_static();

	if (m_width == width && m_height == height) return;

	m_width = width;
	m_height = height;

	if (OnHost())
	{
		void* newMemory = realloc(m_host, BufferSize());
		assert(newMemory && "failed to resize texture");
		m_host = (u8*)newMemory;
	}

	MarkForUpdate();
}

Texture Texture::CopySubRegion(int minX, int minY, int maxX, int maxY, Usage usage, int isStatic) const
{
	Texture tex = Texture(maxX - minX, maxY - minY, usage, PICK_INHERIT);
    tex.ClearHost(Color(0));

    for (int ix = minX; ix < maxX; ix++) // copy to new texture, could flatten loop
    for (int iy = minY; iy < maxY; iy++)
    {
        const Color& c = At(ix, iy);
        if (c.as_u32 > 0)
        {
            tex.Set(ix - minX, iy - minY, c);
        }
    }

    return tex;
}

bool Texture::OnHost()       const { return m_host   != nullptr; }
bool Texture::OnDevice()     const { return m_device != 0u;      }
int  Texture::DeviceHandle() const { return m_device; } 

void Texture::_FreeHost()
{
	log_render("i~Texture Free Host - bytes: %d", BufferSize());

	free(m_host);
	m_host = nullptr;
}

void Texture::_FreeDevice()
{
	log_render("i~Texture Free Device - bytes: %d handle: %d", BufferSize(), m_device);

	gl(glDeleteTextures(1, &m_device));
	m_device = 0;
}

void Texture::_InitOnDevice()
{
	gl(glGenTextures(1, &m_device));
	gl(glBindTexture(GL_TEXTURE_2D, m_device));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	gl(glTexImage2D(GL_TEXTURE_2D, 0, gl_iformat(m_usage), Width(), Height(), 0, gl_format(m_usage), gl_type(m_usage), Pixels()));

	_SetDeviceFilter();

	log_render("i~Texture Init Device - dim: (%d, %d, %d) bytes: %d handle: %d", Width(), Height(), Channels(), BufferSize(), m_device);
}

void Texture::_UpdateOnDevice()
{
	glBindTexture(GL_TEXTURE_2D, m_device);

	int w, h;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

	//if (w != m_width || h != m_height)
	//{
		gl(glTexImage2D(GL_TEXTURE_2D, 0, gl_iformat(m_usage), Width(), Height(), 0, gl_format(m_usage), gl_type(m_usage), Pixels()));
	//}

	//else
	//{
		//gl(glTextureSubImage2D(m_device, 0, 0, 0, Width(), Height(), gl_format(m_usage), gl_type(m_usage), Pixels()));
	//}

	log_render("i~Texture Update Device - dim: (%d, %d, %d) bytes: %d handle: %d", Width(), Height(), Channels(), BufferSize(), m_device);
}

void Texture::_UpdateFromDevice()
{
	// really slow find fix
	// is it the data or the point in time this function is getting called?

	//gl(glGetTextureImage(m_device, 0, gl_format(m_usage), gl_type(m_usage), BufferSize(), Pixels()));
    gl(glBindTexture(GL_TEXTURE_2D, m_device));
    gl(glGetTexImage(GL_TEXTURE_2D, 0, gl_format(m_usage), gl_type(m_usage), Pixels()));

	log_render("i~Texture Copy to Host - handle: %d", m_device);
}

void Texture::_SetDeviceFilter()
{
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter(m_filter)));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter(m_filter)));
}

Texture::Texture()
	: IDeviceObject (true)
{}

Texture::Texture(
	const std::string& path,
	bool isStatic
)
	: IDeviceObject (isStatic)
{
	auto [pixels, width, height, channels] = load_image(path);
	assert(channels > 0 && channels <= 4 && "Invalid RGBA channel count created by stb");
	init_texture_host_memory(pixels, width, height, (Usage)channels);
}

Texture::Texture(
	int width, int height, Usage usage,
	bool isStatic
)
	: IDeviceObject (isStatic)
{
	void* pixels = malloc(width * height * gl_num_channels(usage) * gl_bytes_per_channel(usage));
	init_texture_host_memory(pixels, width, height, usage);
}

Texture::Texture(
	int width, int height, Usage usage,
	void* pixels
)
	: IDeviceObject (false)
{
	init_texture_host_memory(pixels, width, height, usage);
}

Texture::~Texture()
{
	Cleanup();
}

Texture::Texture           (      Texture&& move) noexcept : IDeviceObject(move.IsStatic()) { move_into(std::move(move)); }
Texture::Texture           (const Texture&  copy)          : IDeviceObject(copy.IsStatic()) { copy_into(copy); }
Texture& Texture::operator=(      Texture&& move) noexcept                                  { return move_into(std::move(move)); }
Texture& Texture::operator=(const Texture&  copy)                                           { return copy_into(copy); }

Texture& Texture::move_into(Texture&& move) noexcept
{
	copy_base(&move);

	m_width           = move.m_width;
	m_height          = move.m_height;
	m_channels        = move.m_channels;
	m_bytesPerChannel = move.m_bytesPerChannel;
	m_usage           = move.m_usage;
	m_filter          = move.m_filter;


	m_host            = move.m_host;
	m_device          = move.m_device;

	move.m_host = nullptr;
	move.m_device = 0;

	return *this;
}

Texture& Texture::copy_into(const Texture& copy)
{
	copy_base(&copy);

	m_width           = copy.m_width;
	m_height          = copy.m_height;
	m_channels        = copy.m_channels;
	m_bytesPerChannel = copy.m_bytesPerChannel;
	m_usage           = copy.m_usage;
	m_filter          = copy.m_filter;

	m_device          = 0;

	if (copy.OnHost())
	{
		m_host = (u8*)malloc(BufferSize());

		if (m_host)
		{
			memcpy(m_host, copy.m_host, BufferSize());
		}
		else
		{
			assert(false);
			log_render("Failed to allocate texture memory");
		}
	}

	return *this;
}

void Texture::init_texture_host_memory(void* pixels, int w, int h, Usage usage)
{
	assert(pixels && "Sprite failed to load");

	m_host = (u8*)pixels;
	m_width = w;
	m_height = h;
	m_usage = usage;
	m_channels = gl_num_channels(usage);
	m_bytesPerChannel = gl_bytes_per_channel(usage);
}

bool Texture::IsIndexValid(int index32) const
{
	return index32 >= 0 && index32 < Length();
}

bool Texture::IsInBounds(int x, int y) const
{
	return x >= 0 && y >= 0 && x < m_width && y < m_height;
}

void Texture::assert_valid_index(int index32) const
{
	assert(IsIndexValid(index32) && "Texture pixel index out of bounds");
}

int Target::NumberOfAttachments() const { return (int)m_attachments.size(); }
int Target::Width()               const { return m_width; }
int Target::Height()              const { return m_height; }

	  r<Texture>& Target::Get(AttachmentName name)       { return m_attachments.at(name); }
const r<Texture>& Target::Get(AttachmentName name) const { return m_attachments.at(name); }

void Target::Add(AttachmentName name, const r<Texture>& texture)
{
	// only support same sized textures

	if (m_attachments.size() == 0)
	{
		m_width  = texture->Width();
		m_height = texture->Height();
	}

	else
	{
		assert(m_width == texture->Width() && m_height == texture->Height() && "All attachments must be of equal size");
	}

	assert(m_attachments.find(name) == m_attachments.end() && "Attachment already exists in target");
	// should put asserts for depth and spencil for what rgb values they need
	m_attachments.emplace(name, texture);
}

r<Texture> Target::Add(AttachmentName name, int width, int height, Texture::Usage usage, int isStatic)
{
	r<Texture> texture = mkr<Texture>(width, height, usage, PICK_INHERIT);
	Add(name, texture);

	return texture;
}

void Target::Resize(int width, int height)
{
	if (m_width == width && m_height == height) return;

	for (auto& [_, texture] : m_attachments) texture->Resize(width, height);
	m_width = width;
	m_height = height;

	MarkForUpdate();
}

void Target::Use()
{
	if (!OnDevice() || Outdated()) SendToDevice();
	gl(glBindFramebuffer(GL_FRAMEBUFFER, m_device));
}

bool Target::OnHost()       const { return m_attachments.size() != 0; }
bool Target::OnDevice()     const { return m_device != 0u; }
int  Target::DeviceHandle() const { return m_device; }

void Target::_FreeHost()
{
	for (auto& [_, texture] : m_attachments) if (texture->OnHost() && texture->IsStatic()) texture->FreeHost();
}

void Target::_FreeDevice()
{
	//for (auto& [_, texture] : m_attachments) if (texture->OnDevice()) texture->FreeDevice();
	gl(glDeleteFramebuffers(1, &m_device));
	m_device = 0;
}

void Target::_InitOnDevice()
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

	gl(glDrawBuffers((GLsizei)toDraw.size(), toDraw.data()));

	GLint err = gl(glCheckFramebufferStatus(GL_FRAMEBUFFER));
	if (err != GL_FRAMEBUFFER_COMPLETE)
	{
		log_render("failed to create framebuffer");
	}
}

void Target::_UpdateOnDevice()
{
	for (auto& [_, texture] : m_attachments) if (texture->OnHost()) texture->SendToDevice();
}

void Target::_UpdateFromDevice()
{
	for (auto& [_, texture] : m_attachments) if (texture->OnDevice() && !texture->IsStatic()) texture->SendToHost();
}

Target::Target(
	bool isStatic
)
	: IDeviceObject(isStatic)
{}

Target::~Target() 
{
	Cleanup();
}

Target::Target           (      Target&& move) noexcept : IDeviceObject(move.IsStatic()) { move_into(std::move(move)); }
Target::Target           (const Target&  copy)          : IDeviceObject(copy.IsStatic()) { copy_into(copy); }
Target& Target::operator=(      Target&& move) noexcept                                  { return move_into(std::move(move)); }
Target& Target::operator=(const Target&  copy)                                           { return copy_into(copy); }

Target& Target::move_into(Target&& move) noexcept
{
	copy_base(&move);

	m_width           = move.m_width;
	m_height          = move.m_height;
	m_attachments     = std::move(move.m_attachments);
	m_device          = 0;

	move.m_device = 0;

	return *this;
}

Target& Target::copy_into(const Target& copy)
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

	  void*         Buffer::Data()                   { return m_host.data(); }
const void*         Buffer::Data()             const { return m_host.data(); }
int                 Buffer::Length()           const { return m_length;  }
int                 Buffer::Repeat()           const { return m_repeat; }
Buffer::ElementType Buffer::Type()             const { return m_type; }
int                 Buffer::BytesPerElement()  const { return Repeat() * gl_element_type_size(Type()); }
int                 Buffer::Bytes()            const { return (int)m_host.size(); }

void Buffer::Clear()
{
	m_host.clear();
	m_length = 0;

	MarkForUpdate();
}

void Buffer::Resize(int elementCount)
{
	m_host.resize(BytesPerElement() * elementCount);
	m_length = elementCount;

	MarkForUpdate();
}

void Buffer::SetBytes(int byteCount, const void* data)
{
	m_length = byteCount / BytesPerElement();
	m_host.resize(byteCount);
	memcpy(m_host.data(), data, byteCount); // put an error check to see if memory was actually allocated
	
	MarkForUpdate();
}

void Buffer::PushBytes(int byteCount, const void* data)
{
	int end = Bytes();

	m_length += byteCount / BytesPerElement();
	m_host.resize(end + byteCount);
	memcpy((char*)m_host.data() + end, data, byteCount);
	
	MarkForUpdate();
}

void Buffer::EraseBytes(int byteIndex, int byteCount)
{
	m_length -= byteCount / BytesPerElement();
	m_host.erase(m_host.begin() + byteIndex, m_host.begin() + byteIndex + byteCount);
	
	MarkForUpdate();
}

void Buffer::PopBytes(int byteCount)
{
	m_length -= byteCount / BytesPerElement();
	m_host.erase(m_host.end() - byteCount, m_host.end());
	
	MarkForUpdate();
}

void Buffer::Set  (int elementCount, const void* elements) { SetBytes  (BytesPerElement() * elementCount, elements); }
void Buffer::Push (int elementCount, const void* elements) { PushBytes (BytesPerElement() * elementCount, elements); }
void Buffer::Erase(int elementIndex, int elementCount)     { EraseBytes(BytesPerElement() * elementIndex, elementCount); }
void Buffer::Pop  (int elementCount)                       { PopBytes  (BytesPerElement() * elementCount); }

bool Buffer::OnHost()       const { return m_onHost; }
bool Buffer::OnDevice()     const { return m_device != 0u; }
int  Buffer::DeviceHandle() const { return m_device; }
	
void Buffer::_FreeHost()
{
	m_host = {};
	m_onHost = false;
}

void Buffer::_FreeDevice()
{
	gl(glDeleteBuffers(1, &m_device));
	m_device = 0;
}

void Buffer::_InitOnDevice()
{
	gl(glGenBuffers(1, &m_device));
	gl(glBindBuffer(GL_ARRAY_BUFFER, m_device)); // user can bind to what they want after using ::DeviceHandle
	gl(glBufferData(GL_ARRAY_BUFFER, Bytes(), Data(), gl_buffer_draw(IsStatic())));
}

void Buffer::_UpdateOnDevice()
{
	// on realloc, does old buffer need to be destroied?

    gl(glBindBuffer(GL_ARRAY_BUFFER, m_device));
    
	int size = 0;
	gl(glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size));
	if (Bytes() != size) { gl(glBufferData(GL_ARRAY_BUFFER, Bytes(), Data(), gl_buffer_draw(IsStatic()))); }
	else                 { gl(glBufferSubData(GL_ARRAY_BUFFER, 0, Bytes(), Data())); }
}

void Buffer::_UpdateFromDevice()
{
	gl(glGetNamedBufferSubData(m_device, 0, Bytes(), Data()));
}

Buffer::Buffer(
	bool isStatic
)
	: IDeviceObject (isStatic)
{}

Buffer::Buffer(
	int length, int repeat, ElementType type,
	bool isStatic
)
	: IDeviceObject (isStatic)
	, m_repeat      (repeat)
	, m_type        (type)
{
	m_host.resize((length * repeat * gl_element_type_size(type)));
}

Buffer::~Buffer()
{ 
	Cleanup();
}

Buffer::Buffer           (      Buffer&& move) noexcept : IDeviceObject(move.IsStatic()) { move_into(std::move(move)); }
Buffer::Buffer           (const Buffer&  copy)          : IDeviceObject(copy.IsStatic()) { copy_into(copy); }
Buffer& Buffer::operator=(      Buffer&& move) noexcept                                  { return move_into(std::move(move)); }
Buffer& Buffer::operator=(const Buffer&  copy)                                           { return copy_into(copy); }

Buffer& Buffer::move_into(Buffer&& move) noexcept
{
	copy_base(&move);

	m_repeat		  = move.m_repeat;
	m_type            = move.m_type;
	m_length          = move.m_length;

	m_host  		  = std::move(move.m_host);
	m_device		  = move.m_device;

	move.m_device = 0;

	return *this;
}

Buffer& Buffer::copy_into(const Buffer& copy)
{
	copy_base(&copy);

	m_repeat          = copy.m_repeat;
	m_type            = copy.m_type;
	m_length          = copy.m_length;

	m_device          = copy.IsStatic() ? copy.m_device : 0; // if its static take device
	m_host            = copy.m_host;
	m_onHost          = copy.m_onHost;

	return *this;
}

int  Mesh::NumberOfBuffers()     const { return (int)m_buffers.size(); }
bool Mesh::HasInstancedBuffers() const { return m_hasInstance > 0; }

      r<Buffer>&  Mesh::Get          (AttribName name)       { MarkForUpdate(); return m_buffers.at(name); }
const r<Buffer>&  Mesh::Get          (AttribName name) const {                  return m_buffers.at(name); }
      Mesh::BufferInfo& Mesh::GetInfo(AttribName name)       { MarkForUpdate(); return m_info.at(name); }
const Mesh::BufferInfo& Mesh::GetInfo(AttribName name) const {                  return m_info.at(name); }

void Mesh::Clear()
{
	for (auto [name, buffer] : m_buffers) buffer->Clear();
}

Mesh& Mesh::SetTopology(Topology topology)
{
	this->topology = topology;
	return *this;
}

Mesh& Mesh::SetInst(AttribName name, int instancedStride)
{
	if (instancedStride == 0) m_hasInstance -= GetInfo(name).instancedStride;
	else                      m_hasInstance += instancedStride;

	GetInfo(name).instancedStride = instancedStride; 
		
	return *this; 
}

Mesh& Mesh::SetOffset(AttribName name, int offset)
{
	GetInfo(name).offset = offset;
	return *this; 
}

Mesh& Mesh::SetNormalization(AttribName name, bool normalized)
{
	GetInfo(name).normalized = normalized;
	return *this;
}

Mesh& Mesh::Add(AttribName name, int instancedStride, int forceRepeat, bool normalized, const r<Buffer>& buffer)
{
	assert(m_buffers.find(name) == m_buffers.end() && "Buffer already exists in mesh");
	assert(name != aIndexBuffer || (buffer->Type() == Buffer::eInt && buffer->Repeat() == 1) && "index buffer must be of type 'int' with a repeat of 1.");
	assert(forceRepeat > 0 && "Repeat must be above 0");

	// if repeat is larger than 4, then use the next attribs
	// add the same buffer ref, but change the offset and repeat of buffer info

	int repeat = forceRepeat;
		
	for (int i = 0; i < forceRepeat; i += 4) // will always run once
	{
		BufferInfo info;
		info.instancedStride = instancedStride;
		info.offset = i * sizeof(f32);            // assuming that each element is a single p float, and split is every 4th
		info.repeat = min(4, repeat);
		info.normalized = normalized;

		m_buffers.emplace(AttribName( (int)name + i/4 ), buffer);
		m_info   .emplace(AttribName( (int)name + i/4 ), info);

		m_hasInstance += instancedStride;

		repeat -= 4;
	}
	
	MarkForUpdate();
	return *this;
}

Mesh& Mesh::Add(AttribName name, const r<Buffer>& buffer)
{
	return Add(name, 0, buffer->Repeat(), false, buffer);
}

void Mesh::Draw()
{
    Draw(topology);
}

void Mesh::DrawInstanced(int numberOfInstances)
{
    DrawInstanced(numberOfInstances, topology);
}

// gl() calls cause nullptr exception/

void Mesh::Draw(Topology drawType)
{
	assert(!HasInstancedBuffers() && "mesh has instanced buffers, need to call DrawInstanced");

	bool hasIndex = SendBindAndReturnHasIndex();
	if (hasIndex) { /*gl(*/glDrawElements(gl_drawtype(drawType),    m_buffers.at(aIndexBuffer)->Length(), GL_UNSIGNED_INT, nullptr)/*)*/; }
	else          { /*gl(*/glDrawArrays  (gl_drawtype(drawType), 0, m_buffers.at(aPosition)   ->Length())/*)*/; }
}

void Mesh::DrawInstanced(int numberOfInstances, Topology drawType)
{
	//if (numberOfInstances == 0) return; // skip

	assert(HasInstancedBuffers() && "mesh has no instanced buffers, need to call Draw");

	bool hasIndex = SendBindAndReturnHasIndex();
	if (hasIndex) { /*gl(*/glDrawElementsInstanced(gl_drawtype(drawType),    m_buffers.at(aIndexBuffer)->Length(), GL_UNSIGNED_INT, nullptr, numberOfInstances)/*)*/; }
	else          { /*gl(*/glDrawArraysInstanced  (gl_drawtype(drawType), 0, m_buffers.at(aPosition)   ->Length(),                           numberOfInstances)/*)*/; }
}

bool Mesh::OnHost()       const { return m_buffers.size() != 0; }
bool Mesh::OnDevice()     const { return m_device != 0u; }
int  Mesh::DeviceHandle() const { return m_device; } 

void Mesh::_FreeHost()
{
	for (auto& [_, buffer] : m_buffers) if (buffer->OnHost() && buffer->IsStatic()) buffer->FreeHost();
}

void Mesh::_FreeDevice()
{
	//for (auto& [_, buffer] : m_buffers) if (buffer->OnDevice()) buffer->FreeDevice();
	gl(glDeleteVertexArrays(1, &m_device));
	m_device = 0;
}

void Mesh::_InitOnDevice()
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
		gl(glVertexAttribPointer(attrib, info.repeat, gl_format(buffer->Type()), info.normalized, buffer->BytesPerElement(), (void*)(uintptr_t)info.offset));
		gl(glVertexAttribDivisor(attrib, info.instancedStride));
	}
}

void Mesh::_UpdateOnDevice()
{
	for (auto& [_, buffer] : m_buffers) if (buffer->OnHost()) buffer->SendToDevice();
}

void Mesh::_UpdateFromDevice()
{
	for (auto& [_, buffer] : m_buffers) if (buffer->OnDevice()) buffer->SendToHost();
}

Mesh::Mesh(
	bool isStatic
)
	: IDeviceObject (isStatic)
{}

Mesh::~Mesh() 
{
	Cleanup(); 
}

Mesh::Mesh           (      Mesh&& move) noexcept : IDeviceObject(move.IsStatic()) { move_into(std::move(move)); }
Mesh::Mesh           (const Mesh&  copy)          : IDeviceObject(copy.IsStatic()) { copy_into(copy); }
Mesh& Mesh::operator=(      Mesh&& move) noexcept                                  { return move_into(std::move(move)); }
Mesh& Mesh::operator=(const Mesh&  copy)                                           { return copy_into(copy); }

Mesh Mesh::Copy()
{
	return *this;
}

Mesh& Mesh::move_into(Mesh&& move) noexcept
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

Mesh& Mesh::copy_into(const Mesh& copy)
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
		r<Buffer> copyBuffer = mkr<Buffer>(*buffer);

		for (const AttribName& name : names)
		{
			m_buffers[name] = copyBuffer; // emplace didnt copy?
		}
	}

	return *this;
}

bool Mesh::SendBindAndReturnHasIndex()
{
	if (!OnDevice() || Outdated()) SendToDevice();
	gl(glBindVertexArray(m_device));
	return m_buffers.find(aIndexBuffer) != m_buffers.end();
}

int ShaderProgram::NumberOfShaders()       const { return (int)m_buffers.size(); }
int ShaderProgram::NumberOfBoundTextures() const { return m_slot; }

ShaderProgram& ShaderProgram::Add(ShaderName name, const char* str)
{
	m_buffers[name].append(str);
	return *this;
}

ShaderProgram& ShaderProgram::Add(ShaderName name, const std::string& str)
{
	Add(name, str.c_str());
	return *this;
}

ShaderProgram& ShaderProgram::Use()
{
	if (!OnDevice()) SendToDevice();
	gl(glUseProgram(m_device));

	//m_slot = 0; // reset for texture bindings
	return *this;
}

ShaderProgram& ShaderProgram::Set(const std::string& name, const   int& x) { gl(glUniform1iv       (gl_location(name), 1,            (  int*)  &x)); return *this; }
ShaderProgram& ShaderProgram::Set(const std::string& name, const   u32& x) { gl(glUniform1uiv      (gl_location(name), 1,            (  u32*)  &x)); return *this; }
ShaderProgram& ShaderProgram::Set(const std::string& name, const   f32& x) { gl(glUniform1fv       (gl_location(name), 1,            (float*)  &x)); return *this; }
ShaderProgram& ShaderProgram::Set(const std::string& name, const fvec1& x) { gl(glUniform1fv       (gl_location(name), 1,            (float*)  &x)); return *this; }
ShaderProgram& ShaderProgram::Set(const std::string& name, const fvec2& x) { gl(glUniform2fv       (gl_location(name), 1,            (float*)  &x)); return *this; }
ShaderProgram& ShaderProgram::Set(const std::string& name, const fvec3& x) { gl(glUniform3fv       (gl_location(name), 1,            (float*)  &x)); return *this; }
ShaderProgram& ShaderProgram::Set(const std::string& name, const fvec4& x) { gl(glUniform4fv       (gl_location(name), 1,            (float*)  &x)); return *this; }
ShaderProgram& ShaderProgram::Set(const std::string& name, const ivec1& x) { gl(glUniform1iv       (gl_location(name), 1,            (  int*)  &x)); return *this; }
ShaderProgram& ShaderProgram::Set(const std::string& name, const ivec2& x) { gl(glUniform2iv       (gl_location(name), 1,            (  int*)  &x)); return *this; }
ShaderProgram& ShaderProgram::Set(const std::string& name, const ivec3& x) { gl(glUniform3iv       (gl_location(name), 1,            (  int*)  &x)); return *this; }
ShaderProgram& ShaderProgram::Set(const std::string& name, const ivec4& x) { gl(glUniform4iv       (gl_location(name), 1,            (  int*)  &x)); return *this; }
ShaderProgram& ShaderProgram::Set(const std::string& name, const fmat2& x) { gl(glUniformMatrix2fv (gl_location(name), 1,  GL_FALSE, (float*)  &x)); return *this; }
ShaderProgram& ShaderProgram::Set(const std::string& name, const fmat3& x) { gl(glUniformMatrix3fv (gl_location(name), 1,  GL_FALSE, (float*)  &x)); return *this; }
ShaderProgram& ShaderProgram::Set(const std::string& name, const fmat4& x) { gl(glUniformMatrix4fv (gl_location(name), 1,  GL_FALSE, (float*)  &x)); return *this; }

ShaderProgram& ShaderProgram::Set(const std::string& name, const Color& color)
{
    Set(name, color.as_v4());
	return *this;
}

ShaderProgram& ShaderProgram::Set(const std::string& name, r<Texture> texture)
{
    Set(name, *texture);
	return *this;
}

ShaderProgram& ShaderProgram::Set(const std::string& name, Texture& texture)
{
	if (!texture.OnDevice() || texture.Outdated()) texture.SendToDevice();
	gl(glBindTexture(GL_TEXTURE_2D, texture.DeviceHandle())); // need texture usage
	gl(glActiveTexture(gl_program_texture_slot(m_slot)));
	gl(glUniform1i(gl_location(name), m_slot));

	//m_slot += 1;

	return *this;
}

bool ShaderProgram::OnHost()       const { return m_buffers.size() != 0; }
bool ShaderProgram::OnDevice()     const { return m_device != 0u; }
int  ShaderProgram::DeviceHandle() const { return m_device; } 

void ShaderProgram::_FreeHost()
{
	m_buffers.clear();
}

void ShaderProgram::_FreeDevice()
{
	gl(glDeleteProgram(m_device));
	m_device = 0;
}

void ShaderProgram::_InitOnDevice()
{
	m_device = gl(glCreateProgram());

	for (auto& [name, buffer] : m_buffers)
	{
		const char* source = buffer.c_str();

		GLuint shader = gl(glCreateShader(gl_shader_type(name)));
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
			log_render("Failed to compile shader: %s", (char*)infoLog.data());
			assert(false && "Failed to compile shader"); // maybe soft error
		}

		gl(glAttachShader(m_device, shader));
		gl(glDeleteShader(shader));
	}

	gl(glLinkProgram(m_device));
}

void ShaderProgram::_UpdateOnDevice()
{
	gl(glDeleteProgram(m_device));
	_InitOnDevice();
}

void ShaderProgram::_UpdateFromDevice()
{
	assert(false && "Updating from device has no impl for shaders");
}

ShaderProgram::ShaderProgram(
	bool isStatic
)
	: IDeviceObject (isStatic)
{}

ShaderProgram::~ShaderProgram() 
{ 
	Cleanup(); 
}

ShaderProgram::ShaderProgram           (      ShaderProgram&& move) noexcept : IDeviceObject(move.IsStatic()) { move_into(std::move(move)); }
ShaderProgram::ShaderProgram           (const ShaderProgram&  copy)          : IDeviceObject(copy.IsStatic()) { copy_into(copy); }
ShaderProgram& ShaderProgram::operator=(      ShaderProgram&& move) noexcept                                  { return move_into(std::move(move)); }
ShaderProgram& ShaderProgram::operator=(const ShaderProgram&  copy)                                           { return copy_into(copy); }

ShaderProgram& ShaderProgram::move_into(ShaderProgram&& move)
{
	copy_base(&move);

	m_slot            = move.m_slot;
	m_buffers         = std::move(move.m_buffers);
	m_device          = 0;

	move.m_device = 0;

	return *this;
}

ShaderProgram& ShaderProgram::copy_into(const ShaderProgram& copy)
{
	copy_base(&copy);

	m_slot            = copy.m_slot;
	m_buffers         = copy.m_buffers;
	m_device          = 0;

	return *this;
}

GLint ShaderProgram::gl_location(const std::string& name) const
{
	GLint location = gl(glGetUniformLocation(m_device, name.c_str()));
	return location;
}

// Translation

GLenum gl_format(Texture::Usage usage)
{
	switch (usage)			
	{
		case Texture::Usage::uR:        return GL_RED;
		case Texture::Usage::uRG:       return GL_RG;
		case Texture::Usage::uRGB:      return GL_RGB;
		case Texture::Usage::uRGBA:     return GL_RGBA;
		case Texture::Usage::uDEPTH:    return GL_DEPTH_COMPONENT;
		case Texture::Usage::uSTENCIL:  return GL_STENCIL_INDEX;
		case Texture::Usage::uINT_32:   return GL_RGBA_INTEGER;
		case Texture::Usage::uFLOAT_32: return GL_RGBA; // this doesnt have to specify float???
	}

	assert(false);
	return -1;
}

GLenum gl_iformat(Texture::Usage usage)
{
	switch (usage)
	{
		case Texture::Usage::uR:        return GL_R8;
		case Texture::Usage::uRG:       return GL_RG8;
		case Texture::Usage::uRGB:      return GL_RGB8;
		case Texture::Usage::uRGBA:     return GL_RGBA8;
		case Texture::Usage::uDEPTH:    return GL_DEPTH_COMPONENT32;
		case Texture::Usage::uSTENCIL:  return GL_STENCIL_INDEX8;
		case Texture::Usage::uINT_32:   return GL_RGBA32I;
		case Texture::Usage::uFLOAT_32: return GL_RGBA32F;
	}

	assert(false);
	return -1;
}

GLenum gl_type(Texture::Usage usage)
{
	switch (usage)
	{
		case Texture::Usage::uR:        return GL_UNSIGNED_BYTE;
		case Texture::Usage::uRG:       return GL_UNSIGNED_BYTE;
		case Texture::Usage::uRGB:      return GL_UNSIGNED_BYTE;
		case Texture::Usage::uRGBA:     return GL_UNSIGNED_BYTE;
		case Texture::Usage::uDEPTH:    return GL_FLOAT;
		case Texture::Usage::uSTENCIL:  return GL_UNSIGNED_BYTE;
		case Texture::Usage::uINT_32:   return GL_INT;
		case Texture::Usage::uFLOAT_32: return GL_FLOAT;
	}

	assert(false);
	return -1;
}

int gl_num_channels(Texture::Usage usage)
{
	switch (usage)
	{
		case Texture::Usage::uR:         return 1;
		case Texture::Usage::uRG:        return 2;
		case Texture::Usage::uRGB:       return 3;
		case Texture::Usage::uRGBA:      return 4;
		case Texture::Usage::uDEPTH:     return 1;
		case Texture::Usage::uSTENCIL:   return 1;
		case Texture::Usage::uINT_32:    return 4;
		case Texture::Usage::uFLOAT_32:  return 4;
	}

	assert(false);
	return -1;
}

int gl_bytes_per_channel(Texture::Usage usage)
{
	switch (usage)
	{
		case Texture::Usage::uR:         return sizeof(u8);
		case Texture::Usage::uRG:        return sizeof(u8);
		case Texture::Usage::uRGB:       return sizeof(u8);
		case Texture::Usage::uRGBA:      return sizeof(u8);
		case Texture::Usage::uDEPTH:     return sizeof(f32);
		case Texture::Usage::uSTENCIL:   return sizeof(u8);
		case Texture::Usage::uINT_32:    return sizeof(u32);
		case Texture::Usage::uFLOAT_32:  return sizeof(f32);
	}

	assert(false);
	return -1;
}

GLenum gl_filter(Texture::Filter filter)
{
	switch (filter)
	{
		case Texture::Filter::fSmooth:    return GL_LINEAR;
		case Texture::Filter::fPixelated: return GL_NEAREST;
	}

	assert(false);
	return -1;
}

GLenum gl_attachment(Target::AttachmentName name)
{
	switch (name)
	{
		case Target::AttachmentName::aDepth:   return GL_DEPTH_ATTACHMENT;
		case Target::AttachmentName::aStencil: return GL_STENCIL_ATTACHMENT;
		case Target::AttachmentName::aColor:   return GL_COLOR_ATTACHMENT0;
		case Target::AttachmentName::aColor1:  return GL_COLOR_ATTACHMENT1;
		case Target::AttachmentName::aColor2:  return GL_COLOR_ATTACHMENT2;
		case Target::AttachmentName::aColor3:  return GL_COLOR_ATTACHMENT3;
		case Target::AttachmentName::aColor4:  return GL_COLOR_ATTACHMENT4;
		case Target::AttachmentName::aColor5:  return GL_COLOR_ATTACHMENT5;
	}

	assert(false);
	return -1;
}

int gl_element_type_size(Buffer::ElementType type)
{
	switch (type)
	{
		case Buffer::eByte:  return sizeof(u8);
		case Buffer::eInt:   return sizeof(u32);
		case Buffer::eFloat: return sizeof(f32);
	}

	assert(false && "invalid buffer element type");
	return 0;
}

GLenum gl_buffer_draw(bool isStatic)
{
	return isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
}

GLenum gl_format(Buffer::ElementType type)
{
	switch (type)
	{
		case Buffer::eByte:  return GL_UNSIGNED_BYTE;
		case Buffer::eInt: return GL_INT;
		case Buffer::eFloat: return GL_FLOAT;
	}

	assert(false);
	return -1;
}

GLenum gl_drawtype(Mesh::Topology drawType)
{
	switch (drawType)
	{
		case Mesh::Topology::tTriangles:     return GL_TRIANGLES;
		case Mesh::Topology::tLines:         return GL_LINES;
		case Mesh::Topology::tLoops:         return GL_LINE_LOOP;
		case Mesh::Topology::tTriangleStrip: return GL_TRIANGLE_STRIP;
	}

	assert(false);
	return -1;
}

GLenum gl_shader_type(ShaderProgram::ShaderName type)
{
	switch (type)			
	{
		case ShaderProgram::ShaderName::sVertex:   return GL_VERTEX_SHADER;
		case ShaderProgram::ShaderName::sFragment: return GL_FRAGMENT_SHADER;
		case ShaderProgram::ShaderName::sGeometry: return GL_GEOMETRY_SHADER;
		case ShaderProgram::ShaderName::sCompute:  return GL_COMPUTE_SHADER;
	}

	assert(false);
	return -1;
}

GLint gl_program_texture_slot(int slot)
{
	return GL_TEXTURE0 + slot;
}

void gl_SetClearColor(const Color& color)
{
	gl(glClearColor(color.rf(), color.gf(), color.bf(), color.af()));
}

//
//	Context
//

namespace Render
{
	RenderContext* ctx;

	float RenderContext::WindowAspect() const
	{
		return window_width / (float)window_height;
	}

	float RenderContext::TargetAspect() const
	{
		if (default_target)
		{
			return default_target->Width() / (float)default_target->Height();
		}

		return WindowAspect();
	}

	void CreateContext()
	{
		DestroyContext();
		ctx = new RenderContext();
	}

	void DestroyContext()
	{
		delete ctx;
	}

	RenderContext* GetContext()
	{
		return ctx;
	}

	void SetCurrentContext(RenderContext* context)
	{
		ctx = context;
	}

	void SetDefaultRenderTarget(r<Target> target)
	{
		ctx->default_target = target;
	}

	void SetClearColor(Color color)
	{
		ctx->clear_color = color;
	}

	void SetRenderTarget(r<Target> target)
	{
		if (!target)
		{
			if (!ctx->default_target) // set target to screen
			{
				gl(glBindFramebuffer(GL_FRAMEBUFFER, 0));
				gl(glViewport(0, 0, ctx->window_width, ctx->window_height));

				return; // exit
			}

			// or use the default override
			target = ctx->default_target;
		}

		target->Use();
		gl(glViewport(0, 0, target->Width(), target->Height()));
	}

	void ClearRenderTarget()
	{
		ClearRenderTarget(ctx->clear_color);
	}

	void ClearRenderTarget(Color color)
	{
		gl_SetClearColor(color);
		gl(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		//gl_SetClearColor(Color(0));
	}

	void SetAlphaBlend(bool blend)
	{
		gl(glBlendFunc(GL_SRC_ALPHA, blend ? GL_ONE_MINUS_SRC_ALPHA : GL_ONE));
	}
}