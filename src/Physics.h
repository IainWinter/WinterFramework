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
	b2BodyDef m_body; // Init values for Physics Add
	b2Body* m_instance; // If null, not in physics world
	
	friend struct PhysicsWorld;

public:
	Rigidbody2D(
		Transform2D transform = {}
	)
		: m_instance    (nullptr)
		, LastTransform (transform) 
	{
		m_body.type = b2_dynamicBody;
		SetTransform(transform);
	}
	
	void SetTransform(Transform2D& transform)
	{
		SetPosition(transform.position);
		SetAngle   (transform.rotation);
	}

	// functions for setting properties for box2d...

	bool InWorld() const { return !!m_instance; }
	
	vec2  GetPosition()        const { return _fb(m_instance ? m_instance->GetPosition()        : m_body.position); }
	vec2  GetVelocity()        const { return _fb(m_instance ? m_instance->GetLinearVelocity()  : m_body.linearVelocity); }
	float GetAngle()           const { return     m_instance ? m_instance->GetAngle()           : m_body.angle; }
	float GetAngularVelocity() const { return     m_instance ? m_instance->GetAngularVelocity() : m_body.angularVelocity; }
	bool   IsFixedRotation()   const { return     m_instance ? m_instance->IsFixedRotation()    : m_body.fixedRotation; }

	void SetPosition(vec2 pos)          { if (m_instance) m_instance->SetTransform      (_tb(pos), m_instance->GetAngle()); else m_body.position = _tb(pos); }
	void SetVelocity(vec2 vel)          { if (m_instance) m_instance->SetLinearVelocity (_tb(vel));                         else m_body.linearVelocity = _tb(vel); }
	void SetAngle(float angle)          { if (m_instance) m_instance->SetTransform      (m_instance->GetPosition(), angle); else m_body.angle = angle; }
	void SetAngularVelocity(float avel) { if (m_instance) m_instance->SetAngularVelocity(avel);                             else m_body.angularVelocity = avel; }
	void SetFixedRotation(bool isFixed) { if (m_instance) m_instance->SetFixedRotation  (isFixed);                          else m_body.fixedRotation = isFixed;  }

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

	b2Fixture* AddCollider(const b2Shape& shape, float density = 1.f) { assert_in_world(); return m_instance->CreateFixture(&shape, density); }
	
	      b2Fixture* GetCollider(int i = 0)       { assert_in_world(); return m_instance->GetFixtureList() + i; }
	const b2Fixture* GetCollider(int i = 0) const { assert_in_world(); return m_instance->GetFixtureList() + i; }

	float GetMass() const { assert_in_world(); return m_instance->GetMass(); }

private:
	void assert_in_world() const
	{
		assert(InWorld() && "Physics object has not been added to the world");
	}
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
};

