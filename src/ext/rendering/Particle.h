#pragma once

#include "Entity.h"
#include "ext/rendering/TextureAtlas.h"

struct Particle
{
	a<TextureAtlas> atlas;
	int m_emitterSpawnerIndex = -1;

	float framesPerSecond = 30.f;
	float frameCurrent = 0.f;

	float life = 1.f;        // counts down to 0
	float lifeCurrent = 1.f; 

	Transform2D original;

	std::vector<Color> tints;
	Color tint;

	Particle() {}
	
	Particle(
		a<TextureAtlas> atlas, int fixedFrame = -1
	)
		: atlas           (atlas)
		, frameCurrent    (fixedFrame == -1 ? 0.f  : (float)fixedFrame)
		, framesPerSecond (fixedFrame == -1 ? 30.f : 0.f)
	{}

	Particle(
		const std::string& path, int tilesX = 1, int tilesY = 1, int fixedFrame = -1
	)
		: frameCurrent    (fixedFrame == -1 ? 0.f : (float)fixedFrame)
		, framesPerSecond (fixedFrame == -1 ? 30.f : 0.f)
	{
		atlas = Asset::Make<TextureAtlas>(path, path);
		atlas->SetAutoTile(tilesX, tilesY);
	}

	Particle& SetAtlas           (const r<TextureAtlas>&    atlas)           { this->atlas           = atlas;           return *this; }
	Particle& SetFramesPerSecond (float                     framesPerSecond) { this->framesPerSecond = framesPerSecond; return *this; }
	Particle& SetFrameCurrent    (float                     frameCurrent)    { this->frameCurrent    = frameCurrent;    return *this; }
	Particle& SetLife            (float                     life)            { this->life            = life;            return *this; }
	Particle& SetLifeCurrent     (float                     lifeCurrent)     { this->lifeCurrent     = lifeCurrent;     return *this; }
	Particle& SetOriginal        (const Transform2D&        transform)       { this->original        = transform;       return *this; }
	Particle& SetTint            (const Color&              tint)            { this->tint            = tint;            return *this; }
	Particle& SetTints           (const std::vector<Color>& tints)           { this->tints           = tints;           return *this; }
	Particle& AddTint            (const Color&              tint)            { this->tints.push_back(tint);             return *this; }

	Particle& SetLifetime(float lifetime)
	{
		SetLife(lifetime);
		SetLifeCurrent(lifetime);
		return *this;
	}

	Particle& SetFixedFrame(int frame)
	{
		SetFrameCurrent((float)frame);
		SetFramesPerSecond(0.f);
		return *this;
	}

	Particle& SetFixedFrameRandom()
	{
		SetFixedFrame(get_rand(atlas->GetFrameCount()));
		return *this;
	}

	void TickFrame(float dt)
	{
		frameCurrent += dt * framesPerSecond;
		lifeCurrent  -= dt;
	}

	bool  HasAtlas()        const { return atlas.IsLoaded(); }
	bool  EndOfLife()       const { return lifeCurrent <= 0.f; }
	float Age()             const { return life == 0.f ? 0.f : lifeCurrent / life; }
	float AgeLeft()         const { return 1.f - Age(); }
	int   GetCurrentFrame() const { return (int)floor(frameCurrent); }

	const TextureAtlas::Bounds& GetCurrentFrameUV() const { return atlas->GetUVForFrame(GetCurrentFrame() % atlas->GetFrameCount()); }
	      Texture&              GetTexture()              { return *atlas->source; }

	Color GetTint() const
	{
		return GetTint(AgeLeft());
	}

	Color GetTint(float forceAgeLeft) const
	{
		forceAgeLeft = clamp(forceAgeLeft, 0.f, 1.f);

		if (tints.size() == 0)   return tint;
		if (tints.size() == 1)   return tint * tints.back();
		if (forceAgeLeft >= 1.f) return tint * tints.back();

		float scaledTime = forceAgeLeft * (float)(tints.size() - 1);
		int indexA = (int)scaledTime;
		int indexB = indexA + 1;

		if (indexB >= tints.size())
		{
			log_render(
				"e~Index out of bounds in GetTint. Shouldn't be possible\n"
				"\tIndexA: %d IndexB: %d Number of tints", 
				indexA, indexB, (int)tints.size()
			);

			return tint * tints.back();
		}

		Color oldColor = tints.at(indexA);
		Color newColor = tints.at(indexB);
		float newT = scaledTime - floor(scaledTime);

		return tint * lerp(oldColor, newColor, newT);
	}
};

inline Particle RandomParticle(const a<TextureAtlas>& atlas)
{
	return Particle(atlas, atlas->GetFrameCount());
}

//struct Particle_
//{
//	r<TextureAtlas> atlas;
//	int m_emitterSpawnerIndex = -1;
//
//	int   repeatCount     =  1;
//	int   frameCount      =  1;
//	float framesPerSecond = 24.f;
//	float frameCurrent    =  0.f;
//
//	float m_age = 1.f;
//
//	// could be in seperate component
//
//	Transform2D original;     // for linear effects over time
//	Color tint;               // overall tint
//
//	std::vector<Color> tints; // colors to fade between with AgeLeft
//
//	Particle() {}
//
//	Particle(r<TextureAtlas> atlas)
//		: atlas      (atlas)
//		, frameCount (atlas->GetFrameCount())
//	{}
//
//	Particle(r<TextureAtlas> atlas, int fixedFrame)
//		: atlas           (atlas)
//		, frameCount      (atlas->GetFrameCount())
//		, frameCurrent    (fixedFrame)
//		, framesPerSecond (0.f)
//	{}
//
//	Particle& SetAtlas           (r<TextureAtlas>&          atlas)           { this->atlas           = atlas;           return *this; }
//	Particle& SetRepeatCount     (int                       repeatCount)     { this->repeatCount     = repeatCount;     return *this; }
//	Particle& SetFrameCount      (int                       frameCount)      { this->frameCount      = frameCount;      return *this; }
//	Particle& SetFramesPerSecond (float                     framesPerSecond) { this->framesPerSecond = framesPerSecond; return *this; }
//	Particle& SetFrameCurrent    (float                     frameCurrent)    { this->frameCurrent    = frameCurrent;    return *this; }
//	Particle& SetOriginal        (const Transform2D&        transform)       { this->original        = transform;       return *this; }
//	Particle& SetTint            (const Color&              tint)            { this->tint            = tint;            return *this; }
//	Particle& SetTints           (const std::vector<Color>& tints)           { this->tints           = tints;           return *this; }
//	Particle& AddTint            (const Color&              tint)            { this->tints.push_back(tint);             return *this; }
//
//	void TickFrame(float dt)
//	{
//		frameCurrent += dt * framesPerSecond;
//	}
//
//	int   GetCurrentFrame() const { return (int)floor(frameCurrent); }
//	bool  EndOfLife()       const { return Age() >= 1.f; }
//	float Age()             const { return frameCurrent / float(frameCount * repeatCount); }
//	float AgeLeft()         const { return 1.f - Age(); }
//	bool  HasAtlas()        const { return atlas != nullptr; }
//
//	const TextureAtlas::Bounds& GetCurrentFrameUV() const { return atlas->GetUVForFrame(GetCurrentFrame() % frameCount); }
//	                   Texture& GetTexture()              { return *atlas->source; }
//
//	Color GetTint() const
//	{
//		return GetTint(Age());
//	}
//
//	Color GetTint(float forceAgeLeft) const
//	{
//		if (tints.size() == 0)   return Color(255, 255, 255, 255);
//		if (tints.size() == 1)   return tints.back();
//		if (forceAgeLeft >= 1.f) return tints.back();
//
//		float scaledTime = forceAgeLeft * (float)(tints.size() - 1);
//		Color oldColor = tints.at((int)scaledTime);
//		Color newColor = tints.at((int)(scaledTime + 1.f));
//		float newT = scaledTime - floor(scaledTime);
//
//		return lerp(oldColor, newColor, newT);
//	}
//};

struct ParticleScaleWithAge
{
	vec2 scale = vec2(0.f);

	ParticleScaleWithAge() : scale(0.f) {}

	ParticleScaleWithAge(float x) : scale(x) {}
	ParticleScaleWithAge(vec2 x)  : scale(x) {}
};

struct ParticleSpawner
{
	Particle particle;
	float weight;
	std::function<void(Entity)> onCreate;
};

struct ParticleEmitter
{
	std::vector<ParticleSpawner> spawners;

	float timeBetweenSpawn = .0f;
	float currentTime = 0.f;

	bool enableAutoEmit = true;

	void AddSpawner(Particle particle, float weight, std::function<void(Entity)> onCreate = {})
	{
		spawners.push_back({ particle, weight, onCreate });
	}

	void RemoveSpawner(int index)
	{
		auto itr = spawners.begin() + index;
		spawners.erase(itr);
	}

	// returns a particle at the index
	Particle Emit(int index, vec2 position) const
	{
		const ParticleSpawner& spawner = spawners.at(index);
		
		Particle p = spawner.particle;
		p.original.position += position; // *= emitterTransform;            // todo: should parent when that is a feature
		p.m_emitterSpawnerIndex = index;

		return p;
	}

	// returns a random particle with their weighted probability
	Particle Emit(vec2 position) const
	{
		float totalWeight = 0.f;

		for (int i = 0; i < spawners.size(); i++) // this sucks
		{
			totalWeight += spawners.at(i).weight;
		}

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
	std::vector<Particle> EmitLine(vec2 a, vec2 b, float tickSize) const
	{
		std::vector<Particle> out;

		vec2 delta = b - a;
		float ticks = length(delta) / tickSize;
		vec2 deltaTick = delta / ticks;

		for (float t = 0; t < ticks; t += 1.f)
		{
			a += deltaTick;
			out.push_back(Emit(a));
		}

		return out;
	}
};

// line particles

struct LineParticle
{
	vec2 front = vec2(1.f, 0.f);
	vec2 back  = vec2(0.f, 0.f);
	Color colorFront = Color(255, 255, 255);
	Color colorBack  = Color(0, 0, 0);

	LineParticle& SetFront      (vec2 front)  { this->front      = front; return *this; };
	LineParticle& SetBack       (vec2 back)   { this->back       = back;  return *this; };
	LineParticle& SetColorFront (Color color) { this->colorFront = color; return *this; };
	LineParticle& SetColorBack  (Color color) { this->colorBack  = color; return *this; };

	LineParticle& SetColor(Color color) 
	{ 
		SetColorFront(color); 
		SetColorBack(color);
		return *this;
	};
};