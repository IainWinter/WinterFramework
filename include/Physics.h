#pragma once

#include "Entity.h"

#include "util/function.h"
#include "util/ArrayView.h"
#include "util/math.h"
#include "util/ref.h"
#include "util/stl.h"
#include "util/Transform.h"
#include "util/AABB.h"

// may be able to fwd
#include "box2d/box2d.h"

b2Vec2 _tb(const   vec2& v);
vec2   _fb(const b2Vec2& v);

struct CollisionInfo
{
	Entity me;
	Entity other;
	b2Contact* contact;
	bool isA;
};

struct WorldCollisionInfo
{
	Entity A;
	Entity B;
	b2Contact* contact;
};

using OnWorldCollisionFunc = std::function<void(WorldCollisionInfo)>;
using OnCollisionFunc = std::function<void(CollisionInfo)>;

// fwd
struct Rigidbody2D;
struct PhysicsWorld;

///// colliders

//	The bridge between box2d and winter framework
//
struct ColliderAttachment
{
private:
	b2Fixture* m_fixture; // not owned

	// Config before we attach this to a body

	r<b2FixtureDef> m_tempDef;   // owned
	r<b2Shape>      m_tempShape; // owned, we also need this as the def doesn't own the shape ptr

public:
	ColliderAttachment();

	void InitTemp(b2CircleShape& circe);
	void InitTemp(b2PolygonShape& polygon);

	void InitFixture(Rigidbody2D& body);
	void DnitFixture(Rigidbody2D& body);

	b2Shape& GetShape() const; // non const ref from inner datatype

	vec2 GetBodyPosition() const;
	bool OnBody() const;

	// settings
	void SetDensity(float density);

	float GetDensity() const;
};

//	The base type of a collider
//
struct Collider
{
public:
	enum ColliderType
	{
		tCircle,
		tHull,
		tPolygon
	};

private:
	ColliderType m_type;

protected:
	std::vector<ColliderAttachment> m_attachments;

public:
	Collider(ColliderType type);

	// allow no copies on stack, only MakeCopy
	//Collider(const Collider& copy) = delete;
	//Collider& operator=(const Collider& copy) = delete;

	virtual ~Collider() {}

	virtual r<Collider> MakeCopy() const = 0;

public:
	virtual vec2 GetWorldCenter() const = 0;
	virtual vec2 GetCenter() const = 0;

	ColliderType GetType() const;

	template<typename _t>
	_t& As() const
	{
		static_assert(
			   _t::cType::value == tCircle 
			|| _t::cType::value == tHull 
			|| _t::cType::value == tPolygon
		);

		return *(_t*)this;
	}

	// Settings

	Collider& SetDensity(float density, int attachment = -1);
	float GetDensity(int attachment = -1) const;

private:
	friend struct Rigidbody2D;
	friend struct PhysicsWorld;

	virtual void AddToBody(Rigidbody2D& body) = 0;
	virtual void RemoveFromBody(Rigidbody2D& body) = 0;
};

//	A circle with a radius and origin
//
struct CircleCollider : Collider
{
	using cType = constant<tCircle>;

	CircleCollider(float radius = 1.f, vec2 center = vec2(0.f, 0.f));

	r<Collider> MakeCopy() const override;

	vec2 GetWorldCenter() const override;

	float GetRadius() const;
	vec2  GetCenter() const override;

	CircleCollider& SetRadius(float radius);
	CircleCollider& SetCenter(vec2 center);

private:
	b2CircleShape& GetShape() const;

	void AddToBody(Rigidbody2D& body) override;
	void RemoveFromBody(Rigidbody2D& body) override;
};

//	A convex polygon. There is a hard limit on the number of points equal to MaxPoints 
//
struct HullCollider : Collider
{
	using cType = constant<tHull>;
	using MaxPoints = constant<b2_maxPolygonVertices>;

	HullCollider();

	r<Collider> MakeCopy() const override;

	vec2 GetWorldCenter() const override;
	vec2 GetCenter() const override;

	// return a view of a fixed size array of points
	ArrayView<vec2> GetPoints() const;

	// set points as a box
	HullCollider& SetPointsBox(float w = 1.f, float h = 1.f, float a = 0.f, vec2 center = vec2(0.f));

	// set points from an array
	HullCollider& SetPoints(const ArrayView<vec2>& list, vec2 scale = vec2(1.f));

	// set points from a range
	template<typename _itr>
	HullCollider& SetPoints(const _itr& begin, const _itr& end, vec2 scale = vec2(1.f), vec2 offset = vec2(0.f))
	{
		// convert points to box2d
		std::vector<b2Vec2> converted;

		for (_itr b = begin; b != end; ++b)
		{
			b2Vec2 b2 = _tb(*b * scale + offset);
			converted.push_back(b2);
		}

		GetShape().Set(converted.data(), (int32)converted.size());
		return *this;
	}

private:
	b2PolygonShape& GetShape() const;

	void AddToBody(Rigidbody2D& body) override;
	void RemoveFromBody(Rigidbody2D& body) override;
};

//struct PolygonCollider : Collider
//{
//	using cType = constant<tPolygon>;
//
//	std::vector<HullCollider> m_hulls;
//
//	PolygonCollider();
//	PolygonCollider(const std::vector<vec2>& points);
//	PolygonCollider(const ArrayView<vec2>& points);
//
//	r<Collider> MakeCopy() const override;
//
//	void AddToBody(Rigidbody2D& body) override;
//	void RemoveFromBody(Rigidbody2D& body) override;
//
//	vec2 GetWorldCenter() const override;
//	vec2 GetCenter() const override;
//
//	ArrayView<vec2> GetPoints() const;
//
//	HullCollider& SetPoints(const std::vector<vec2>& points);
//	HullCollider& SetPoints(const ArrayView<vec2>& points);
//
//	//HullCollider& SetPointsBox(float w = 1.f, float h = 1.f, float a = 0.f, vec2 center = vec2(0.f));
//
//private:
//	b2PolygonShape& GetShape() const;
//
//	void InitPoints(const ArrayView<vec2>& points);
//};

struct LastPositionState2D
{
	vec2 position = vec2(0.f);
	float angle = 0;
	float timePoint = 0;
	float deltaTime = 0;
};

struct Rigidbody2D
{
public:
	func<void(CollisionInfo)> OnCollision;

	enum Type
	{
		Static,
		Kinematic,
		Dynamic
	};

private:
	b2Body* m_instance;

	// If null, not in physics world. This pointer is not owned
	b2World* m_world;

	// this is for serialization loading
	b2BodyDef m_preinit;
    std::vector<b2Shape*> colliders;
    
	// default value on each new collider, if the new collider had 0 density
	float m_density;
	bool m_collisionEnabled;

	// last physics tick location for interpolation
	LastPositionState2D m_last;

	// attached colliders
	std::vector<r<Collider>> m_colliders;

	friend struct PhysicsWorld;
	friend struct ColliderAttachment;
    
	aabb2D m_aabb;

public:
	Rigidbody2D();
	
	void RemoveFromWorld();

	void move_into(Rigidbody2D&& other);
	void copy_into(const Rigidbody2D& other);

	Rigidbody2D& SetTransform(Transform2D& transform);

	const LastPositionState2D& GetLast() const;
	void UpdateLast(float deltaTime, float timePoint);

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
	float GetMass()            const;
	float GetSpeed()           const;
	float GetAngularSpeed()    const;
	Type  GetType()            const;

	Rigidbody2D& SetPosition       (vec2  pos);
	Rigidbody2D& SetVelocity       (vec2  vel);
	Rigidbody2D& SetAngle          (float angle);
	Rigidbody2D& SetAngularVelocity(float avel);
	Rigidbody2D& SetDamping        (float damping);
	Rigidbody2D& SetAngularDamping (float adamping);
	Rigidbody2D& SetEnableCollision(bool  isEnabled);
	Rigidbody2D& SetRotationFixed  (bool  isFixed);
	Rigidbody2D& SetDensity        (float density);
	Rigidbody2D& SetType           (Type type);
	Rigidbody2D& SetEntity         (int id);

	Rigidbody2D& SetIsBullet       (bool isBullet);

	// colliders

	r<Collider> AddCollider(const Collider& collider);
	void RemoveCollider(const r<Collider>& collider);

	void ClearColliders();

	int GetColliderCount() const;

	      r<Collider>& GetCollider(int colliderIndex = 0);
	const r<Collider>& GetCollider(int colliderIndex = 0) const;
	
	const std::vector<r<Collider>>& GetColliders() const;
	
	void UpdateBounds(aabb2D bounds);
	aabb2D GetBounds() const;

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

		int v2EntityId;
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
		b2Fixture* fixture; // temp for new entity system
		Entity entity;
		float distance;
		vec2 point;
		vec2 normal;

		int v2EntityId;
	};

	std::vector<Result> results;

	bool          HasResult()     const;
	const Result& FirstResult()   const;
	Entity        FirstEntity()    const;
	float         FirstDistance() const;
	vec2          FirstPoint()    const;
	vec2          FirstNormal()   const;
};

struct PhysicsWorld
{
private:
	b2World* m_world;
	func<void(WorldCollisionInfo)> m_onCollision;

public:
	PhysicsWorld();
	~PhysicsWorld();

	void ReallocWorld();

	void Add(EntityWith<Rigidbody2D> e);
	void Remove(Entity& e);

	Rigidbody2D CreateBody();
	void RegisterBody(Rigidbody2D* body);

	void Tick(float dt);

	  RayQueryResult QueryRay  (vec2 point, vec2 direction, float distance) const;
	  RayQueryResult QueryRay  (vec2 point, vec2 target) const;
	PointQueryResult QueryPoint(vec2 point, float radius) const;

	// yes moves
	PhysicsWorld(PhysicsWorld&& move) noexcept;
	PhysicsWorld& operator=(PhysicsWorld&& move) noexcept;
	
	//  no copy
	PhysicsWorld(const PhysicsWorld& move) = delete;
	PhysicsWorld& operator=(const PhysicsWorld& move) = delete;

	template<typename _c1, typename _c2>
	OnWorldCollisionFunc OnCollision(const OnWorldCollisionFunc& func)
	{
		OnWorldCollisionFunc f = [func](WorldCollisionInfo info)
		{
			// this is wasteful because this causes a loop in m_onCollision for each pair
			// better solve would be a pair hash that doesn't care about order, then a map of pair hash -> func
			// but this is ez

			if (   !info.A.Get<Rigidbody2D>().IsCollisionEnabled()
				|| !info.B.Get<Rigidbody2D>().IsCollisionEnabled()) // exit if collision isn't enabled, need to make a sensor bool so those can work still
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

	void RemoveOnCollision(const OnWorldCollisionFunc& func)
	{
		m_onCollision -= func;
	}

	void FireOnCollision(const WorldCollisionInfo& info)
	{
		m_onCollision(info);
	}
};
