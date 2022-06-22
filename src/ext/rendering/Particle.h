#pragma once

#include "ext/rendering/TextureAtlas.h"

struct Particle
{
	r<TextureAtlas> atlas;
	int frameCount;
	int repeatCount;
	int frameCurrent;

	Particle()
		: atlas        (nullptr)
		, frameCount   (0)
		, repeatCount  (1)
		, frameCurrent (0)
	{}

	Particle(r<TextureAtlas> atlas)
		: atlas        (atlas)
		, frameCount   (atlas->GetFrameCount())
		, repeatCount  (1)
		, frameCurrent (0)
	{}

	Particle(r<TextureAtlas> atlas, int frameCount)
		: atlas        (atlas)
		, frameCount   (frameCount)
		, repeatCount  (1)
		, frameCurrent (0)
	{}

	void TickFrame(int skipFrame = 1)
	{
		frameCurrent += skipFrame;
	}

	bool EndOfLife() const
	{
		return frameCurrent / frameCount >= repeatCount;
	}

	const TextureAtlas::Bounds& GetCurrentFrameUV() const
	{
		return atlas->GetUVForFrame(frameCurrent % frameCount);
	}

	Texture& GetTexture()
	{
		return *atlas->source;
	}
};