#pragma once

#include "Common.h"
#include "box2d/box2d.h"

b2Vec2 _tb(const vec2& v)
{
	return b2Vec2(v.x, v.y);
}

vec2 _fb(const b2Vec2& v)
{
	return vec2(v.x, v.y);
}

struct Rigidbody2D
{
public:
	Transform2D LastTransform;
	
private:
	friend class PhysicsWorld;

	// Init values for Physics Add
	b2BodyDef m_body;

	// If null, not in physics world
	b2Body* m_instance;

public:
	Rigidbody2D(
		Transform2D transform = {}
	)
		: m_instance    (nullptr)
		, LastTransform (transform) 
	{
		m_body.type = b2_dynamicBody;
		m_body.position = b2Vec2(transform.x, transform.y);
		m_body.angle = transform.r;
	}
	
	// functions for setting properties for box2d...

	bool InWorld() const { return !!m_instance; }
	
	vec2 GetPosition() const { assert_in_world(); return _fb(m_instance->GetPosition()); }
	vec2 GetVelocity() const { assert_in_world(); return _fb(m_instance->GetLinearVelocity()); }
	float GetAngle()           const { assert_in_world(); return m_instance->GetAngle(); }
	float GetAngularVelocity() const { assert_in_world(); return m_instance->GetAngularVelocity(); }

	void SetPosition(vec2 pos) const { assert_in_world(); return m_instance->SetTransform(_tb(pos), m_instance->GetAngle()); }
	void SetVelocity(vec2 vel) const { assert_in_world(); return m_instance->SetLinearVelocity(_tb(vel)); }
	void SetAngle(float angle) const { assert_in_world(); return m_instance->SetTransform(m_instance->GetPosition(), angle); }
	void SetAngularVelocity(float avel) const { assert_in_world(); return m_instance->SetAngularVelocity(avel); }

	void ApplyForce(vec2 force) { assert_in_world(); m_instance->ApplyForceToCenter(_tb(force), true); }

	// functions that should hide the box2d api, but dont right now
	// could add density...

	void AddCollider(const b2Shape& shape) { assert_in_world(); m_instance->CreateFixture(&shape, 1.f); }

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

	// adds a Rigdbody2D and an on_destroy to remove from physics at eol
	Rigidbody2D& AddEntity(Entity& e)
	{
		e.Add<Rigidbody2D>(e.Get<Transform2D>());
		//e.on_destroy([this](Entity e) { Remove(e.Get<Rigidbody2D>()); });

		Rigidbody2D& body = e.Get<Rigidbody2D>();
		Add(body);
		return body;
	}

	void Add(Rigidbody2D& body)
	{
		body.m_instance = m_world->CreateBody(&body.m_body);
	}
	
	void Remove(Rigidbody2D& body)
	{
		m_world->DestroyBody(body.m_instance);
		body.m_instance = nullptr;
	}

	void Step(float dt)
	{
		m_world->Step(dt, 8, 3);
	}

	Entity CreatePhysicsEntity(const Transform2D& transform = {})
	{
		Entity e = GetWorld().Create().AddAll(transform, Rigidbody2D(transform));
		Add(e.Get<Rigidbody2D>());
		return e;
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

