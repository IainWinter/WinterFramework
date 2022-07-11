#pragma once

#include "ext/rendering/TextureAtlas.h"

struct Particle
{
	r<TextureAtlas> atlas;
	
	int   repeatCount     =  1;
	int   frameCount      =  1;
	float framesPerSecond = 24.f;
	float frameCurrent    =  0.f;

	// could be in seperate component

	Transform2D original;     // for linear effects over time
	Color tint;               // overall tint

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

	Particle& SetAtlas           (r<TextureAtlas>&          atlas)           { this->atlas           = atlas;           return *this; }
	Particle& SetRepeatCount     (int                       repeatCount)     { this->repeatCount     = repeatCount;     return *this; }
	Particle& SetFrameCount      (int                       frameCount)      { this->frameCount      = frameCount;      return *this; }
	Particle& SetFramesPerSecond (float                     framesPerSecond) { this->framesPerSecond = framesPerSecond; return *this; }
	Particle& SetFrameCurrent    (float                     frameCurrent)    { this->frameCurrent    = frameCurrent;    return *this; }
	Particle& SetOriginal        (const Transform2D&        transform)       { this->original        = transform;       return *this; }
	Particle& SetTint            (const Color&              tint)            { this->tint            = tint;            return *this; }
	Particle& SetTints           (const std::vector<Color>& tints)           { this->tints           = tints;           return *this; }
	Particle& AddTint            (const Color&              tint)            { this->tints.push_back(tint);             return *this; }


	void TickFrame(float dt)
	{
		frameCurrent += dt * framesPerSecond;
	}

	int   GetCurrentFrame() const { return (int)frameCurrent; }
	bool  EndOfLife()       const { return Age() >= 1.f; }
	float Age()             const { return frameCurrent / float(frameCount * repeatCount); }
	float AgeLeft()         const { return 1.f - Age(); }
	bool  HasAtlas()        const { return atlas != nullptr; }

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
	int _pad;
};

struct ParticleEmitter
{
	struct Spawner
	{
		Particle particle;
		float weight;
		std::function<void(Particle)> onCreate;
	};

	std::vector<Spawner> spawners;
	float totalWeight = 0.f;

	float timeBetweenSpawn = .016f;
	float currentTime = 0.f;

	bool enableAutoEmit = true;

	void AddSpawner(Particle particle, float weight, std::function<void(Particle)> onCreate)
	{
		spawners.push_back({ particle, weight, onCreate });
		totalWeight += weight;
	}

	void RemoveSpawner(int index)
	{
		auto itr = spawners.begin() + index;
		totalWeight -= itr->weight;
		spawners.erase(itr);
	}

	// returns a particle at the index
	Particle Emit(int index, vec2 position) const
	{
		const Spawner& spawner = spawners.at(index);
		
		Particle p = spawner.particle;
		p.original.position += position; // *= emitterTransform;            // todo: should parent when that is a feature

		if (spawner.onCreate)
		{
			spawner.onCreate(p);
		}

		return p;
	}

	// returns a random particle with their weighted probability
	Particle Emit(vec2 position) const
	{
		float pick = get_rand(totalWeight);

		int i = 0;
		for (; i < spawners.size(); i++)
		{
			auto& [particle, weight, _] = spawners.at(i);
			if (pick <= weight) break;
			pick -= weight;
		}

		return Emit(i, position);
	}

	// could return a vector of particles
	void EmitLine(vec2 a, vec2 b) const
	{
		vec2 delta = b - a;
		float ticks = length(delta) * 15.f;
		vec2 deltaTick = delta / ticks;

		for (float t = 0; t < ticks; t += 1.f)
		{
			a += deltaTick;
			Emit(a);
		}
	}
};