#include "ext/physics/ClosestPointOnShape.h"

std::vector<vec2> GetBodyVertices(const Rigidbody2D& body)
{
    std::vector<vec2> vertices;

    for (int i = 0; i < body.GetColliderCount(); i++)
    {
        const b2Fixture* f = body.GetCollider(i);
        if (f->GetType() == b2Shape::e_polygon)
        {
            b2PolygonShape* ps = (b2PolygonShape*)f->GetShape();
            for (int j = 0; j < ps->m_count; j++)
            {
                b2Rot rot; rot.Set(body.GetAngle());
                vec2 v = _fb(b2Mul(rot, ps->m_vertices[j])) + body.GetPosition();
                vertices.push_back(v);
            }
        }
    }

    return vertices;
}

vec2 GetClosestPointInLine(vec2 point, vec2 linePoint1, vec2 linePoint2)
{
    vec2 bVec1 = linePoint2 - linePoint1; // LINE FROM POINT 1 to 2
    vec2 bVec2 = point - linePoint1;      // LINE FROM VERTEX TO POINT 1
    
    float av = bVec1.x * bVec1.x + bVec1.y * bVec1.y;
    float bv = bVec2.x * bVec1.x + bVec2.y * bVec1.y;
    float t = bv / av;
    
    //IF POINT LIES OUTSIDE THE LINE SEGMENT, THEN WE FIX IT TO ONE OF THE END POINTS
    if(t < 0) t = 0;
    if(t > 1) t = 1;
    
    // Closest point = point 1 + line from point 1 to 2 * t
    
    return linePoint1 + vec2(bVec1.x * t, bVec1.y * t);
}

vec2 GetClosestPoint(const Rigidbody2D& body, vec2 boxPoint)
{
    vec2 closestPoint = vec2(0.f);
    
    // exit on no collider
    if (body.GetColliderCount() == 0) return closestPoint;

    switch (body.GetCollider()->GetType())
    {
        case b2Shape::e_polygon:
        {
            std::vector<vec2> vs = GetBodyVertices(body);
            float minDis = FLT_MAX;

            for (int i = 0; i < vs.size(); i++)
            {
                vec2 closest = GetClosestPointInLine(boxPoint, vs.at(i), vs.at((i + 1) % vs.size()));
                float d = distance(closest, boxPoint);

                if (d < minDis)
                {
                    minDis = d;
                    closestPoint = closest;
                }
            }

            break;
        }

        case b2Shape::e_circle:
        {
            b2CircleShape* cs = (b2CircleShape*)body.GetCollider()->GetShape();
            float dist = distance(boxPoint, body.GetPosition()); 
            float circumferenceDistance = dist - cs->m_radius;
            closestPoint = (body.GetPosition() - boxPoint) * circumferenceDistance / dist + boxPoint;

            break;
        }
    }

    return closestPoint;
}