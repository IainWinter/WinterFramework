#include "render.h"

#include "Physics.h"
#include "ext/rendering/Particle.h"
#include "ext/rendering/BatchSpriteRenderer.h"
#include "ext/rendering/BatchLineRenderer.h"
#include "ext/Transform.h"

r<BatchSpriteRenderer> spriteDefault;
r<BatchLineRenderer> lineDefault;
r<Target> cellRender;

void render_init()
{
	spriteDefault = mkr<BatchSpriteRenderer>();
	lineDefault = mkr<BatchLineRenderer>();
}

void render_dnit()
{
	spriteDefault.reset();
	lineDefault.reset();
}

void render(const Camera& camera, EntityWorld& world)
{
	Render::SetRenderTarget(nullptr);
    
	spriteDefault->Begin();
	lineDefault->Begin();

	for (auto [e, transform, sprite] : world.QueryWithEntity<Transform2D, Sprite>())
		spriteDefault->SubmitSprite(WorldTransform(e, transform), sprite);

	for (auto [e, transform, particle] : world.QueryWithEntity<Transform2D, Particle>())
		spriteDefault->SubmitSprite(WorldTransform(e, transform), particle.GetTint());

	// debug for colliders

	for (auto [e, transform, body] : world.QueryWithEntity<Transform2D, Rigidbody2D>())
	{
		Transform2D noScale = WorldTransform(e, transform);
		noScale.scale = vec2(1.f);
		noScale.z = 3;

		for (const r<Collider>& collider : body.GetColliders())
		{
			switch (collider->GetType())
			{
				case Collider::tHull:
				{
					ArrayView<vec2> points = collider->As<HullCollider>().GetPoints();

					for (int i = 0; i < points.size(); i++)
					{
						int j = (i + 1) % points.size();

						vec2& a = points.at(i);
						vec2& b = points.at(j);

						lineDefault->SubmitLine(a, b, Color(100, 255, 100), noScale);
					}

					break;
				}
			}
		}
	}

	spriteDefault->Draw(camera);
	lineDefault->Draw(camera);
}
