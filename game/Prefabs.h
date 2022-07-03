#pragma once

#include "ext/rendering/Particle.h"
#include "Physics.h"
#include "Leveling.h"
#include "Rendering.h"

ParticleEmitter GetPrefab_BulletEmitter();
ParticleEmitter GetPrefab_LaserEmitter();
ParticleEmitter GetPrefab_FuelShotEmitter();

r<Texture> GetPrefab_Texture(const std::string& str, bool loadAsStatic = true);