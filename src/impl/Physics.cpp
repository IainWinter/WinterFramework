#include "Physics.h"

b2Vec2 _tb(const vec2& v)   { return b2Vec2(v.x, v.y); }
vec2   _fb(const b2Vec2& v) { return vec2(v.x, v.y); }

Entity FixtureToEntity(b2Fixture* fixture)
{
	b2BodyUserData& data = fixture->GetBody()->GetUserData();
	return ((EntityWorld*)data.entityOwning)->Wrap(data.entityId);
}

struct PointQueryCallback : b2QueryCallback
{
	PointQueryResult result;
	vec2 point;

	PointQueryCallback(vec2 point)
		: point(point) {}

	bool ReportFixture(b2Fixture* fixture)
	{
		result.results.push_back({ FixtureToEntity(fixture), distance(_fb(fixture->GetBody()->GetPosition()), point) });
		return false;
	}
};

struct RayQueryCallback : b2RayCastCallback
{
	RayQueryResult result;
	float length;

	RayQueryCallback(float length)
		: length(length) {}

	float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
	{
		result.results.push_back({ FixtureToEntity(fixture), fraction * length, _fb(point), _fb(normal) });
		return -1;
	}
};

struct ContactCallback : b2ContactListener
{
	PhysicsWorld* m_world;

	ContactCallback(PhysicsWorld* world) 
		: m_world (world)
	{}

	void BeginContact(b2Contact* contact) override 
	{
		Entity entityA = FixtureToEntity(contact->GetFixtureA());
		Entity entityB = FixtureToEntity(contact->GetFixtureB());

		if (!entityA.IsAlive() || !entityB.IsAlive()) return; // when an entity is deleted in collision / midframe

		Rigidbody2D& bodyA = entityA.Get<Rigidbody2D>();
		Rigidbody2D& bodyB = entityB.Get<Rigidbody2D>();

		if (bodyA.IsCollisionEnabled()) bodyA.OnCollision(CollisionInfo { entityA, entityB, contact, true });
		if (bodyB.IsCollisionEnabled()) bodyB.OnCollision(CollisionInfo { entityB, entityA, contact, false });

		m_world->FireOnCollision(WorldCollsionInfo
		{
			entityA, entityB, contact
		});
	}
};

//
//	Collider attachment
//

ColliderAttachment::ColliderAttachment()
	: m_fixture   (nullptr)
	, m_tempDef   (nullptr)
	, m_tempShape (nullptr)
{
	m_tempDef = mkr<b2FixtureDef>();
	m_tempDef->density = 1.0f;
}

void ColliderAttachment::InitTemp(b2CircleShape& shape)
{
	m_tempShape = mkr<b2CircleShape>(shape);
}

void ColliderAttachment::InitTemp(b2PolygonShape& shape)
{
	m_tempShape = mkr<b2PolygonShape>(shape);
}

void ColliderAttachment::InitFixture(Rigidbody2D& body)
{
	if (m_fixture)
	{
		log_physics("w~Failed to add collider to body: It is already attached");
		return;
	}

	// assign the shape, this will get copied into box2d
	m_tempDef->shape = m_tempShape.get();

	// create the fixture from the temp def
    m_fixture = body.m_instance->CreateFixture(m_tempDef.get());
    
    // reset temp, this indicates that the fixture has been created
	m_tempDef = nullptr;
	m_tempShape = nullptr;

	// fixture settings

	// this is wrong, should use filters
	m_fixture->SetSensor(!body.IsCollisionEnabled());
}

void ColliderAttachment::DnitFixture(Rigidbody2D& body)
{
	if (!m_fixture)
	{
		log_physics("w~Failed to remove collider from body: It is not attached");
		return;
	}

	body.m_instance->DestroyFixture(m_fixture);
	m_fixture = nullptr;
}

b2Shape& ColliderAttachment::GetShape() const
{
	return *(m_fixture ? m_fixture->GetShape() : m_tempShape.get());
}

vec2 ColliderAttachment::GetBodyPosition() const
{
	return (m_fixture ? _fb(m_fixture->GetBody()->GetPosition()) : vec2(0.f, 0.f));
}

bool ColliderAttachment::OnBody() const
{
	return !!m_fixture;
}

// settings

void ColliderAttachment::SetDensity(float density)
{
    if (m_fixture) m_fixture->SetDensity(density);
    else           m_tempDef->density = density;
}

float ColliderAttachment::GetDensity() const
{
	return m_fixture ? m_fixture->GetDensity()
		             : m_tempDef->density;
}

//
//	Collider
//

Collider::Collider(ColliderType type)
	: m_type(type)
{
	// all colliders have at least one attachment
	// allow them to store a refernece to this

	m_attachments.push_back(ColliderAttachment());
}

Collider::ColliderType Collider::GetType() const
{
	return m_type;
}

Collider& Collider::SetDensity(float density, int attachment)
{
	if (attachment == -1)
	{
		for (ColliderAttachment& attachment : m_attachments)
		{
			attachment.SetDensity(density);
		}
	}

	else if (m_attachments.size() < attachment)
	{
		m_attachments.at(attachment).SetDensity(density);
	}

	else
	{
		log_physics("w~Tried to set the density of an attachment that doesnt exist: "
			"%d. There are only %d on this body", attachment, m_attachments.size());
	}

	return *this;
}

float Collider::GetDensity(int attachment) const
{
	float density = 0.f;

	if (attachment == -1)
	{
		for (const ColliderAttachment& attachment : m_attachments)
		{
			density += attachment.GetDensity();
		}

		density /= m_attachments.size();
	}

	else if (m_attachments.size() < attachment)
	{
		density = m_attachments.at(attachment).GetDensity();
	}

	else
	{
		log_physics("w~Tried to get the density of an attachment that doesnt exist: "
			"%d. There are only %d on this body", attachment, m_attachments.size());
	}

	return density;
}

//
//  Circle Collider
//

CircleCollider::CircleCollider(float radius, vec2 center)
	: Collider     (tCircle)
{
	b2CircleShape circle;
	circle.m_radius = radius;
	circle.m_p = _tb(center);

	m_attachments.at(0).InitTemp(circle);
}

r<Collider> CircleCollider::MakeCopy() const
{
	return mkr<CircleCollider>(GetRadius(), GetCenter());
}

void CircleCollider::AddToBody(Rigidbody2D& body)
{
	m_attachments.at(0).InitFixture(body);
}

void CircleCollider::RemoveFromBody(Rigidbody2D& body)
{
	m_attachments.at(0).DnitFixture(body);
}

vec2 CircleCollider::GetWorldCenter() const
{
	return GetCenter() + m_attachments.at(0).GetBodyPosition();
}

float CircleCollider::GetRadius() const { return GetShape().m_radius; }
vec2  CircleCollider::GetCenter() const { return _fb(GetShape().m_p); }

CircleCollider& CircleCollider::SetRadius(float radius) { GetShape().m_radius = radius; return *this; }
CircleCollider& CircleCollider::SetCenter(vec2 center)  { GetShape().m_p = _tb(center); return *this; }

b2CircleShape& CircleCollider::GetShape() const
{
	return (b2CircleShape&)m_attachments.at(0).GetShape();
}

//
//	Hull Collider
//

HullCollider::HullCollider()
	: Collider     (tHull)
{
	b2PolygonShape shape;
	m_attachments.at(0).InitTemp(shape);
}

r<Collider> HullCollider::MakeCopy() const
{
	ArrayView<vec2> view = GetPoints();

	r<HullCollider> hull = mkr<HullCollider>();
	hull->SetPoints(view.begin(), view.end());

	return hull;
}

void HullCollider::AddToBody(Rigidbody2D& body)
{
	m_attachments.at(0).InitFixture(body);
}

void HullCollider::RemoveFromBody(Rigidbody2D& body)
{
	m_attachments.at(0).DnitFixture(body);
}

vec2 HullCollider::GetWorldCenter() const
{
	return /*GetCenter() +*/ m_attachments.at(0).GetBodyPosition();
}

vec2 HullCollider::GetCenter() const
{
	return _fb(GetShape().m_centroid);
}

ArrayView<vec2> HullCollider::GetPoints() const
{
	b2Vec2* b = GetShape().m_vertices;
	b2Vec2* e = b + GetShape().m_count;

	return ArrayView<vec2>((vec2*)b, (vec2*)e); // casts to vec2
}

HullCollider& HullCollider::SetPointsBox(float w, float h, float a, vec2 center)
{
	GetShape().SetAsBox(w, h, _tb(center), a);
	return *this;
}

HullCollider& HullCollider::SetPoints(const ArrayView<vec2>& list)
{
	return SetPoints(list.begin(), list.end());
}

b2PolygonShape& HullCollider::GetShape() const
{
	return (b2PolygonShape&)m_attachments.at(0).GetShape();
}

//b2PolygonShape& PolygonCollider::GetShape() const
//{
//	return *(b2PolygonShape*)(OnBody() ? m_fixture->GetShape() : m_pass.get());
//}
//
////
////  Polygon Collider
////
//    
//PolygonCollider::PolygonCollider(const std::vector<vec2>& convexHull)
//	: Collider(tPolygon)
//{
//    m_pass = mkr<b2PolygonShape>();
//    SetPoints(convexHull);
//}
//    
//r<Collider> PolygonCollider::MakeCopy() const
//{
//    return mkr<PolygonCollider>(m_points);
//}
//
//vec2 PolygonCollider::GetCenter() const
//{
//    return _fb(GetShape().m_centroid);
//}
//
//const std::vector<vec2>& PolygonCollider::GetPoints() const
//{
//    return m_points;
//}
//
//PolygonCollider& PolygonCollider::SetCenter(vec2 center)
//{
//    GetShape().m_centroid = _tb(center); // is this valid or do we need to add center to each vert
//    return *this;
//}
//
//PolygonCollider& PolygonCollider::SetPoints(const std::vector<vec2>& convexHull)
//{
//	// this is not 100% safe, but glm and box2d have same memory layout by default
//
//	if (convexHull.size() >= VertLimit::value) // exit on too many points
//	{
//		log_physics("e~Failed to set points on polygon collider, too many points");
//		return *this;
//	}
//
//	if (convexHull.size() > 0) // only set points if there are more than 0 verts
//	{
//		GetShape().Set((b2Vec2*)convexHull.data(), (int32)convexHull.size());
//	}
//
//	UpdatePoints();
//    
//    return *this;
//}
//
//PolygonCollider& PolygonCollider::SetPointsBox(float w, float h, float a, vec2 center)
//{
//	GetShape().SetAsBox(w, h, _tb(center), a);
//	UpdatePoints();
//
//	return *this;
//}
//
//vec2 PolygonCollider::GetWorldCenter() const
//{
//    return GetCenter() + GetBodyPosition();
//}
//
//b2PolygonShape& PolygonCollider::GetShape() const
//{
//    return *(b2PolygonShape*)(OnBody() ? m_fixture->GetShape() : m_pass.get());
//}
//
//void PolygonCollider::UpdatePoints()
//{
//	b2PolygonShape& p = GetShape();
//
//	// box2d will edit the point, so lets copy them back into our array to sync
//
//	m_points.clear();
//	m_points.reserve(p.m_count);
//	for (int i = 0; i < p.m_count; i++)
//	{
//		m_points.push_back(_fb(p.m_vertices[i]));
//	}
//}

//
//  Rigidbody2D
//

Rigidbody2D::Rigidbody2D()
	: m_instance         (nullptr)
	, m_density          (1.f)
	, m_collisionEnabled (true)
{}
	
Rigidbody2D& Rigidbody2D::SetTransform(Transform2D& transform)
{
	SetPosition(transform.position);
	SetAngle   (transform.rotation);

	m_lastTransform = transform;

	return *this;
}

const Transform2D& Rigidbody2D::GetLastTransform() const
{
	return m_lastTransform;
}

void Rigidbody2D::UpdateLastTransform()
{
	m_lastTransform.position = GetPosition();
	m_lastTransform.rotation = GetAngle();
}

void Rigidbody2D::ApplyForce(vec2 force)                        { m_instance->ApplyForceToCenter(_tb(force), true); }
void Rigidbody2D::ApplyForce(vec2 force, vec2 offsetFromCenter) { m_instance->ApplyForce(_tb(force), _tb(GetPosition() + offsetFromCenter), true); }
void Rigidbody2D::ApplyTorque(float force)                      { m_instance->ApplyTorque(force, true); }
	
vec2  Rigidbody2D::GetPosition()        const { return _fb(m_instance->GetPosition()); }
vec2  Rigidbody2D::GetVelocity()        const { return _fb(m_instance->GetLinearVelocity()); }
float Rigidbody2D::GetAngle()           const { return m_instance->GetAngle(); }
float Rigidbody2D::GetAngularVelocity() const { return m_instance->GetAngularVelocity(); }
float Rigidbody2D::GetDamping()         const { return m_instance->GetLinearDamping(); }
float Rigidbody2D::GetAngularDamping()  const { return m_instance->GetAngularDamping(); }
bool  Rigidbody2D::IsRotationFixed()    const { return m_instance->IsFixedRotation(); }
bool  Rigidbody2D::IsCollisionEnabled() const { return m_collisionEnabled; }
float Rigidbody2D::GetDensity()         const { return m_density; }
float Rigidbody2D::GetMass()            const { return m_instance->GetMass(); }
float Rigidbody2D::GetSpeed()           const { return length(GetVelocity()); }
float Rigidbody2D::GetAngularSpeed()    const { return abs(GetAngularVelocity()); }

Rigidbody2D& Rigidbody2D::SetPosition       (vec2  pos)      { m_instance->SetTransform      (_tb(pos), m_instance->GetAngle()); return *this; }
Rigidbody2D& Rigidbody2D::SetVelocity       (vec2  vel)      { m_instance->SetLinearVelocity (_tb(vel));                         return *this; }
Rigidbody2D& Rigidbody2D::SetAngle          (float angle)    { m_instance->SetTransform      (m_instance->GetPosition(), angle); return *this; }
Rigidbody2D& Rigidbody2D::SetAngularVelocity(float avel)     { m_instance->SetAngularVelocity(avel);                             return *this; }
Rigidbody2D& Rigidbody2D::SetDamping        (float damping)  { m_instance->SetLinearDamping  (damping);                          return *this; }
Rigidbody2D& Rigidbody2D::SetAngularDamping (float adamping) { m_instance->SetAngularDamping (adamping);                         return *this; }
Rigidbody2D& Rigidbody2D::SetRotationFixed  (bool  isFixed)  { m_instance->SetFixedRotation  (isFixed);                          return *this; }

Rigidbody2D& Rigidbody2D::SetEnableCollision(bool respond)
{
	m_collisionEnabled = respond;
	for (b2Fixture* fix = GetColList(); fix; fix = fix->GetNext()) fix->SetSensor(!respond);
	return *this;
}

Rigidbody2D& Rigidbody2D::SetDensity(float density)
{ 
	m_density = density;
	for (b2Fixture* fix = GetColList(); fix; fix = fix->GetNext()) fix->SetDensity(density);
	return *this; 
}

Rigidbody2D& Rigidbody2D::AddCollider(const Collider& collider)
{
	r<Collider> c = collider.MakeCopy();
	
	if (m_instance)
	{
		c->AddToBody(*this);
	}

	m_colliders.push_back(c);

	return *this;
}

void Rigidbody2D::ClearColliders()
{
	if (m_instance)
	{
		for (r<Collider>& c : m_colliders)
		{
			c->RemoveFromBody(*this);
		}
	}

	m_colliders.clear();
}

int Rigidbody2D::GetColliderCount() const
{
	return (int)m_colliders.size();
}

r<Collider>& Rigidbody2D::GetCollider(int colliderIndex)
{
	return m_colliders.at(colliderIndex);
}

const r<Collider>& Rigidbody2D::GetCollider(int colliderIndex) const
{
	return m_colliders.at(colliderIndex);
}
	
const std::vector<r<Collider>>& Rigidbody2D::GetColliders() const
{
	return m_colliders;
}

void Rigidbody2D::SetPreInit(const b2BodyDef& def)
{
	m_preinit = def;
}

b2BodyDef Rigidbody2D::GetBodyDef() const
{
	b2BodyDef def;

	if (!m_instance)
	{
		return m_preinit;
	}

	def.type            = m_instance->GetType();
	def.position        = m_instance->GetPosition();
	def.angle           = m_instance->GetAngle();
	def.linearVelocity  = m_instance->GetLinearVelocity();
	def.angularVelocity = m_instance->GetAngularVelocity();
	def.linearDamping   = m_instance->GetLinearDamping();
	def.angularDamping  = m_instance->GetAngularDamping();
	def.allowSleep      = m_instance->IsSleepingAllowed();
	def.awake           = m_instance->IsAwake();
	def.fixedRotation   = m_instance->IsFixedRotation();
	def.bullet          = m_instance->IsBullet();
	def.enabled         = m_instance->IsEnabled();
	def.userData        = m_instance->GetUserData();
	def.gravityScale    = m_instance->GetGravityScale();

	return def;
}

b2Fixture* Rigidbody2D::GetCol(int index) const
{
	b2Fixture* fix = m_instance->GetFixtureList();
	for (int i = 0; i < index; i++) fix = fix->GetNext();
	return fix;
}

b2Fixture* Rigidbody2D::GetColList() const
{
	return m_instance ? m_instance->GetFixtureList() : nullptr;
}

bool   PointQueryResult::HasResult()     const { return results.size() > 0; }
Entity PointQueryResult::FirstEntity()   const { return FirstResult().entity; }
float  PointQueryResult::FirstDistance() const { return FirstResult().distance; }

bool   RayQueryResult::HasResult()     const { return results.size() > 0; }
Entity RayQueryResult::FirstEntity()   const { return FirstResult().entity; }
float  RayQueryResult::FirstDistance() const { return FirstResult().distance; }
vec2   RayQueryResult::FirstPoint()    const { return FirstResult().point; }
vec2   RayQueryResult::FirstNormal()   const { return FirstResult().normal; }

const PointQueryResult::Result& PointQueryResult::FirstResult() const { return results.at(0); }
const   RayQueryResult::Result&   RayQueryResult::FirstResult() const { return results.at(0); }

PhysicsWorld::PhysicsWorld()
	: m_world (nullptr)
{
	ReallocWorld();
}

PhysicsWorld::~PhysicsWorld()
{
	delete m_world;
}

void PhysicsWorld::ReallocWorld()
{
	delete m_world;
	m_world = new b2World();
	m_world->SetContactListener(new ContactCallback(this));
}

void PhysicsWorld::Add(EntityWith<Rigidbody2D> e)
{
	Rigidbody2D& body = e.Get<Rigidbody2D>();

	// add body to world

	b2BodyDef def = body.m_preinit;
	def.type = b2_dynamicBody;

	body.m_instance = m_world->CreateBody(&def);

	// add colliders

	for (r<Collider>& collider : body.m_colliders)
	{
		collider->AddToBody(body);
	}

	// set user data for the collision callbacks to be able to create entities

	b2BodyUserData& data = body.m_instance->GetUserData();
	data.entityId     = e.Id();
	data.entityOwning = e.Owning();

	// Set position to where transform is if entity has one

    if (e.Has<Transform2D>())
    {
        e.Get<Rigidbody2D>()
            .SetTransform(e.Get<Transform2D>());
    }
}
	
void PhysicsWorld::Remove(Entity& e)
{
	Rigidbody2D& body = e.Get<Rigidbody2D>();
	m_world->DestroyBody(body.m_instance);
	body.m_instance = nullptr;
}

void PhysicsWorld::Tick(float dt)
{
	m_world->Step(dt, 8, 3);
}

RayQueryResult PhysicsWorld::QueryRay(vec2 point, vec2 direction, float distance) const
{
	return QueryRay(point, point + safe_normalize(direction) * distance);
}

RayQueryResult PhysicsWorld::QueryRay(vec2 point, vec2 target) const
{
	b2Vec2 a = _tb(point);
	b2Vec2 b = _tb(target);

	RayQueryCallback query(distance(target, point));
	m_world->RayCast(&query, a, b);
	std::sort(
		query.result.results.begin(),
		query.result.results.end(),
		[](const auto& a, const auto& b) { return a.distance < b.distance; }
	);

	return query.result;
}

PointQueryResult PhysicsWorld::QueryPoint(vec2 point, float radius) const
{
	b2AABB aabb;
	aabb.lowerBound = b2Vec2(point.x - radius, point.y - radius);
	aabb.upperBound = b2Vec2(point.x + radius, point.y + radius);

	PointQueryCallback query(point);
	m_world->QueryAABB(&query, aabb);
	std::sort(
		query.result.results.begin(),
		query.result.results.end(),
		[](const auto& a, const auto& b) { return a.distance < b.distance; }
	);
		
	return query.result;
}

PhysicsWorld::PhysicsWorld(PhysicsWorld&& move) noexcept
	: m_world (move.m_world)
{
	move.m_world = nullptr;
}

PhysicsWorld& PhysicsWorld::operator=(PhysicsWorld&& move) noexcept
{
	m_world = move.m_world;
	move.m_world = nullptr;
	return *this;
}
