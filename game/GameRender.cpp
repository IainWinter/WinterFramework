#include "GameRender.h"

r<Mesh> g_quad;
r<ShaderProgram> g_wireframe;
r<ShaderProgram> g_sprite;
r<ShaderProgram> g_collisionInfo;

r<Mesh>          GetQuadMesh2D()             { return g_quad; }
r<ShaderProgram> GetProgram_Sprite()         { return g_sprite; }
r<ShaderProgram> GetProgram_Wireframe()      { return g_wireframe; }
r<ShaderProgram> GetProgram_SandSpriteInfo() { return g_collisionInfo; }

void InitQuadMesh2D()
{
	g_quad = mkr<Mesh>();
	g_quad->Add<vec2>(Mesh::aPosition,     { vec2(-1, -1), vec2(1, -1), vec2(1, 1), vec2(-1, 1) });
	g_quad->Add<vec2>(Mesh::aTextureCoord, { vec2( 0,  0), vec2(1,  0), vec2(1, 1), vec2( 0, 1) });
	g_quad->Add<int> (Mesh::aIndexBuffer,  { 0, 1, 2, 0, 2, 3 });
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
		"gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);"
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

		"void main()"
		"{"
			"vec4 spriteColor = texture(sprite, TexCoords);"

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
			"vec4 mask = texture(colliderMask, TexCoords);"
			"if (mask.a > .7) mask.a = 1.f;" // round up for health thing
			"if (mask.a == 0) discard;"
			"spriteId = ivec4(TexCoords * spriteSize, spriteIndex, 1);"
		"}";

	g_collisionInfo = MakeVertexAndFragmentShader(source_vert, source_frag);

}

void InitGameRenderVars()
{
	InitQuadMesh2D();
	InitWireframeProgram();
	InitSpriteProgram();
	InitCollisionInfoProgram();
}