#pragma once

#include "app/System.h"
#include "ext/rendering/DebugRender.h"

struct System_PhysicsColliderRender : SystemBase
{
	void Update()
	{
        Debug::Begin();
        
		for (auto [transform, body] : Query<Transform2D, Rigidbody2D>())
        {
            for (const r<Collider>& collider : body.GetColliders())
            {
                switch (collider->GetType())
                {
                    case Collider::tCircle:
                        DrawCircleCollider(collider->As<CircleCollider>(), transform.z);
                        break;
                    default:
                        break;
                }
            }
        }
        
        Debug::End(First<Camera>());
	}
    
private:
    
    void DrawCircleCollider(const CircleCollider* circle, float z)
    {
        const int numbCircleSegments = 32; // this should be a function of the size of the circles
        const float step = w2PI / numbCircleSegments;
        
        for (int i = 1; i <= numbCircleSegments; i++)
        {
            float a1 = step * (i - 1);
            float a2 = step * i;
            
            vec2 v1 = circle->GetWorldCenter() + on_unit(a1) * circle->GetRadius();
            vec2 v2 = circle->GetWorldCenter() + on_unit(a2) * circle->GetRadius();
            
            Debug::Line(v1, v2, Color(40, 255, 40), z + 0.5f); // draw lines on top
        }
    }
};
