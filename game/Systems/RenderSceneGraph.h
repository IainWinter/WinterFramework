#pragma once

#include "Leveling.h"
#include "Rendering.h"
#include "ext/rendering/SceneGraph.h"
#include "ext/rendering/Camera.h"
#include "ext/rendering/Sprite.h"
#include "ext/rendering/Particle.h"
#include "GameRender.h"

struct Stage_ClearTarget : RenderStage
{
	Color color;

	Stage_ClearTarget(Color color)
		: color (color)
	{}

	void Draw() override
	{
		Target::Clear(color);
	}
};

struct Stage_Meshes : RenderStage
{
	void Draw() override
	{
		program->Set("projection", level->GetApp()->GetModule<Camera>().Projection());

		for (auto [transform, mesh] : level->GetWorld()->Query<Transform2D, Mesh>())
		{
			program->Set("model", transform.World());
			mesh.Draw(mesh.topology);
		}
	}
};

struct Stage_Sprites : RenderStage
{
private:
	r<Mesh> m_quad;

public:
	Stage_Sprites()
	{
		m_quad = GetQuadMesh2D();
	}

	void Draw() override
	{
		program->Set("projection", level->GetApp()->GetModule<Camera>().Projection());
		program->Set("uvOffset", vec2(0.f, 0.f));
		program->Set("uvScale",  vec2(1.f, 1.f));

		for (auto [transform, sprite] : level->GetWorld()->Query<Transform2D, Sprite>())
		{
			program->Set("model",  transform.World());
			program->Set("sprite", sprite.Get());
			m_quad->Draw();
		}
	}
};

struct Stage_Particles : RenderStage
{
private:
	r<Mesh> m_quad;

public:
	Stage_Particles()
	{
		m_quad = GetQuadMesh2D();
	}

	void Draw() override
	{
		program->Set("projection", level->GetApp()->GetModule<Camera>().Projection());

		for (auto [transform, particle] : level->GetWorld()->Query<Transform2D, Particle>())
		{
			program->Set("model",    transform.World());
			program->Set("sprite",   particle.GetTexture());
			program->Set("uvOffset", particle.GetCurrentFrameUV().uvOffset);
			program->Set("uvScale",  particle.GetCurrentFrameUV().uvScale);

			m_quad->Draw();
		}
	}
};

struct Stage_SandSpriteInfo : RenderStage
{
private:
	r<Mesh> m_quad;

public:
	Stage_SandSpriteInfo()
	{
		m_quad = GetQuadMesh2D();
	}

	void Draw() override
	{
		auto [camera, sand] = level->GetApp()->GetModules<Camera, SandWorld>();
		program->Set("projection", camera.Projection());

		for (auto [entity, transform, sandSprite] : level->GetWorld()->QueryWithEntity<Transform2D, SandSprite>())
		{
			program->Set("model",         transform.World());
			program->Set("colliderMask", *sandSprite.colliderMask);
			program->Set("spriteSize",    sandSprite.colliderMask->Dimensions());
			program->Set("spriteIndex",   entity.Id());

			m_quad->Draw();
		}
	}
};

struct Stage_SendTargetToHost : RenderStage
{
	void Draw() override
	{
		target->SendToHost();
	}
};

struct System_RenderSceneGraph : SystemBase
{
	SceneGraph graph;

	void Init()
	{
		graph = SceneGraph(LevelManager::CurrentLevel());

		r<Target> collisionInfo = LevelManager::CurrentLevel()->GetApp()->GetModule<SandWorld>().screenRead;
		
		RenderStage*  clearScreen = new Stage_ClearTarget(Color(22, 22, 22));
		RenderStage*  meshes      = new Stage_Meshes();
		RenderStage*  sprites     = new Stage_Sprites();
		RenderStage*  particles   = new Stage_Particles();

		RenderStage*  clearSandSprites = new Stage_ClearTarget(Color(0, 0, 0, 0));
		RenderStage*  sandSprites      = new Stage_SandSpriteInfo();
		RenderStage* getCollisionInfo  = new Stage_SendTargetToHost();

		graph.AddStage(clearScreen, nullptr);
		graph.AddStage(meshes,      nullptr, GetProgram_Wireframe());
		graph.AddStage(sprites,     nullptr, GetProgram_Sprite());
		graph.AddStage(particles,   nullptr, GetProgram_Sprite());
		
		graph.AddStage(clearSandSprites, collisionInfo);
		graph.AddStage(sandSprites,      collisionInfo, GetProgram_SandSpriteInfo());
		graph.AddStage(getCollisionInfo, collisionInfo);
	}

	void Update()
	{
		graph.Execute();
	}
};