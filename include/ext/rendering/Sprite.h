#pragma once

#include "Rendering.h"
#include "ext/AssetStore.h"

struct Sprite
{
	a<Texture> source;

	vec2 uvOffset = vec2(0.f, 0.f);
	vec2 uvScale  = vec2(1.f, 1.f);

    Color tint = Color(255, 255, 255, 255);
    
	Sprite() {}
	Sprite(a<Texture> source) : source(source) {}
	Sprite(const Texture& sourceToCopy) : source(mkr<Texture>(sourceToCopy)) {}
	Texture& Get() { return *source.ptr(); }

    Sprite& SetSource(a<Texture> source) { this->source = source; return *this; }
    Sprite& SetTint(Color tint) { this->tint = tint; return *this; }
    Sprite& SetUvOffset(vec2 offset) { this->uvOffset = offset; return *this; }
    Sprite& SetUvScale(vec2 scale) { this->uvScale = scale; return *this; }
};

struct SpriteStaticHandle
{
    int sourceHandle;
    ivec2 dimensions;
    
    vec2 uvOffset = vec2(0.f, 0.f);
    vec2 uvScale  = vec2(1.f, 1.f);

    Color tint = Color(255, 255, 255, 255);
    
    SpriteStaticHandle& SetSource(a<Texture> source) {
        if (source->OnHost())
            source->SendToDevice();
        sourceHandle = source->DeviceHandle();
        dimensions = source->Dimensions();
        return *this;
        
    }
    SpriteStaticHandle& SetTint(Color tint) { this->tint = tint; return *this; }
    SpriteStaticHandle& SetUvOffset(vec2 offset) { this->uvOffset = offset; return *this; }
    SpriteStaticHandle& SetUvScale(vec2 scale) { this->uvScale = scale; return *this; }
};

struct SpriteMesh
{
    Mesh geometry;
    Sprite sprite;
    
    SpriteMesh(Mesh geometry, Sprite sprite)
        : geometry (geometry)
        , sprite   (sprite)
    {}
};
