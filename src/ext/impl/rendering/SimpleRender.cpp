#include "ext/rendering/SimpleRender.h"

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
    meshProgram->Set("model", transform.World());
    meshProgram->Set("uvOffset", sprite.uvOffset);
    meshProgram->Set("uvScale", sprite.uvScale);
    meshProgram->Set("sprite", sprite.source ? sprite.source : spriteRender->GetDefaultTexture());
    meshProgram->Set("tint", sprite.tint);
}

void RenderMesh(const Camera& camera, const Transform2D& transform, Mesh& mesh)
{
	meshProgram->Use();
	meshProgram->Set("projection", camera.Projection());

	SetMeshProgramDefaults(transform);
	mesh.Draw();
}

void RenderMesh(const Camera& camera, const Transform2D& transform, Mesh& mesh, Sprite& sprite)
{
	meshProgram->Use();
	meshProgram->Set("projection", camera.Projection());

	SetMeshProgramValues(transform, sprite);
	mesh.Draw();
}

void RenderMeshes(const Camera& camera, EntityWorld& world)
{
	meshProgram->Use();
	meshProgram->Set("projection", camera.Projection());

	for (auto [transform, mesh] : world.Query<Transform2D, Mesh>())
	{
        SetMeshProgramDefaults(transform);
        mesh.Draw();
	}

	for (auto [transform, model] : world.Query<Transform2D, Model>())
	{
        SetMeshProgramDefaults(transform);
        
        for (r<Mesh>& mesh : model.meshes)
        {
            mesh->Draw();
        }
    }
    
    for (auto [transform, mesh] : world.Query<Transform2D, SpriteMesh>())
    {
        SetMeshProgramValues(transform, mesh.sprite);
        mesh.geometry.Draw();
    }
}

void RenderSprites(const Camera& camera, EntityWorld& world)
{
	spriteRender->Begin(camera, drawEntityIdMode == ONLY);

	// draw sprites

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
	for (auto [entity, transform, sprite] : world.QueryWithEntity<Transform2D, Sprite>())
	{
		spriteRender->SubmitSprite(transform, sprite.source, vec2(0.f, 0.f), vec2(1.f, 1.f), TintToDebugMode(entity, sprite));
	}
	spriteRender->Draw();

	// draw particles

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	for (auto [t, p] : world.Query<Transform2D, Particle>())
	{
		if (p.HasAtlas())
		{
			const TextureAtlas::Bounds& uv = p.GetCurrentFrameUV();
			spriteRender->SubmitSprite(t, p.atlas->source, uv.uvOffset, uv.uvScale, p.GetTint());
		}

		else
		{
			spriteRender->SubmitSprite(t, p.GetTint());
		}
	}

	spriteRender->Draw();
}

void RenderLines(const Camera& camera, EntityWorld& world)
{
	lineRender->Begin();

	for (auto [transform, line] : world.Query<Transform2D, LineParticle>())
	{
		lineRender->SubmitLine(transform, line.back, line.front, line.colorBack, line.colorFront);
	}

	lineRender->Draw(camera);
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