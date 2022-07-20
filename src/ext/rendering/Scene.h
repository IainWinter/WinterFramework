#pragma once

#include "Rendering.h"
#include "app/System.h"

// I guess this is when a scene graph comes into play
// each stage has a Target and a ShaderProgram 

struct Scene;

// these are too similar to systems

struct RenderStage
{
	r<Level> level;
	r<ShaderProgram> program;
	r<Target> target;
	const char* name;

	friend struct Scene;

public:
	virtual void Draw() = 0;
	virtual void UI() {}
};

struct Scene
{
	std::vector<RenderStage*> stages;

private:
	r<Level> m_level;

public:
	Scene() = default;

	Scene(r<Level> level)
		: m_level (level)
	{}

	void AddStage(const char* name, RenderStage* stage, r<Target> target, r<ShaderProgram> program = nullptr)
	{
		stage->level = m_level;
		stage->target = target;
		stage->program = program;
		stage->name = name;

		stages.push_back(stage);
	}
};
