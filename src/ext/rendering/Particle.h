#pragma once

#include "ext/rendering/TextureAtlas.h"

struct Particle
{
	r<TextureAtlas> atlas;
	
	int framesPerSecond = 24;
	int frameCount      =  0;
	int repeatCount     =  1;

	float frameCurrent  = 0.f;

	// could be in seperate component

	Transform2D orignal;      // for linear effects over time
	std::vector<Color> tints; // colors to fade between with AgeLeft

	Particle() {}

	Particle(r<TextureAtlas> atlas)
		: atlas      (atlas)
		, frameCount (atlas->GetFrameCount())
	{}

	Particle(r<TextureAtlas> atlas, int frameCount)
		: atlas      (atlas)
		, frameCount (frameCount)
	{}

	void TickFrame(float dt)
	{
		frameCurrent += dt * framesPerSecond;
	}

	int   GetCurrentFrame() const { return (int)frameCurrent; }
	bool  EndOfLife()       const { return Age() >= 1.f; }
	float Age()             const { return frameCurrent / float(frameCount * repeatCount); }
	float AgeLeft()         const { return 1.f - Age(); }

	const TextureAtlas::Bounds& GetCurrentFrameUV() const { return atlas->GetUVForFrame(GetCurrentFrame() % frameCount); }
	                   Texture& GetTexture()              { return *atlas->source; }

	Color GetTint() const
	{
		return GetTint(Age());
	}

	Color GetTint(float forceAgeLeft) const
	{
		if (tints.size() == 0)   return Color(255, 255, 255, 255);
		if (tints.size() == 1)   return tints.back();
		if (forceAgeLeft >= 1.f) return tints.back();

		float scaledTime = forceAgeLeft * (float)(tints.size() - 1);
		Color oldColor = tints.at((int)scaledTime);
		Color newColor = tints.at((int)(scaledTime + 1.f));
		float newT = scaledTime - floor(scaledTime);

		return lerp(oldColor, newColor, newT);
	}
};

struct ParticleShrinkWithAge
{
	float scale = 1.f;
};

struct ParticleEmitter
{
	struct Spawner
	{
		Particle particle;
		float weight;
	};

	std::vector<Spawner> spawners;
	float totalWeight = 0.f;

	float timeBetweenSpawn = .001f;
	float currentTime = 0.f;

	void AddSpawner(Particle particle, float weight)
	{
		spawners.push_back({ particle, weight });
		totalWeight += weight;
	}

	void RemoveSpawner(int index)
	{
		auto itr = spawners.begin() + index;
		totalWeight -= itr->weight;
		spawners.erase(itr);
	}

	// returns a particle at the index
	Particle Emit(int index, Transform2D& emitterTransform) const
	{
		Particle p = spawners.at(index).particle;
		p.orignal *= emitterTransform;            // todo: should parent when that is a feature
		return p;
	}

	// returns a random particle with their weighted probability
	Particle Emit(Transform2D& emitterTransform) const
	{
		float pick = get_rand(totalWeight);

		int i = 0;
		for (; i < spawners.size(); i++)
		{
			auto& [particle, weight] = spawners.at(i);
			if (pick <= weight) break;
			pick -= weight;
		}

		return Emit(i, emitterTransform);
	}
};