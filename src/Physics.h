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

struct WorldCollsionInfo
{
	Entity A;
	Entity B;
	b2Contact* contact;
};

// to pass collider from framework to box2d, the body needs to be already created, so we need to store a b2Shape* but also mamnage it because we need to delete
// it when it gets added to the body

// fwd
struct Rigidbody2D;

struct Collider
{
protected:
    b2Fixture* m_fixture;
    b2Shape* m_pass;       // this will be inited to hold temp values before being copied into the fixture
    
public:
    Collider()
        : m_fixture (nullptr)
        , m_pass    (nullptr)
    {}
    
    virtual ~Collider()
    {
        if (!OnBody()) // if this has never been added to a body, delete the temp shape
        {
            delete m_pass;
        }
    }
    
    bool OnBody() const
    {
        return !!m_pass;
    }
    
    void AddToBody(Rigidbody2D& body);
};

struct CircleCollider : Collider
{
    CircleCollider(float radius, vec2 origin = vec2(0, 0))
        : Collider()
    {
        m_pass = new b2CircleShape();
        GetShape().m_radius = radius;
        GetShape().m_p = _tb(origin);
    }
    
    float GetRadius() const { return GetShape().m_radius; }
    vec2  GetOrigin() const { return _fb(GetShape().m_p); }
    
    CircleCollider& SetRadius(float radius) { GetShape().m_radius = radius; return *this; }
    CircleCollider& SetOrigin(vec2 origin) { GetShape().m_p = _tb(origin); return *this; }
    
    b2CircleShape& GetShape() const
    {
        return *(b2CircleShape*) (OnBody() ? m_fixture->GetShape() : m_pass);
    }
};

struct Rigidbody2D
{
public:
	func<void(CollisionInfo)> OnCollision;

private:
	// If null, not in physics world
	b2Body* m_instance;
	
	// this is for serialization loading
	b2BodyDef m_preinit;
    std::vector<b2Shape*> colliders;
    
	// deafult value on each new collider, if the new collider had 0 density
	float m_density;
	bool m_collisionEnabled;

	// last physics tick location for interpolation
	Transform2D m_lastTransform;

	friend struct PhysicsWorld;
    friend struct Collider;
    
public:
	Rigidbody2D();
	
	void SetTransform(Transform2D& transform);

	const Transform2D& GetLastTransform() const;
	void UpdateLastTransform();

	void ApplyForce (vec2 force);
	void ApplyForce (vec2 force, vec2 offsetFromCenter);
	void ApplyTorque(float force);
	
	vec2  GetPosition()        const;
	vec2  GetVelocity()        const;
	float GetAngle()           const;
	float GetAngularVelocity() const;
	float GetDamping()         const;
	float GetAngularDamping()  const;
	bool  IsRotationFixed()    const;
	bool  IsCollisionEnabled() const;
	float GetDensity()         const;

	Rigidbody2D& SetPosition       (vec2  pos);
	Rigidbody2D& SetVelocity       (vec2  vel);
	Rigidbody2D& SetAngle          (float angle);
	Rigidbody2D& SetAngularVelocity(float avel);
	Rigidbody2D& SetDamping        (float damping);
	Rigidbody2D& SetAngularDamping (float adamping);
	Rigidbody2D& SetEnableCollision(bool  isEnabled);
	Rigidbody2D& SetRotationFixed  (bool  isFixed);
	Rigidbody2D& SetDensity        (float density);

	float GetMass()            const;
	float GetSpeed()           const;
	float GetAngularSpeed()    const;

	void RemoveColliders();
	int GetColliderCount() const;

	// functions that should hide the box2d api, but dont right now
    
	b2Fixture* AddCollider(const b2Shape& shape);
	b2Fixture* AddCollider(const b2Shape& shape, float density);
	
	      b2Fixture* GetCollider(int i = 0);
	const b2Fixture* GetCollider(int i = 0) const;

	void SetPreInit(const b2BodyDef& def);
	
	// return a body def that can be used to serialize this Rigidbody
	b2BodyDef GetBodyDef() const;

private:
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
	Entity        FirstEntity()    const;
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
	Entity        FirstEntity()    const;
	float         FirstDistance() const;
	vec2          FirstPoint()    const;
	vec2          FirstNormal()   const;
};

using OnCollisionFunc = std::function<void(WorldCollsionInfo)>;

struct PhysicsWorld
{
private:
	b2World* m_world;
	func<void(WorldCollsionInfo)> m_onCollision;

public:
	PhysicsWorld();
	~PhysicsWorld();

	void ReallocWorld();

	void Add(EntityWith<Rigidbody2D> e);
	void Remove(Entity& e);

	void Tick(float dt);

	  RayQueryResult QueryRay  (vec2 point, vec2 direction, float distance) const;
	  RayQueryResult QueryRay  (vec2 point, vec2 target) const;
	PointQueryResult QueryPoint(vec2 point, float radius) const;

	// yes moves
	PhysicsWorld(PhysicsWorld&& move) noexcept;
	PhysicsWorld& operator=(PhysicsWorld&& move) noexcept;
	
	//  no copys
	PhysicsWorld(const PhysicsWorld& move) = delete;
	PhysicsWorld& operator=(const PhysicsWorld& move) = delete;

	template<typename _c1, typename _c2>
	OnCollisionFunc OnCollision(const OnCollisionFunc& func)
	{
		OnCollisionFunc f = [func](WorldCollsionInfo info)
		{
			// this is wastefull because this causes a loop in m_onCollision for each pair
			// better solve would be a pair hash that doesnt care about order, then a map of pair hash -> func
			// but this is ez

			if (   !info.A.Get<Rigidbody2D>().IsCollisionEnabled()
				|| !info.B.Get<Rigidbody2D>().IsCollisionEnabled()) // exit if collision isnt enabled, need to make a sensor bool so those can work still
			{
				return;
			}

			if (   (info.A.Has<_c1>() && info.B.Has<_c2>())
				|| (info.A.Has<_c2>() && info.B.Has<_c1>()))
			{
				func(info);
			}
		};

		m_onCollision += f;

		return f;
	}

	void RemoveOnCollision(const OnCollisionFunc& func)
	{
		m_onCollision -= func;
	}

	void FireOnCollision(const WorldCollsionInfo& info)
	{
		m_onCollision(info);
	}
};
