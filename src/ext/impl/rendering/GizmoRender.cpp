#include "ext/rendering/GizmoRender.h"
#include "ext/rendering/DebugRender.h"
#include "Common.h"
#include "Physics.h"

void DrawCircleCollider(const CircleCollider& circle, float a, float z);
void DrawHullCollider(const HullCollider& hull, float a, float z);

void Gizmo_RenderPhysicsColliders(const Camera& camera, EntityWorld& world)
{
    Debug::Begin();

	for (auto [transform, body] : world.Query<Transform2D, Rigidbody2D>())
    {
        for (const r<Collider>& collider : body.GetColliders())
        {
            switch (collider->GetType())
            {
                case Collider::tCircle:
                    DrawCircleCollider(collider->As<CircleCollider>(), transform.rotation, transform.z);
                    break;
                case Collider::tHull:
                    DrawHullCollider(collider->As<HullCollider>(), transform.rotation, transform.z);
                    break;
                default:
                    break;
            }
        }
    }

    Debug::End(camera);
}

void Gizmo_RenderCamearFrustum(const Camera& camera, EntityWorld& world)
{
}

//
//  internal, make a config struct
//

const Color color = Color(40, 255, 40);
const float zAbove = 0.01f;              // draw lines on top

void DrawCircleCollider(const CircleCollider& circle, float a, float z)
{
    const int numbCircleSegments = 32; // this should be a function of the size of the circles
    const float step = w2PI / numbCircleSegments;

    for (int i = 0; i < numbCircleSegments; i++)
    {
        float a1 = a + step * i;
        float a2 = a + step * (i + 1);

        vec2 v1 = circle.GetWorldCenter() + on_unit(a1) * circle.GetRadius();
        vec2 v2 = circle.GetWorldCenter() + on_unit(a2) * circle.GetRadius();

        Debug::Line(v1, v2, color, z + zAbove);
    }
}

void DrawHullCollider(const HullCollider& hull, float a, float z)
{
    ArrayView<vec2> view = hull.GetPoints();

    for (size_t i = 0; i < view.size(); i++)
    {
        size_t j = (i + 1) % view.size();

        vec2 v1 = hull.GetWorldCenter() + rotate(view.at(i), a);
        vec2 v2 = hull.GetWorldCenter() + rotate(view.at(j), a);

        Debug::Line(v1, v2, color, z + zAbove);
    }
}