#pragma one

#include "Rendering.h"

void InitGameRenderVars();

r<Mesh> GetQuadMesh2D();
Mesh& InitQuadMesh2D(Mesh& mesh);

r<ShaderProgram> GetProgram_Sprite();
r<ShaderProgram> GetProgram_Wireframe();
r<ShaderProgram> GetProgram_SandSpriteInfo();

r<ShaderProgram> GetProgram_Debug_DisplayCollisionInfo();