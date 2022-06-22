#pragma once

#include "Rendering.h"
#include "Leveling.h"

// I guess this is when a scene graph comes into play
// each stage has a Target and a ShaderProgram 

struct SceneGraph;

struct RenderStage
{
	r<Level> level;
	r<ShaderProgram> program;
	r<Target> target;

	friend struct SceneGraph;

public:
	virtual void Draw() = 0;
};

struct SceneGraph
{
	std::vector<RenderStage*> stages;

private:
	r<Level> m_level;
	r<Target> m_lastTarget;
	r<ShaderProgram> m_lastProgram;

public:
	SceneGraph() = default;

	SceneGraph(r<Level> level)
		: m_level (level)
	{}

	void AddStage(RenderStage* stage, r<Target> target, r<ShaderProgram> program = nullptr)
	{
		stage->level = m_level;
		stage->target = target;
		stage->program = program;
		stages.push_back(stage);
	}

	void Execute()
	{
		if (stages.size() == 0) return;
		
		auto stage = stages.begin();

		// assume opengl state could be anything
		ForceUse(**stage);

		for (; stage != stages.end(); ++stage)
		{
			UseDiff(**stage);
			(*stage)->Draw();
		}
	}

private:
	void UseProgram(RenderStage& stage)
	{
		if (stage.program)
		{
			stage.program->Use();
		}
	}
	
	void UseTarget(RenderStage& stage)
	{
		if (stage.target)
		{
			stage.target->Use();
		}

		else
		{
			Target::UseDefault();
		}
	}
	
	void ForceUse(RenderStage& stage)
	{
		UseProgram(stage);
		UseTarget (stage);
	}

	void UseDiff(RenderStage& stage)
	{
		if (m_lastProgram != stage.program) UseProgram(stage);
		if (m_lastTarget  != stage.target)  UseTarget (stage);

		m_lastProgram = stage.program;
		m_lastTarget  = stage.target;
	}
};
