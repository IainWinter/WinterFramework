#pragma once

#include "Common.h"
#include "Entity.h"
#include "box2d/box2d.h"
#include "util/function.h"

b2Vec2 _tb(const   vec2& v);
vec2   _fb(const b2Vec2& v);

struct CollisionInfo
{
	Entity me;
	Entity other;
	b2Contact* contact;
	bool isA;
};

struct Rigidbody2D
{
public:
	Transform2D LastTransform;
	callback<void(CollisionInfo)> OnCollision;

private:
	b2BodyDef m_body;   // Init values for Physics Add
	b2Body* m_instance; // If null, not in physics world
	float m_density;    // deafult value on each new collider, if the new collider had 0 density
	bool m_collisionEnabled;

	friend struct PhysicsWorld;

public:
	Rigidbody2D(Transform2D transform = {});
	
	void SetTransform(Transform2D& transform);

	// functions for setting properties for box2d...

	bool InWorld()            const;
	bool IsFixedRotation()    const;
	bool IsCollisionEnabled() const;

	void ApplyForce (vec2 force);
	void ApplyForce (vec2 force, vec2 offsetFromCenter);
	void ApplyTorque(float force);
	
	vec2  GetPosition()        const;
	vec2  GetVelocity()        const;
	float GetAngle()           const;
	float GetAngularVelocity() const;
	float GetDamping()         const;
	float GetAngularDamping()  const;
	float GetMass()            const;
	float GetSpeed()           const;

	Rigidbody2D& SetPosition       (vec2  pos);
	Rigidbody2D& SetVelocity       (vec2  vel);
	Rigidbody2D& SetAngle          (float angle);
	Rigidbody2D& SetAngularVelocity(float avel);
	Rigidbody2D& SetFixedRotation  (bool  isFixed);
	Rigidbody2D& SetDamping        (float damping);
	Rigidbody2D& SetAngularDamping (float adamping);
	Rigidbody2D& SetDensity        (float density);
	Rigidbody2D& SetEnableCollision(bool respond);

	void RemoveColliders();
	int GetColliderCount() const;

	// functions that should hide the box2d api, but dont right now

	b2Fixture* AddCollider(const b2Shape& shape);
	b2Fixture* AddCollider(const b2Shape& shape, float density);
	
	      b2Fixture* GetCollider(int i = 0);
	const b2Fixture* GetCollider(int i = 0) const;

private:
	void assert_in_world() const;
	b2Fixture* GetCol(int index) const;
	b2Fixture* GetColList() const;
};

struct PointQueryResult
{
	struct Result
	{
		Entity entity;
		float distance;
	};

	std::vector<Result> results;

	bool          HasResult()     const;
	const Result& FirstResult()   const;
	Entity        FirstEntiy()    const;
	float         FirstDistance() const;
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

	bool          HasResult()     const;
	const Result& FirstResult()   const;
	Entity        FirstEntiy()    const;
	float         FirstDistance() const;
	vec2          FirstPoint()    const;
	vec2          FirstNormal()   const;
};

struct PhysicsWorld
{
	b2World* m_world;

	PhysicsWorld();
	~PhysicsWorld();

	Rigidbody2D& AddEntity(Entity& e);
	void Remove(Entity& e);

	void Step(float dt);

	  RayQueryResult QueryRay  (vec2 point, vec2 direction, float distance) const;
	  RayQueryResult QueryRay  (vec2 point, vec2 target) const;
	PointQueryResult QueryPoint(vec2 point, float radius) const;

	// yes moves
	PhysicsWorld(PhysicsWorld&& move) noexcept;
	PhysicsWorld& operator=(PhysicsWorld&& move) noexcept;
	
	//  no copys
	PhysicsWorld(const PhysicsWorld& move) = delete;
	PhysicsWorld& operator=(const PhysicsWorld& move) = delete;
};