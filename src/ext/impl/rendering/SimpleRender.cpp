#include "ext/rendering/SimpleRender.h"

// mesh
r<ShaderProgram> meshProgram;

// sprites
std::unordered_map<int, Color> spriteColors;
ShowEntityIdMode drawEntityIdMode;
BatchSpriteRenderer spriteRender;

// lines
BatchLineRenderer lineRender;

// assets
r<Mesh> g_quad;
r<ShaderProgram> g_wireframe;
r<ShaderProgram> g_sprite;
r<ShaderProgram> g_collisionInfo;
r<ShaderProgram> g_debug_displayCollisionInfo;

void SetDrawEntityDebugMode(ShowEntityIdMode mode)
{
	drawEntityIdMode = mode;
}

void RenderMeshes(const Camera& camera, EntityWorld& world)
{
	meshProgram->Use();
	meshProgram->Set("projection", camera.Projection());

	for (auto [transform, mesh] : world.Query<Transform2D, Mesh>())
	{
		meshProgram->Set("model", transform.World());
		mesh.Draw(mesh.topology);
	}

	for (auto [transform, model] : world.Query<Transform2D, Model>())
	{
		meshProgram->Set("model", transform.World());
		for (r<Mesh>& mesh : model.meshes)
		{
			mesh->Draw(mesh->topology);
		}
	}
}

void RenderSprites(const Camera& camera, EntityWorld& world)
{
	spriteRender.Begin(camera, drawEntityIdMode == ONLY);

	// draw sprites

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
	for (auto [entity, transform, sprite] : world.QueryWithEntity<Transform2D, Sprite>())
	{
		// give sprites a tint based on their entity id

		Color tint = sprite.tint;
			
		if (drawEntityIdMode != NONE)
		{
			auto itr = spriteColors.find(entity.Id());
			if (itr == spriteColors.end())
			{
				itr = spriteColors.emplace(entity.Id(), Color::rand(255)).first;
			}

			switch (drawEntityIdMode)
			{
				case MIX:
					tint *= itr->second;
					break;
				case ONLY:
					tint = itr->second;
					break;
			}
		}

		spriteRender.SubmitSprite(transform, sprite.source, vec2(0.f, 0.f), vec2(1.f, 1.f), tint);
	}
	spriteRender.Draw();

	// draw particles

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	for (auto [t, p] : world.Query<Transform2D, Particle>())
	{
		if (p.HasAtlas())
		{
			const TextureAtlas::Bounds& uv = p.GetCurrentFrameUV();
			spriteRender.SubmitSprite(t, p.atlas->source, uv.uvOffset, uv.uvScale, p.GetTint());
		}

		else
		{
			spriteRender.SubmitSprite(t, p.GetTint());
		}
	}

	spriteRender.Draw();
}

void RenderLines(const Camera& camera, EntityWorld& world)
{
	lineRender.Begin();

	for (auto [transform, line] : world.Query<Transform2D, LineParticle>())
	{
		lineRender.SubmitLine(transform, line.back, line.front, line.colorBack, line.colorFront);
	}

	lineRender.Draw(camera);
}

r<Mesh>          GetQuadMesh2D()             { return g_quad; }
r<ShaderProgram> GetProgram_Sprite()         { return g_sprite; }
r<ShaderProgram> GetProgram_Wireframe()      { return g_wireframe; }
r<ShaderProgram> GetProgram_SandSpriteInfo() { return g_collisionInfo; }

r<ShaderProgram> GetProgram_Debug_DisplayCollisionInfo() { return g_debug_displayCollisionInfo; }

Mesh& InitQuadMesh2D(Mesh& mesh)
{
	mesh.Add<vec2>(Mesh::aPosition,     { vec2(-1, -1), vec2(1, -1), vec2(1, 1), vec2(-1, 1) });
	mesh.Add<vec2>(Mesh::aTextureCoord, { vec2( 0,  0), vec2(1,  0), vec2(1, 1), vec2( 0, 1) });
	mesh.Add<int> (Mesh::aIndexBuffer,  { 0, 1, 2, 0, 2, 3 });

	return mesh;
}

r<ShaderProgram> MakeVertexAndFragmentShader(const char* source_vert, const char* source_frag)
{
	r<ShaderProgram> program = mkr<ShaderProgram>();
	program->Add(ShaderProgram::sVertex, source_vert);
	program->Add(ShaderProgram::sFragment, source_frag);
	return program;
}

void InitWireframeProgram()
{
	const char* source_vert =
		"#version 330 core\n"

		"layout (location = 0) in vec2 vertex;"
		"layout (location = 5) in vec4 color;"

		"uniform mat4 model;"
		"uniform mat4 projection;"

		"out vec4 vertColor;"

		"void main()"
		"{"
			"vertColor = color;"
			"gl_Position = projection * model * vec4(vertex, 3.0, 1.0);" // hack: draw wireframes above
		"}";

	const char* source_frag =
		"#version 330 core\n"
		"in vec4 vertColor;"
		"out vec4 color;"
		"void main()"
		"{"
			"color = vertColor;"
		"}";

	g_wireframe = MakeVertexAndFragmentShader(source_vert, source_frag);

}

void InitSpriteProgram()
{
	const char* source_vert = 
		"#version 330 core\n"
		"layout (location = 0) in vec2 pos;"
		"layout (location = 1) in vec2 uv;"

		"out vec2 TexCoords;"

		"uniform mat4 model;"
		"uniform mat4 projection;"
		"uniform vec2 uvOffset;"
		"uniform vec2 uvScale;"

		"void main()"
		"{"
			"TexCoords = uv * uvScale + uvOffset;"
			"gl_Position = projection * model * vec4(pos, 0.0, 1.0);"
		"}";

	const char* source_frag = 
		"#version 330 core\n"
		"in vec2 TexCoords;"

		"out vec4 color;"

		"uniform sampler2D sprite;"
		"uniform vec4 tint = vec4(1, 1, 1, 1);"

		"void main()"
		"{"
			"vec4 spriteColor = tint * texture(sprite, TexCoords);"

			"if (spriteColor.a > .7) spriteColor.a = 1.f;"
			"else                    discard;"

			"color = spriteColor;"
		"}";

	g_sprite = MakeVertexAndFragmentShader(source_vert, source_frag);
}

void InitCollisionInfoProgram()
{
	const char* source_vert =
		"#version 330 core\n"
		"layout (location = 0) in vec2 pos;"
		"layout (location = 1) in vec2 uv;"

		"out vec2 TexCoords;"

		"uniform mat4 model;"
		"uniform mat4 projection;"

		"void main()"
		"{"
			"TexCoords = uv;"
			"gl_Position = projection * model * vec4(pos, 0.0, 1.0);"
		"}";

	const char* source_frag =
		"#version 330 core\n"
		"in vec2 TexCoords;"

		"out ivec4 spriteId;" // sprite (x, y) (entity index), (alpha for if its even there)

		"uniform sampler2D colliderMask;"
		"uniform vec2 spriteSize;"
		"uniform uint spriteIndex;"

		"void main()"
		"{"
			"float mask = texture(colliderMask, TexCoords).a;"
			"if (mask == 0) discard;"
			"spriteId = ivec4(TexCoords * spriteSize, spriteIndex, 1);"
		"}";

	g_collisionInfo = MakeVertexAndFragmentShader(source_vert, source_frag);
}

void Init_Debug_DisplayCollisionInfo()
{
	const char* source_vert =
		"#version 330 core\n"
		"layout (location = 0) in vec2 pos;"
		"layout (location = 1) in vec2 uv;"
		
		"out vec2 TexCoords;"
		
		"uniform mat4 model;"
		"uniform mat4 projection;"

		"void main()"
		"{"
			"TexCoords = uv;"
			"gl_Position = projection * model * vec4(pos, 0.0, 1.0);"
		"}";

	const char* source_frag =
		"#version 330 core\n"
		"in vec2 TexCoords;"
		
		"out vec4 color;"
		
		"uniform isampler2D sprite;"

		"void main()"
		"{"
			"ivec4 spriteColor = texture(sprite, TexCoords);"
			"if (spriteColor.a == 0) discard;"
			"color = vec4(1, 1, 1, 1);"
		"}";

	g_debug_displayCollisionInfo = MakeVertexAndFragmentShader(source_vert, source_frag);
}

void InitRendering()
{
	InitWireframeProgram();
	InitSpriteProgram();
	InitCollisionInfoProgram();
	Init_Debug_DisplayCollisionInfo();

	meshProgram = GetProgram_Wireframe();

	g_quad = mkr<Mesh>();
	InitQuadMesh2D(*g_quad);
}