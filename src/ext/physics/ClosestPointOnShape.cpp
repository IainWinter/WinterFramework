#include "ext/physics/ClosestPointOnShape.h"

vec2 _GetClosestPointInLine(vec2 point, vec2 linePoint1, vec2 linePoint2)
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

vec2 physics_GetClosestPointOnBody(const Rigidbody2D& body, vec2 pointInWorldSpace)
{
    vec2 pointLocal = pointInWorldSpace;

    vec2 closestPoint = vec2(0.f);
    float minDis = FLT_MAX;

    // exit on no collider
    if (body.GetColliderCount() == 0) return closestPoint;

    for (r<Collider> collider : body.GetColliders())
    {
        switch (collider->GetType())
        {
            case Collider::tHull:
            {
                HullCollider& hull = collider->As<HullCollider>();
                ArrayView<vec2> points = hull.GetPoints();

                for (int i = 0; i < points.size(); i++)
                {
                    vec2 a = points.at(i);
                    vec2 b = points.at((i + 1) % points.size());

                    a = rotate(a, body.GetAngle()) + body.GetPosition();
                    b = rotate(b, body.GetAngle()) + body.GetPosition();

                    vec2 closest = _GetClosestPointInLine(pointLocal, a, b);
                    float d = distance(closest, pointLocal);

                    if (d < minDis)
                    {
                        minDis = d;
                        closestPoint = closest;
                    }
                }

                break;
            }

            case Collider::tCircle:
            {
                float radius = collider->As<CircleCollider>().GetRadius();
                float dist = distance(pointLocal, body.GetPosition());
                float circumferenceDistance = dist - radius;
                vec2 closest = (body.GetPosition() - pointLocal) * circumferenceDistance / dist + pointLocal;

                if (dist < minDis)
                {
                    minDis = dist;
                    closestPoint = closest;
                }

                break;
            }
                
            default:
                break;
        }
    }

    return closestPoint;
}
