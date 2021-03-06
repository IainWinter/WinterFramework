#pragma once

#include "Rendering.h"
#include "app/System.h"
#include "Sand/Sand.h"
#include "ext/rendering/Camera.h"
#include "ext/rendering/Sprite.h"
#include "ext/rendering/Particle.h"
#include "ext/rendering/Model.h"
#include "ext/rendering/BatchSpriteRenderer.h"
#include "imgui/imgui.h"
#include <algorithm>

#include "GameRender.h"

struct System_Render_Meshes : SystemBase
{
private:
	r<ShaderProgram> program;

public:
	void Init()
	{
		program = GetProgram_Wireframe();
	}

	void Update()
	{
		program->Use();
		program->Set("projection", First<Camera>().Projection());

		for (auto [transform, mesh] : Query<Transform2D, Mesh>())
		{
			program->Set("model", transform.World());
			mesh.Draw(mesh.topology);
		}

		for (auto [transform, model] : Query<Transform2D, Model>())
		{
			program->Set("model", transform.World());
			for (r<Mesh>& mesh : model.meshes)
			{
				mesh->Draw(mesh->topology);
			}
		}
	}
};

struct System_Render_Sprites : SystemBase
{
private:
	BatchSpriteRenderer render;

public:
	void Update()
	{
		Target::UseDefault();

		glViewport(0, 0, GetWindow().Width(), GetWindow().Height());

		render.Begin(First<Camera>());

		// draw sprites

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		for (auto [transform, sprite] : Query<Transform2D, Sprite>())
		{
			render.SubmitSprite(transform, sprite.source, vec2(0.f, 0.f), vec2(1.f, 1.f), sprite.tint);
		}
		render.Draw();

		// draw particles

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		std::vector<std::tuple<float, Transform2D*, Particle*>> toDraw; // will be sorted by age
		for (auto [transform, particle] : Query<Transform2D, Particle>())
		{
			toDraw.push_back({ particle.Age(), &transform, &particle });
		}
		std::sort(toDraw.begin(), toDraw.end(), [](const auto& a, const auto& b) { return std::get<0>(a) > std::get<0>(b); });
		for (auto& [age, t, p] : toDraw)
		{
			if (p->HasAtlas())
			{
				TextureAtlas::Bounds uv = p->GetCurrentFrameUV();
				render.SubmitSprite(*t, p->atlas->source, uv.uvOffset, uv.uvScale, p->GetTint(age));
			}

			else
			{
				render.SubmitSprite(*t, p->GetTint(age));
			}
		}
		render.Draw();
	}
};

struct System_Render_SandCollisionInfo : SystemBase
{
private:
	r<Target> sandSpriteInfoTarget;
	r<ShaderProgram> sandSpriteInfoProgram;
	r<Mesh> m_quad;

public:
	void Init()
	{
		sandSpriteInfoTarget = First<SandWorld>().screenRead;
		sandSpriteInfoProgram = GetProgram_SandSpriteInfo();
		m_quad = GetQuadMesh2D();
	}

	void Update()
	{
		sandSpriteInfoTarget->Use();
		sandSpriteInfoTarget->Clear(Color(0, 0, 0, 0));

		sandSpriteInfoProgram->Use();
		sandSpriteInfoProgram->Set("projection", First<Camera>().Projection());

		glViewport(0, 0, sandSpriteInfoTarget->Width(), sandSpriteInfoTarget->Height());

		for (auto [entity, transform, sandSprite] : QueryWithEntity<Transform2D, SandSprite>())
		{
			sandSpriteInfoProgram->Set("model",         transform.World());
			sandSpriteInfoProgram->Set("colliderMask", *sandSprite.colliderMask);
			sandSpriteInfoProgram->Set("spriteSize",    sandSprite.colliderMask->Dimensions());
			sandSpriteInfoProgram->Set("spriteIndex",   entity.Id());

			m_quad->Draw();
		}

		sandSpriteInfoTarget->SendToHost(); // research how to make this async
	}
};