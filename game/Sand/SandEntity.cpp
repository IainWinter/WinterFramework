#include "Sand/SandEntity.h"

Entity CreateSandSprite(r<World> world, const std::string& path, const std::string& collider_mask_path)
{
	r<Texture> sprite = mkr<Texture>(_a(path), false);
	r<Texture> mask   = mkr<Texture>(_a(collider_mask_path), false);

	assert(sprite->Length() == mask->Length());

	Entity entity = world->GetEntityWorld().Create();
	if (!entity.Has<Transform2D>()) entity.Add<Transform2D>();
	entity.Add<SandSprite>(mask);
	entity.Add<Sprite>(sprite);

	world->GetEventQueue().send(event_SandAddSprite { entity });

	return entity;
}

Entity CreateTexturedCircle(r<World> world, const std::string& path)
{
	r<Texture> sprite = mkr<Texture>(_a(path), false);

	Entity entity = world->GetEntityWorld().Create();
	if (!entity.Has<Transform2D>()) entity.Add<Transform2D>();
	entity.Add<SandSprite>(sprite).isCircle = true;
	entity.Add<Sprite>(sprite);

	world->GetEventQueue().send(event_SandAddSprite{ entity });

	return entity;
}