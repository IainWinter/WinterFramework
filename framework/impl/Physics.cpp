#pragma once

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
	void BeginContact(b2Contact* contact) override 
	{
		Entity entityA = FixtureToEntity(contact->GetFixtureA());
		Entity entityB = FixtureToEntity(contact->GetFixtureB());

		if (!entityA.IsAlive() || !entityB.IsAlive()) return; // when an entity is deleted in collision / midframe

		Rigidbody2D& bodyA = entityA.Get<Rigidbody2D>();
		Rigidbody2D& bodyB = entityB.Get<Rigidbody2D>();

		if (bodyA.IsCollisionEnabled()) bodyA.OnCollision(CollisionInfo { entityA, entityB, contact, true });
		if (bodyB.IsCollisionEnabled()) bodyB.OnCollision(CollisionInfo { entityB, entityA, contact, false });
	}
};

Rigidbody2D::Rigidbody2D()
	: m_instance         (nullptr)
	, m_density          (1.f)
	, m_collisionEnabled (true)
{
	m_body.type = b2_dynamicBody;
}
	
void Rigidbody2D::SetTransform(Transform2D& transform)
{
	SetPosition(transform.position);
	SetAngle   (transform.rotation);

	LastTransform = transform;
}

bool Rigidbody2D::InWorld()            const { return !!m_instance; }
bool Rigidbody2D::IsFixedRotation()    const { return m_instance ? m_instance->IsFixedRotation() : m_body.fixedRotation; }
bool Rigidbody2D::IsCollisionEnabled() const { return m_collisionEnabled; }

void Rigidbody2D::ApplyForce(vec2 force)                        { assert_in_world(); m_instance->ApplyForceToCenter(_tb(force), true); }
void Rigidbody2D::ApplyForce(vec2 force, vec2 offsetFromCenter) { assert_in_world(); m_instance->ApplyForce(_tb(force), _tb(GetPosition() + offsetFromCenter), true); }
void Rigidbody2D::ApplyTorque(float force)                      { assert_in_world(); m_instance->ApplyTorque(force, true); }
	
vec2  Rigidbody2D::GetPosition()        const { return _fb(m_instance ? m_instance->GetPosition()        : m_body.position); }
vec2  Rigidbody2D::GetVelocity()        const { return _fb(m_instance ? m_instance->GetLinearVelocity()  : m_body.linearVelocity); }
float Rigidbody2D::GetAngle()           const { return     m_instance ? m_instance->GetAngle()           : m_body.angle; }
float Rigidbody2D::GetAngularVelocity() const { return     m_instance ? m_instance->GetAngularVelocity() : m_body.angularVelocity; }
float Rigidbody2D::GetDamping()         const { return     m_instance ? m_instance->GetLinearDamping()   : m_body.linearDamping; }
float Rigidbody2D::GetAngularDamping()  const { return     m_instance ? m_instance->GetAngularDamping()  : m_body.angularDamping; }
float Rigidbody2D::GetMass()            const { assert_in_world(); return m_instance->GetMass(); }
float Rigidbody2D::GetSpeed()           const { return length(GetVelocity()); }
float Rigidbody2D::GetAngularSpeed()    const { return abs(GetAngularVelocity()); }

Rigidbody2D& Rigidbody2D::SetPosition       (vec2  pos)      { if (m_instance) m_instance->SetTransform      (_tb(pos), m_instance->GetAngle()); else m_body.position        = _tb(pos); return *this; }
Rigidbody2D& Rigidbody2D::SetVelocity       (vec2  vel)      { if (m_instance) m_instance->SetLinearVelocity (_tb(vel));                         else m_body.linearVelocity  = _tb(vel); return *this; }
Rigidbody2D& Rigidbody2D::SetAngle          (float angle)    { if (m_instance) m_instance->SetTransform      (m_instance->GetPosition(), angle); else m_body.angle           = angle;    return *this; }
Rigidbody2D& Rigidbody2D::SetAngularVelocity(float avel)     { if (m_instance) m_instance->SetAngularVelocity(avel);                             else m_body.angularVelocity = avel;     return *this; }
Rigidbody2D& Rigidbody2D::SetFixedRotation  (bool  isFixed)  { if (m_instance) m_instance->SetFixedRotation  (isFixed);                          else m_body.fixedRotation   = isFixed;  return *this; }
Rigidbody2D& Rigidbody2D::SetDamping        (float damping)  { if (m_instance) m_instance->SetLinearDamping  (damping);                          else m_body.linearDamping   = damping;  return *this; }
Rigidbody2D& Rigidbody2D::SetAngularDamping (float adamping) { if (m_instance) m_instance->SetAngularDamping (adamping);                         else m_body.angularDamping  = adamping; return *this; }

Rigidbody2D& Rigidbody2D::SetDensity(float density)
{ 
	m_density = density;

	if (m_instance)
	for (b2Fixture* fix = GetColList(); fix; fix = fix->GetNext())
	{
		fix->SetDensity(density);
	}

	return *this; 
}

Rigidbody2D& Rigidbody2D::SetEnableCollision(bool respond)
{
	m_collisionEnabled = respond;

	if (m_instance)
	for (b2Fixture* fix = GetColList(); fix; fix = fix->GetNext())
	{
		fix->SetSensor(!respond);
	}

	return *this;
}

void Rigidbody2D::RemoveColliders()
{
	if (!m_instance) return;

	b2Fixture* fix = GetColList();
	while (fix)
	{
		b2Fixture* next = fix->GetNext();
		m_instance->DestroyFixture(fix);
		fix = next;
	}
}

int Rigidbody2D::GetColliderCount() const
{
	int count = 0;
	for (b2Fixture* fix = GetColList(); fix; fix = fix->GetNext())
	{
		count += 1;
	}

	return count;
}

b2Fixture* Rigidbody2D::AddCollider(const b2Shape& shape) 
{ 
	return AddCollider(shape, m_density); 
}

b2Fixture* Rigidbody2D::AddCollider(const b2Shape& shape, float density)
{
	assert_in_world();
	b2Fixture* fixture = m_instance->CreateFixture(&shape, density);
	fixture->SetSensor(!m_collisionEnabled);
	return fixture;
}
	
	  b2Fixture* Rigidbody2D::GetCollider(int i)       { return GetCol(i); }
const b2Fixture* Rigidbody2D::GetCollider(int i) const { return GetCol(i); }

void Rigidbody2D::assert_in_world() const
{
	assert(InWorld() && "Physics object has not been added to the world");
}

b2Fixture* Rigidbody2D::GetCol(int index) const
{
	assert_in_world();
	b2Fixture* fix = m_instance->GetFixtureList();
	for (int i = 0; i < index; i++) fix = fix->GetNext();
	return fix;
}

b2Fixture* Rigidbody2D::GetColList() const
{
	return m_instance ? m_instance->GetFixtureList() : nullptr;
}

bool   PointQueryResult::HasResult()     const { return results.size() > 0; }
Entity PointQueryResult::FirstEntiy()    const { return FirstResult().entity; }
float  PointQueryResult::FirstDistance() const { return FirstResult().distance; }

bool   RayQueryResult::HasResult()     const { return results.size() > 0; }
Entity RayQueryResult::FirstEntiy()    const { return FirstResult().entity; }
float  RayQueryResult::FirstDistance() const { return FirstResult().distance; }
vec2   RayQueryResult::FirstPoint()    const { return FirstResult().point; }
vec2   RayQueryResult::FirstNormal()   const { return FirstResult().normal; }

const PointQueryResult::Result& PointQueryResult::FirstResult() const { return results.at(0); }
const   RayQueryResult::Result&   RayQueryResult::FirstResult() const { return results.at(0); }

PhysicsWorld::PhysicsWorld()
{
	m_world = new b2World();
	m_world->SetContactListener(new ContactCallback());
}

PhysicsWorld::~PhysicsWorld()
{
	delete m_world;
}

void PhysicsWorld::Add(EntityWith<Transform2D, Rigidbody2D> e)
{
	e.Get<Rigidbody2D>()
		.SetTransform(e.Get<Transform2D>());

	Rigidbody2D& body = e.Get<Rigidbody2D>();
	body.m_instance = m_world->CreateBody(&body.m_body);

	b2BodyUserData& data = body.m_instance->GetUserData();
	data.entityId     = e.Id();
	data.entityOwning = e.Owning();
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
	return QueryRay(point, point + direction * distance);
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
