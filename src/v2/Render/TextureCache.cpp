#include "v2/Render/TextureCache.h"

// not done

TextureCache::TextureCache(int maxWidth, int maxHeight, int channels)
{
	//TextureLayout layout;
	//layout.width = maxWidth;
	//layout.height = maxHeight;
	//layout.format = format;

    cache = ref(Texture(maxWidth, maxHeight, Texture::uRGBA, false));
	//cache = v2Texture(layout, AccessHostDevice);

    root = new Node();
    root->rect = { 0, 0, maxWidth - 1, maxHeight - 1 };
}

TextureCacheImg TextureCache::Add(char* pixels, int width, int height, int channels)
{
    Node* node = root->AddRect(width, height);

    if (!node) // error, todo: handle this in some way
        throw nullptr;

    //TextureView cacheView = cache.host;
    //TextureLayout cacheLayout = cacheView.layout;

    for (int x = 0; x < width; x++)
    for (int y = 0; y < height; y++)
    {
        int from = (x + y * height) * channels;
        int to = (x + node->rect.minX + (y + node->rect.minY) * cache->Width()) * 4;

        for (int c = 0; c < channels; c++)
            cache->Pixels()[to + c] = pixels[from + c];
    }

    node->handle = nextHandle;
    nextHandle += 1;

    TextureCacheImg img;
    img.handle = node->handle;
    img.offset = vec2(node->rect.minX, node->rect.minY) / vec2(cache->Width(), cache->Height());
    img.scale = vec2(node->rect.width(), node->rect.height()) / vec2(cache->Width(), cache->Height());

    return img;
}

void TextureCache::SendToDevice() {
    cache->SendToDevice();
}

int TextureCache::GetTextureHandle() const {
    return cache->DeviceHandle();
}

TextureCache::Node* TextureCache::Node::AddRect(int width, int height)
{
    if (child[0] != nullptr && child[1] != nullptr)
    {
        // (try inserting into first child)
        Node* newNode = child[0]->AddRect(width, height);

        if (newNode != NULL)
            return newNode;

        // (no room, insert into second)
        return child[1]->AddRect(width, height);
    }
    else
        //  (if there's already a lightmap here, return)
        if (handle != 0)
            return nullptr;

        // (if we're too small, return)
        if (rect.no_fit(width, height))
            return nullptr;

        // (if we're just right, accept)
        if (rect.exactly_fits(width, height))
            return this;
        
        // (otherwise, gotta split this node and create some kids)
        child[0] = new Node();
        child[1] = new Node();
        
        //(decide which way to split)
        int dw = rect.width() - width;
        int dh = rect.height() - height;
        
        if (dw > dh) {
            child[0]->rect = Rect{ rect.minX,         rect.minY, rect.minX + width - 1, rect.maxY };
            child[1]->rect = Rect{ rect.minX + width, rect.minY, rect.maxX,             rect.maxY };
        }
        else {
            child[0]->rect = Rect { rect.minX, rect.minY,          rect.maxX, rect.minY + height - 1 };
            child[1]->rect = Rect { rect.minX, rect.minY + height, rect.maxX, rect.maxY };
        }

        //(insert into first child we created)
        return child[0]->AddRect(width, height);
}

void TextureCache::Node::PrintGraphviz()
{
    if (child[0] == nullptr)
        return;

    printf("\tn%d_%d__%d_%d_w%d_h%d__x%d -> n%d_%d__%d_%d_w%d_h%d__x%d\n", 
        rect.minX, rect.minY, rect.maxX, rect.maxY, rect.width(), rect.height(), handle,
        child[0]->rect.minX, child[0]->rect.minY, child[0]->rect.maxX, child[0]->rect.maxY, child[0]->rect.width(), child[0]->rect.height(), child[0]->handle
    );

    printf("\tn%d_%d__%d_%d_w%d_h%d__x%d -> n%d_%d__%d_%d_w%d_h%d__x%d\n",
        rect.minX, rect.minY, rect.maxX, rect.maxY, rect.width(), rect.height(), handle,
        child[1]->rect.minX, child[1]->rect.minY, child[1]->rect.maxX, child[1]->rect.maxY, child[1]->rect.width(), child[1]->rect.height(), child[1]->handle
    );

    child[0]->PrintGraphviz();
    child[1]->PrintGraphviz();
}
