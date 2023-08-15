#include "v2/RenderSystem.h"
#include "util/error_check.h"

#include "RenderSystemTranslation.h"

v2Index::v2Index()
	: width            (0)
	, height           (0)
	, depth            (0)
	, elementsPerIndex (0)
	, bytesPerElement  (0) 
{}

v2Index::v2Index(int width, int elementsPerIndex, int bytesPerElement)
	: width            (width)
	, height           (1)
	, depth            (1)
	, elementsPerIndex (elementsPerIndex)
	, bytesPerElement  (bytesPerElement) 
{}

v2Index::v2Index(int width, int height, int elementsPerIndex, int bytesPerElement)
	: width            (width)
	, height           (height)
	, depth            (1)
	, elementsPerIndex (elementsPerIndex)
	, bytesPerElement  (bytesPerElement) 
{}

v2Index::v2Index(int width, int height, int depth, int elementsPerIndex, int bytesPerElement)
	: width            (width)
	, height           (height)
	, depth            (depth)
	, elementsPerIndex (elementsPerIndex)
	, bytesPerElement  (bytesPerElement) 
{}

int v2Index::length() const {
	return width * height * depth;
}

int v2Index::bytes() const {
	return width * height * depth * elementsPerIndex * bytesPerElement;
}

int v2Index::index(int x) const {
	return x * elementsPerIndex * bytesPerElement;
}

int v2Index::index(int x, int y) const {
	return (x + y * width) * elementsPerIndex * bytesPerElement;
}

int v2Index::index(int x, int y, int z) const {
	return (x + y * width + z * width * height) * elementsPerIndex * bytesPerElement;
}

std::tuple<int, int> v2Index::xy(int index) const {
	int x = index % width;
	int y = (index / width) % height;
	return std::make_tuple(x, y);
}

std::tuple<int, int, int> v2Index::xyz(int index) const {
	int x = index % width;
	int y = (index / width) % height;
	int z = (index / (width * height)) % depth;
	return std::make_tuple(x, y, z);
}

v2TextureResource::v2TextureResource(int width, int height, v2TextureFormat format, v2TextureFilter filter)
	: m_format   (format)
	, m_filter   (filter)
	, gl_handle (0)
{
	switch (format)
	{
		case v2TextureFormatR:
			m_index = v2Index(width, height, 1, 1);
			break;
		case v2TextureFormatRG:
			m_index = v2Index(width, height, 1, 2);
			break;
		case v2TextureFormatRGB:
			m_index = v2Index(width, height, 1, 3);
			break;
		case v2TextureFormatRGBA:
			m_index = v2Index(width, height, 1, 4);
			break;
		case v2TextureFormatFloat32:
			m_index = v2Index(width, height, 4, 1);
			break;
		case v2TextureFormatInt32:
			m_index = v2Index(width, height, 4, 1);
			break;
		case v2TextureFormatDepth:
		case v2TextureFormatStencil:
			assert(false && "Not implemented");
			break;
		default:
			assert(false && "Invalid texture format");
			break;
	}

	m_data = v2Array<char>(m_index.bytes());
}

void v2TextureResource::copyToDevice() 
{
	// set this based on the constructor called
	GLuint glTarget = GL_TEXTURE_2D;
	GLenum glFilter = gl_filter(m_filter);
	GLenum glIformat = gl_iformat(m_format);
	GLenum glFormat = gl_format(m_format);
	GLenum glType = gl_type(m_format);
	GLint glWidth = m_index.width;
	GLint glHeight = m_index.height;
	GLint glDepth = m_index.depth;
	const void* glData = m_data.data();

	glGenTextures(1, &gl_handle);
	glBindTexture(glTarget, gl_handle);

	glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, glFilter);
	glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, glFilter);

	switch (glTarget)
	{
		case GL_TEXTURE_1D:
			gl(glTexImage1D(glTarget, 0, glIformat, glWidth, 0, glFormat, glType, glData));
			break;
		case GL_TEXTURE_2D:
			gl(glTexImage2D(glTarget, 0, glIformat, glWidth, glHeight, 0, glFormat, glType, glData));
			break;
		case GL_TEXTURE_3D:
			gl(glTexImage3D(glTarget, 0, glIformat, glWidth, glHeight, glDepth, 0, glFormat, glType, glData));
			break;
	}
}

void v2TextureResource::copyFromDevice() 
{
	GLuint glTarget = GL_TEXTURE_2D;
	GLenum glFormat = gl_format(m_format);
	GLenum glType = gl_type(m_format);
	void* glData = m_data.data();

	glBindTexture(glTarget, gl_handle);
	glGetTexImage(glTarget, 0, glFormat, glType, glData);
}

void v2TextureResource::eraseFromDevice()
{
	glDeleteTextures(1, &gl_handle);
}

Color* v2TextureResource::pixels() {
	return (Color*)m_data.data();
}

const Color* v2TextureResource::pixels() const {
	return (const Color*)m_data.data();
}

Color& v2TextureResource::at(int index) {
	Color* colors = pixels();
	int idx = m_index.index(index);
	return colors[idx];
}

const Color& v2TextureResource::at(int index) const {
	const Color* colors = pixels();
	int idx = m_index.index(index);
	return colors[idx];
}

void v2TextureResource::set(int index, Color color) {
	Color* colors = pixels();
	int idx = m_index.index(index);
	colors[idx] = color;
}

int v2TextureResource::length() const {
	return m_index.length();
}

int v2TextureResource::bytes() const {
	return m_index.bytes();
}

int v2TextureResource::index(int x) const {
	return m_index.index(x);
}

int v2TextureResource::index(int x, int y) const {
	return m_index.index(x, y);
}

int v2TextureResource::index(int x, int y, int z) const {
	return m_index.index(x, y, z);
}

std::tuple<int, int> v2TextureResource::xy(int index) const {
	return m_index.xy(index);
}

std::tuple<int, int, int> v2TextureResource::xyz(int index) const {
	return m_index.xyz(index);
}

char* v2TextureResource::data() {
	return m_data.data();
}

const char* v2TextureResource::data() const {
	return m_data.data();
}

v2Texture::v2Texture(v2TextureResource* resource)
	: resource (resource)
{}

void v2Texture::copyToDevice() {
	resource->copyToDevice();
}

void v2Texture::copyFromDevice() {
	resource->copyFromDevice();
}

void v2Texture::eraseFromDevice() {
	resource->eraseFromDevice();
}

Color* v2Texture::pixels() {
	return resource->pixels();
}

const Color* v2Texture::pixels() const {
	return resource->pixels();
}

Color& v2Texture::at(int index) {
	return resource->at(index);
}

const Color& v2Texture::at(int index) const {
	return resource->at(index);
}

void v2Texture::set(int index, Color color) {
	return resource->set(index, color);
}

int v2Texture::length() const {
	return resource->length();
}

int v2Texture::bytes() const {
	return resource->bytes();
}

int v2Texture::index(int x) const {
	return resource->index(x);
}

int v2Texture::index(int x, int y) const {
	return resource->index(x, y);
}

int v2Texture::index(int x, int y, int z) const {
	return resource->index(x, y, z);
}

std::tuple<int, int> v2Texture::xy(int index) const {
	return resource->xy(index);
}

std::tuple<int, int, int> v2Texture::xyz(int index) const {
	return resource->xyz(index);
}

char* v2Texture::data() {
	return resource->data();
}

const char* v2Texture::data() const {
	return resource->data();
}