#pragma once

//#include "Texture.h"
#include "Rendering.h"
#include "util/math.h"
#include <vector>

struct TextureCacheImg {
	vec2 scale;
	vec2 offset;
	int handle;
};

class TextureCache
{
private:
	struct Rect {
		int minX = 0, minY = 0, maxX = 0, maxY = 0;

		int width() { return maxX - minX + 1; }
		int height() { return maxY - minY + 1; }

		bool no_fit(int w, int h) {
			return width() < w && height() < h;
		}

		bool exactly_fits(int w, int h) {
			return width() == w && height() == h;
		}
	};

	struct Node {
		Node* child[2] = { nullptr, nullptr };
		Rect rect;
		int handle = 0;

		Node* AddRect(int width, int height);
		void PrintGraphviz();
	};

public:
	TextureCache() = default;
	TextureCache(int maxWidth, int maxHeight, int channels);

	TextureCacheImg Add(char* pixels, int width, int height, int channels);

	void SendToDevice();
	int GetTextureHandle() const;

private:
	r<Texture> cache;
	Node* root = nullptr;
	int nextHandle = 1;
};