#pragma once

#include "v2/Memory.h"
#include "v2/RenderSystemOptions.h"

#include "util/Color.h"

#include <vector>
#include <assert.h>

// This should create a scene graph which renders itself instead of being a
// thing that gets submitted to by the program

// that makes much more sense if the entities only store handles into this, just like the
// physics system

// I like the idea of using a centralized store for rendering resources

//	describes the layout of an array of data
//	uses row major ordering
//
struct v2Index
{
	int width;
	int height;
	int depth;
	int elementsPerIndex;
	int bytesPerElement;

	v2Index();
	v2Index(int width, int elementsPerIndex, int bytesPerElement);
	v2Index(int width, int height, int elementsPerIndex, int bytesPerElement);
	v2Index(int width, int height, int depth, int elementsPerIndex, int bytesPerElement);

	// return the number of elements in the index
	int length() const;

	// return the number of bytes in the index
	int bytes() const;

	int index(int x) const;
	int index(int x, int y) const;
	int index(int x, int y, int z) const;

	std::tuple<int, int> xy(int index) const;
	std::tuple<int, int, int> xyz(int index) const;
};

class v2ResourceInterface 
{
public:
	virtual ~v2ResourceInterface() {}
};

class v2DeviceResourceInterface : public v2ResourceInterface 
{
public:
	virtual ~v2DeviceResourceInterface() {}

	virtual void copyToDevice() = 0;
	virtual void copyFromDevice() = 0;
	virtual void eraseFromDevice() = 0;
};

class v2TextureResourceInterface : public v2DeviceResourceInterface
{
public:
	virtual Color& at(int index) = 0;
	virtual const Color& at(int index) const = 0;

	virtual void set(int index, Color color) = 0;

	virtual Color* pixels() = 0;
	virtual const Color* pixels() const = 0;

	virtual char* data() = 0;
	virtual const char* data() const = 0;

	virtual void clear(Color color) = 0;

	virtual int width() const = 0;
	virtual int height() const = 0;
	virtual int length() const = 0;
	virtual int bytes() const = 0;
	virtual int index(int x) const = 0;
	virtual int index(int x, int y) const = 0;
	virtual int index(int x, int y, int z) const = 0;
	virtual std::tuple<int, int> xy(int index) const = 0;
	virtual std::tuple<int, int, int> xyz(int index) const = 0;
};

class v2TextureResource : public v2DeviceResourceInterface 
{
public:
	v2TextureResource(int width, int height, v2TextureFormat format, v2TextureFilter filter);

	void copyToDevice() override;
	void copyFromDevice() override;
	void eraseFromDevice() override;

	Color& at(int index);
	const Color& at(int index) const;
	
	void set(int index, Color color);

	Color* pixels();
	const Color* pixels() const;

	// encapsulation pass through functions to data

	char* data();
	const char* data() const;

	// encapsulation pass through functions to index

	int length() const;
	int bytes() const;
	int index(int x) const;
	int index(int x, int y) const;
	int index(int x, int y, int z) const;
	std::tuple<int, int> xy(int index) const;
	std::tuple<int, int, int> xyz(int index) const;

private:
	v2Array<char> m_data;
	v2Index m_index;
	v2TextureFormat m_format;
	v2TextureFilter m_filter;
	unsigned int gl_handle;
};

// Texture
// supports only 2d textures for now
//
class v2Texture
{
public:
	v2Texture(v2TextureResource* resource);

	void copyToDevice();
	void copyFromDevice();
	void eraseFromDevice();

	Color* pixels();
	const Color* pixels() const;

	Color& at(int index);
	const Color& at(int index) const;

	void set(int index, Color color);

	// encapsulation pass through functions to index

	int length() const;
	int bytes() const;
	int index(int x) const;
	int index(int x, int y) const;
	int index(int x, int y, int z) const;
	std::tuple<int, int> xy(int index) const;
	std::tuple<int, int, int> xyz(int index) const;

	// encapsulation pass through functions to data

	char* data();
	const char* data() const;

private:
	v2TextureResource* resource;
};

void test_v2Texture() {
	v2TextureResource resource(10, 10, v2TextureFormatRGBA, v2TextureFilterPixel);
	v2Texture texture(&resource);

	texture.set(0, Color(1, 0, 0, 1));
	texture.set(1, Color(0, 1, 0, 1));
	texture.set(2, Color(0, 0, 1, 1));
	texture.set(3, Color(1, 1, 0, 1));
	texture.set(4, Color(1, 0, 1, 1));
	texture.set(5, Color(0, 1, 1, 1));
	texture.set(6, Color(1, 1, 1, 1));
	texture.set(7, Color(0, 0, 0, 1));
	texture.set(8, Color(0.5, 0.5, 0.5, 1));
	texture.set(9, Color(0.25, 0.25, 0.25, 1));

	texture.copyToDevice();
	texture.copyFromDevice();
	texture.eraseFromDevice();
}