#pragma once

#include "ext/rendering/Particle.h"
#include "Physics.h"
#include "app/System.h"
#include "Rendering.h"

void ClearPrefabs();

ParticleEmitter GetPrefab_BulletEmitter();
ParticleEmitter GetPrefab_LaserEmitter();
ParticleEmitter GetPrefab_FuelShotEmitter();
ParticleEmitter GetPrefab_LightningEmitter();

r<Texture> GetPrefab_Texture(const std::string& str, bool loadAsStatic = true);