#include "ext/rendering/SimpleRender.h"
#include "util/error_check.h"

// mesh
r<ShaderProgram> meshProgram;

// sprites
std::unordered_map<int, Color> spriteColors;
ShowEntityIdMode drawEntityIdMode;
r<BatchSpriteRenderer> spriteRender;

// lines
r<BatchLineRenderer> lineRender;

// assets
r<Mesh> g_quad;
r<ShaderProgram> g_sprite;

void SetDrawEntityDebugMode(ShowEntityIdMode mode)
{
	drawEntityIdMode = mode;
}

Color TintToDebugMode(const Entity& entity, const Sprite& sprite)
{
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
            default:
                break;
        }
    }
    
    return tint;
}

void SetMeshProgramDefaults(const Transform2D& transform)
{
    meshProgram->Set("model", transform.World());
    meshProgram->Set("uvOffset", vec2(0.f));
    meshProgram->Set("uvScale", vec2(1.f));
    meshProgram->Set("sprite", spriteRender->GetDefaultTexture());
    meshProgram->Set("tint", Color(255, 255, 255));
}

void SetMeshProgramValues(const Transform2D& transform, Sprite& sprite)
{
    r<Texture> source = sprite.source ? (r<Texture>)sprite.source : spriteRender->GetDefaultTexture();
    
    meshProgram->Set("model", transform.World());
    meshProgram->Set("uvOffset", sprite.uvOffset);
    meshProgram->Set("uvScale", sprite.uvScale);
    meshProgram->Set("sprite", source);
    meshProgram->Set("tint", sprite.tint);
}

void SetMeshProgramCameraValues(const Camera& camera)
{
	meshProgram->Set("projection", camera.Projection());
	meshProgram->Set("view", camera.View());
}

void RenderMesh(const Camera& camera, const Transform2D& transform, Mesh& mesh)
{
	meshProgram->Use();
	SetMeshProgramCameraValues(camera);
	SetMeshProgramDefaults(transform);
	mesh.Draw();
}

void RenderMesh(const Camera& camera, const Transform2D& transform, Mesh& mesh, Sprite& sprite)
{
	meshProgram->Use();
	SetMeshProgramCameraValues(camera);
	SetMeshProgramValues(transform, sprite);
	mesh.Draw();
}

void RenderMeshes(const Camera& camera, EntityWorld& world)
{
	meshProgram->Use();
	SetMeshProgramCameraValues(camera);

	for (auto [transform, mesh] : world.Query<Transform2D, Mesh>())
	{
        SetMeshProgramDefaults(transform);
        mesh.Draw();
	}

	for (auto [transform, model] : world.Query<Transform2D, Model>())
	{
        SetMeshProgramDefaults(transform);
		model.Draw();
    }
    
    for (auto [transform, mesh] : world.Query<Transform2D, SpriteMesh>())
    {
        SetMeshProgramValues(transform, mesh.sprite);
        mesh.geometry.Draw();
    }
}

void RenderSprites(BatchSpriteRenderer& render, const Camera& camera, EntityWorld& world)
{
	render.Begin();

	// draw sprites

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
	for (auto [entity, transform, sprite] : world.QueryWithEntity<Transform2D, Sprite>())
	{
		render.SubmitSprite(transform, sprite.source, sprite.uvOffset, sprite.uvScale, TintToDebugMode(entity, sprite));
	}
    
	render.Draw(camera, drawEntityIdMode == ONLY);

	// draw particles

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	for (auto [t, p] : world.Query<Transform2D, Particle>())
	{
		if (p.HasAtlas())
		{
			const TextureAtlas::Bounds& uv = p.GetCurrentFrameUV();

			a<TextureAtlas>& atlas = p.atlas;
			a<Texture>& source = atlas->source;
			r<Texture> s = source;

			render.SubmitSprite(t, s, uv.uvOffset, uv.uvScale, p.GetTint());
		}

		else
		{
			render.SubmitSprite(t, p.GetTint());
		}
	}

	render.Draw(camera, drawEntityIdMode == ONLY);
}

void RenderLines(BatchLineRenderer& render, const Camera& camera, EntityWorld& world)
{
	render.Begin();

	for (auto [transform, line] : world.Query<Transform2D, LineParticle>())
	{
		render.SubmitLine(line.back, line.front, line.colorBack, line.colorFront, transform);
	}

	render.Draw(camera);
}

void RenderSprites(const Camera& camera, EntityWorld& world)
{
	RenderSprites(*spriteRender, camera, world);
}

void RenderLines(const Camera& camera, EntityWorld& world)
{
	RenderLines(*lineRender, camera, world);
}

r<Mesh>          GetQuadMesh2D()             { return g_quad; }
r<ShaderProgram> GetProgram_Sprite()         { return g_sprite; }

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

void InitSpriteProgram()
{
	const char* source_vert = 
		"#version 330 core\n"
		"layout (location = 0) in vec2 pos;"
		"layout (location = 1) in vec2 uv;"

		"out vec2 TexCoords;"

		"uniform mat4 model;"
		"uniform mat4 projection;"
		"uniform mat4 view;"
		"uniform vec2 uvOffset;"
		"uniform vec2 uvScale;"

		"void main()"
		"{"
			"TexCoords = uv * uvScale + uvOffset;"
			"gl_Position = projection * view * model * vec4(pos, 0.0, 1.0);"
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
    
			"if (spriteColor.a < .01) discard;"
			"color = spriteColor;"
		"}";

	g_sprite = MakeVertexAndFragmentShader(source_vert, source_frag);
}

void InitSimpleRendering()
{
	InitSpriteProgram();

	meshProgram = GetProgram_Sprite();

	g_quad = mkr<Mesh>();
	InitQuadMesh2D(*g_quad);

	spriteRender = mkr<BatchSpriteRenderer>();
	lineRender = mkr<BatchLineRenderer>();
}

void DnitSimpleRendering()
{
	meshProgram = nullptr;
	g_quad      = nullptr;
	g_sprite    = nullptr;

	spriteRender = nullptr;
	lineRender = nullptr;
}
