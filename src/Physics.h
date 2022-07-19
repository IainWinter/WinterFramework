#pragma once

#include "Common.h"
#include "Entity.h"
#include "box2d/box2d.h"

inline b2Vec2 _tb(const vec2& v) { return b2Vec2(v.x, v.y); }
inline vec2 _fb(const b2Vec2& v) { return vec2(v.x, v.y); }

//struct Collider2D
//{
//	enum ColliderType
//	{
//		CIRCLE,
//		TRIANGLE
//	};
//
//private:
//	ColliderType m_type;
//	b2Shape*     m_shape = nullptr;
//
//public:
//	Collider2D(
//		ColliderType type
//	)
//		: m_type (type)
//	{
//		switch ()
//	}
//
//
//
//
//};

struct Rigidbody2D
{
public:
	Transform2D LastTransform;
	
private:
	b2BodyDef m_body;   // Init values for Physics Add
	b2Body* m_instance; // If null, not in physics world
	float m_density;    // deafult value on each new collider, if the new collider had 0 density

	friend struct PhysicsWorld;

public:
	Rigidbody2D(
		Transform2D transform = {}
	)
		: m_instance    (nullptr)
		, m_density     (1.f)
		, LastTransform (transform) 
	{
		m_body.type = b2_dynamicBody;
		SetTransform(transform);
	}
	
	void SetTransform(Transform2D& transform)
	{
		SetPosition(transform.position);
		SetAngle   (transform.rotation);

		LastTransform = transform;
	}

	// functions for setting properties for box2d...

	bool InWorld() const { return !!m_instance; }
	
	vec2  GetPosition()        const { return _fb(m_instance ? m_instance->GetPosition()        : m_body.position); }
	vec2  GetVelocity()        const { return _fb(m_instance ? m_instance->GetLinearVelocity()  : m_body.linearVelocity); }
	float GetAngle()           const { return     m_instance ? m_instance->GetAngle()           : m_body.angle; }
	float GetAngularVelocity() const { return     m_instance ? m_instance->GetAngularVelocity() : m_body.angularVelocity; }
	bool  IsFixedRotation()    const { return     m_instance ? m_instance->IsFixedRotation()    : m_body.fixedRotation; }
	float GetDamping()         const { return     m_instance ? m_instance->GetLinearDamping()   : m_body.linearDamping; }
	float GetAngularDamping()  const { return     m_instance ? m_instance->GetAngularDamping()  : m_body.angularDamping; }

	Rigidbody2D& SetPosition       (vec2  pos)      { if (m_instance) m_instance->SetTransform      (_tb(pos), m_instance->GetAngle()); else m_body.position        = _tb(pos); return *this; }
	Rigidbody2D& SetVelocity       (vec2  vel)      { if (m_instance) m_instance->SetLinearVelocity (_tb(vel));                         else m_body.linearVelocity  = _tb(vel); return *this; }
	Rigidbody2D& SetAngle          (float angle)    { if (m_instance) m_instance->SetTransform      (m_instance->GetPosition(), angle); else m_body.angle           = angle;    return *this; }
	Rigidbody2D& SetAngularVelocity(float avel)     { if (m_instance) m_instance->SetAngularVelocity(avel);                             else m_body.angularVelocity = avel;     return *this; }
	Rigidbody2D& SetFixedRotation  (bool  isFixed)  { if (m_instance) m_instance->SetFixedRotation  (isFixed);                          else m_body.fixedRotation   = isFixed;  return *this; }
	Rigidbody2D& SetDamping        (float damping)  { if (m_instance) m_instance->SetLinearDamping  (damping);                          else m_body.linearDamping   = damping;  return *this; }
	Rigidbody2D& SetAngularDamping (float adamping) { if (m_instance) m_instance->SetAngularDamping (adamping);                         else m_body.angularDamping  = adamping; return *this; }
	Rigidbody2D& SetDensity        (float density)  { m_density = density;                                                                                                      return *this; }

	void ApplyForce(vec2 force)                        { assert_in_world(); m_instance->ApplyForceToCenter(_tb(force), true); }
	void ApplyForce(vec2 force, vec2 offsetFromCenter) { assert_in_world(); m_instance->ApplyForce(_tb(force), _tb(GetPosition() + offsetFromCenter), true); }
	void ApplyTorque(float force)                      { assert_in_world(); m_instance->ApplyTorque(force, true); }

	// functions that should hide the box2d api, but dont right now

	void RemoveColliders() 
	{ 
		assert_in_world();
		b2Fixture* fix = m_instance->GetFixtureList();
		while (fix)
		{
			b2Fixture* next = fix->GetNext();
			m_instance->DestroyFixture(fix);
			fix = next;
		}
	}

	int GetColliderCount() const
	{
		assert_in_world();
		b2Fixture* fix = m_instance->GetFixtureList();
		int count = 0;
		while (fix) { fix = fix->GetNext(); count += 1; }
		return count;
	}

	b2Fixture* AddCollider(const b2Shape& shape)                { return AddCollider(shape, m_density); }
	b2Fixture* AddCollider(const b2Shape& shape, float density) { assert_in_world(); return m_instance->CreateFixture(&shape, density); }
	
	      b2Fixture* GetCollider(int i = 0)       { return GetCol(i); }
	const b2Fixture* GetCollider(int i = 0) const { return GetCol(i); }

	float GetMass()  const { assert_in_world(); return m_instance->GetMass(); }
	float GetSpeed() const { return length(_fb(m_instance ? m_instance->GetLinearVelocity() : m_body.linearVelocity)); }

private:
	void assert_in_world() const
	{
		assert(InWorld() && "Physics object has not been added to the world");
	}

	b2Fixture* GetCol(int index) const
	{
		assert_in_world();
		b2Fixture* fix = m_instance->GetFixtureList();
		for (int i = 0; i < index; i++) fix = fix->GetNext();
		return fix;
	}
};

struct PointQueryResult
{
	struct Result
	{
		Entity entity;
		float distance;
	};

	std::vector<Result> results;

	bool          HasResult()     const { return results.size() > 0; }
	const Result& FirstResult()   const { return results.at(0); }
	Entity        FirstEntiy()    const { return FirstResult().entity; }
	float         FirstDistance() const { return FirstResult().distance; }
};

struct RayQueryResult
{
	struct Result
	{
		Entity entity;
		float distance;
		vec2 point;
		vec2 normal;
	};

	std::vector<Result> results;

	bool          HasResult()     const { return results.size() > 0; }
	const Result& FirstResult()   const { return results.at(0); }
	Entity        FirstEntiy()    const { return FirstResult().entity; }
	float         FirstDistance() const { return FirstResult().distance; }
	vec2          FirstPoint()    const { return FirstResult().point; }
	vec2          FirstNormal()   const { return FirstResult().normal; }
};

struct PhysicsWorld
{
	b2World* m_world;

	PhysicsWorld()
	{
		m_world = new b2World();
	}

	~PhysicsWorld()
	{
		delete m_world;
	}

	Rigidbody2D& AddEntity(Entity& e)
	{
		if (!e.Has<Rigidbody2D>())
		{
			e.Add<Rigidbody2D>(e.Get<Transform2D>());
		}

		e.OnDestroy([this](Entity entity) 
		{ 
			Remove(entity); 
		});

		Rigidbody2D& body = e.Get<Rigidbody2D>();
		body.m_instance = m_world->CreateBody(&body.m_body);

		b2BodyUserData& data = body.m_instance->GetUserData();
		data.entityId     = e.Id();
		data.entityOwning = e.Owning();

		return body;
	}
	
	void Remove(Entity& e)
	{
		Rigidbody2D& body = e.Get<Rigidbody2D>();
		m_world->DestroyBody(body.m_instance);
		body.m_instance = nullptr;
	}

	void Step(float dt)
	{
		m_world->Step(dt, 8, 3);
	}

	RayQueryResult QueryRay(vec2 point, vec2 direction, float distance) const
	{
		return QueryRay(point, point + direction * distance);
	}

	RayQueryResult QueryRay(vec2 point, vec2 target) const
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


	// query an AABB
	PointQueryResult QueryPoint(vec2 point, float radius) const
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

	// yes moves
	//  no copys
	PhysicsWorld(PhysicsWorld&& move) noexcept
		: m_world (move.m_world)
	{
		move.m_world = nullptr;
	}
	PhysicsWorld& operator=(PhysicsWorld&& move) noexcept
	{
		m_world = move.m_world;
		move.m_world = nullptr;
		return *this;
	}
	PhysicsWorld(const PhysicsWorld& move) = delete;
	PhysicsWorld& operator=(const PhysicsWorld& move) = delete;

private:

	struct PointQueryCallback : b2QueryCallback
	{
		PointQueryResult result;
		vec2 point;

		PointQueryCallback(vec2 point)
			: point(point) {}

		bool ReportFixture(b2Fixture* fixture)
		{
			b2BodyUserData& data = fixture->GetBody()->GetUserData();
			EntityWorld* world = (EntityWorld*)data.entityOwning;

			result.results.push_back({
				world->Wrap(data.entityId),
				distance(_fb(fixture->GetBody()->GetPosition()), point)
			});

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
			b2BodyUserData& data = fixture->GetBody()->GetUserData();
			EntityWorld* world = (EntityWorld*)data.entityOwning;
			
			result.results.push_back({
				world->Wrap(data.entityId),
				fraction * length,
				_fb(point),
				_fb(normal)
			});

			return -1;
		}
	};
};

