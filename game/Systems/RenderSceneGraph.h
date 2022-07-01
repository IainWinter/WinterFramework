#pragma once

#include "Leveling.h"
#include "Rendering.h"
#include "ext/rendering/SceneGraph.h"
#include "ext/rendering/Camera.h"
#include "ext/rendering/Sprite.h"
#include "ext/rendering/Particle.h"
#include "ext/rendering/BatchSpriteRenderer.h"
#include "GameRender.h"

#include <algorithm>

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
		program->Set("tint",     vec4(1, 1, 1, 1));

		for (auto [transform, sprite] : level->GetWorld()->Query<Transform2D, Sprite>())
		{
			program->Set("model",  transform.World());
			program->Set("sprite", sprite.Get());
			m_quad->Draw();
		}
	}
};

struct Stage_NewSpriteRender : RenderStage
{
private:
	BatchSpriteRenderer render;

public:
	void Draw() override
	{
		render.Begin(level->GetApp()->GetModule<Camera>());

		// draw sprites

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		for (auto [transform, sprite] : level->GetWorld()->Query<Transform2D, Sprite>())
		{
			render.SubmitSprite(transform, sprite.source, vec2(0.f, 0.f), vec2(1.f, 1.f), Color(255, 255, 255, 255));
		}
		render.Draw();

		// draw particles

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		std::vector<std::tuple<float, Transform2D*, Particle*>> toDraw; // will be sorted by age
		for (auto [transform, particle] : level->GetWorld()->Query<Transform2D, Particle>())
		{
			toDraw.push_back({ particle.Age(), &transform, &particle });
		}
		std::sort(toDraw.begin(), toDraw.end(), [](const auto& a, const auto& b) { return std::get<0>(a) > std::get<0>(b); });
		for (auto& [age, t, p] : toDraw)
		{
			TextureAtlas::Bounds uv = p->GetCurrentFrameUV();
			render.SubmitSprite(*t, p->atlas->source, uv.uvOffset, uv.uvScale, p->GetTint(age));
		}
		render.Draw();
	}

	void UI()
	{
		ImGui::Begin("Particle System");
		int i = 0;

		for (auto [transform, particle] : level->GetWorld()->Query<Transform2D, Particle>())
		{
			i++;
		}

		ImGui::Text("particles rendered %d", i);
		ImGui::End();
	}
};

struct Stage_DebugSandSprites : RenderStage
{
private:
	r<Mesh> m_quad;
	r<Texture> m_source;

public:
	Stage_DebugSandSprites(r<Texture> source)
	{
		m_quad = GetQuadMesh2D();
		m_source = source;
	}

	void Draw() override
	{
		Camera& camera = level->GetApp()->GetModule<Camera>();

		program->Set("projection", camera.Projection());
		program->Set("model",      Transform2D(vec2(0), vec2(camera.w, camera.h)).World());
		program->Set("sprite",     *m_source);
		m_quad->Draw();
	}
};

//struct Stage_Particles : RenderStage
//{
//private:
//	r<Mesh> m_quad;
//
//public:
//	Stage_Particles()
//	{
//		m_quad = GetQuadMesh2D();
//	}
//
//	void Draw() override
//	{
//		program->Set("projection", level->GetApp()->GetModule<Camera>().Projection());
//
//		float z = 2.f;
//
//		for (auto [transform, particle] : level->GetWorld()->Query<Transform2D, Particle>())
//		{
//			Transform2D t = transform;
//			t.z = z;
//			z += .01f;
//
//			program->Set("model",    t.World());
//			program->Set("sprite",   particle.GetTexture());
//			program->Set("uvOffset", particle.GetCurrentFrameUV().uvOffset);
//			program->Set("uvScale",  particle.GetCurrentFrameUV().uvScale);
//			program->Set("tint",     vec4(1, 1, 1, 1/* - particle.Age()*/));
//
//			m_quad->Draw();
//		}
//	}
//};

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
		
		vec2 offset = sand.cameraSizeMeters / 2.f;

		Camera adjusted = camera;
		//adjusted.x -= offset.x;
		//adjusted.y -= offset.y;
		//adjusted.w *= 2;
		//adjusted.h *= 2;
		
		program->Set("projection", adjusted.Projection());

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
		
		RenderStage*  clearScreen = new Stage_ClearTarget(Color(22, 22, 22, 22));
		RenderStage*  meshes      = new Stage_Meshes();
		//RenderStage*  sprites     = new Stage_Sprites();
		//RenderStage*  particles   = new Stage_Particles();

		RenderStage*  debugDrawSandSprites = new Stage_DebugSandSprites(collisionInfo->Get(Target::aColor));

		RenderStage*  clearSandSprites = new Stage_ClearTarget(Color(0, 0, 0, 0));
		RenderStage*  sandSprites      = new Stage_SandSpriteInfo();
		RenderStage* getCollisionInfo  = new Stage_SendTargetToHost();

		graph.AddStage(clearScreen, nullptr);
		graph.AddStage(meshes,      nullptr, GetProgram_Wireframe());
		//graph.AddStage(sprites,     nullptr, GetProgram_Sprite());
		//graph.AddStage(particles,   nullptr, GetProgram_Sprite());
		
		graph.AddStage(new Stage_NewSpriteRender(), nullptr, nullptr);
		
		
		collisionInfo->Use();
		collisionInfo->Clear(Color(0, 0, 0, 0));

		graph.AddStage(clearSandSprites, collisionInfo);
		graph.AddStage(sandSprites,      collisionInfo, GetProgram_SandSpriteInfo());
		graph.AddStage(getCollisionInfo, collisionInfo);

		//graph.AddStage(debugDrawSandSprites, nullptr, GetProgram_Debug_DisplayCollisionInfo());
	}

	void Update()
	{
		graph.Execute();
	}

	void UI()
	{
		graph.ExecuteUI();
	}
};