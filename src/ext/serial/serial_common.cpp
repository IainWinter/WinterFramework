#include "ext/serial/serial_common.h"
#include "ext/serial/serial.h"

#include "ext/rendering/Sprite.h"
#include "ext/rendering/Camera.h"
#include "ext/rendering/Particle.h"

#include "ext/EntityPrefab.h"

#include "util/AABB.h"

#include "Physics.h"

#include <string>
#include <vector>

void write_Color(meta::serial_writer* serial, const Color& instance)
{
	serial->write(instance.as_u32);
}

void read_Color(meta::serial_reader* serial, Color& instance)
{
	serial->read(instance.as_u32);
}

void write_Rigidbody2D(meta::serial_writer* serial, const Rigidbody2D& instance)
{
    b2BodyDef def = instance.GetBodyDef();
    std::vector<meta::any> colliders;
    
    for (const r<Collider>& collider : instance.GetColliders())
    {
        switch (collider->GetType())
        {
            case Collider::tCircle:
                colliders.push_back(meta::any(&collider->As<CircleCollider>()));
                break;
            default:
                log_io("e~Error, type not supported by write_Rigidbody2D");
                break;
        }
    }
    
	serial->pseudo().begin<Rigidbody2D>()
		.member("body", def)
		.member("colliders", colliders)
		.end();
}

void read_Rigidbody2D(meta::serial_reader* serial, Rigidbody2D& instance)
{
    b2BodyDef def;
    std::vector<meta::any> colliders;
    
	serial->pseudo().begin<Rigidbody2D>()
		.member("body", def)
		.member("colliders", colliders)
		.end();

    for (const meta::any& collider : colliders)
    {
        instance.AddCollider(*(Collider*)collider.data());
    }
    
    instance.SetPreInit(def);
}

void write_CircleCollider(meta::serial_writer * writer, const CircleCollider& instance)
{
	vec2 center = instance.GetCenter();
	float radius = instance.GetRadius();

	writer->pseudo().begin<CircleCollider>()
		.member("radius", radius)
		.member("center", center)
		.end();
}

void read_CircleCollider(meta::serial_reader* writer, CircleCollider& instance)
{
	vec2 center;
	float radius;

	writer->pseudo().begin<CircleCollider>()
		.member("radius", radius)
		.member("center", center)
		.end();

	instance.SetRadius(radius);
	instance.SetCenter(center);
}

void write_Texture(meta::serial_writer* serial, const Texture& instance)
{
	serial->pseudo()
		.begin<Texture>()
		.member("width", instance.Width())
		.member("height", instance.Height())
		.member("usage", instance.UsageType())
		.member("filter", instance.FilterType())

		// storing a copy of the host is kinda annoying because if you edit the texture
		// you cannot save it properly

		.custom<u8>("host", [&]() { serial->write_bytes((const char*)instance.Pixels(), instance.BufferSize()); })
		.end();
}

void read_Texture(meta::serial_reader* serial, Texture& instance)
{
	int width, height;
	Texture::Usage usage;
	Texture::Filter filter;

	meta::pseudo_reader r = serial->pseudo()
		.begin<Texture>()
		.member("width",  width)
		.member("height", height)
		.member("usage",  usage)
		.member("filter", filter);

	instance = Texture(width, height, usage);
	instance.SetFilter(filter);

	r.custom<u8>("host", [&]() { serial->read_bytes((char*)instance.Pixels(), instance.BufferSize()); })
	 .end();
}

void write_TextureAtlas(meta::serial_writer* serial, const TextureAtlas& atlas)
{
	std::string sourceName = atlas.source.Name();

	serial->pseudo()
		.begin<TextureAtlas>()
		.member("source", sourceName)
		.member("bounds", atlas.bounds)
		.end();
}

void read_TextureAtlas(meta::serial_reader* serial, TextureAtlas& atlas)
{
	std::string sourceName;

	serial->pseudo()
		.begin<TextureAtlas>()
		.member("source", sourceName)
		.member("bounds", atlas.bounds)
		.end();

	atlas.source = Asset::LoadFromFile<Texture>(sourceName);
}

void write_EntityPrefab(meta::serial_writer* serial, const EntityPrefab& entityPrefab)
{
	serial->write(entityPrefab.GetComponents());
}

void read_EntityPrefab(meta::serial_reader* serial, EntityPrefab& entityPrefab)
{
	std::vector<meta::any> components;
	serial->read(components);

	entityPrefab = {};

	for (const meta::any& component : components)
	{
		entityPrefab.Add(component);
	}
}

void register_common_types()
{
	using namespace meta;

	// glm

	describe< vec2>().name( "vec2").member<& vec2::x>("x").member<& vec2::y>("y");
	describe<ivec2>().name("ivec2").member<&ivec2::x>("x").member<&ivec2::y>("y");
	describe<bvec2>().name("bvec2").member<&bvec2::x>("x").member<&bvec2::y>("y");

	describe< vec3>().name( "vec3").member<& vec3::x>("x").member<& vec3::y>("y").member<& vec3::z>("z");
	describe<ivec3>().name("ivec3").member<&ivec3::x>("x").member<&ivec3::y>("y").member<&ivec3::z>("z");
	describe<bvec3>().name("bvec3").member<&bvec3::x>("x").member<&bvec3::y>("y").member<&bvec3::z>("z");

	describe< vec4>().name( "vec4").member<& vec4::x>("x").member<& vec4::y>("y").member<& vec4::z>("z").member<& vec4::w>("w");
	describe<ivec4>().name("ivec4").member<&ivec4::x>("x").member<&ivec4::y>("y").member<&ivec4::z>("z").member<&ivec4::w>("w");
	describe<bvec4>().name("bvec4").member<&bvec4::x>("x").member<&bvec4::y>("y").member<&bvec4::z>("z").member<&bvec4::w>("w");

	describe<Transform2D>()
		.name("Transform2D")
		.prop("is_component", true)
		.member<&Transform2D::position>("position")
		.member<&Transform2D::z>("z")
		.member<&Transform2D::scale>("scale")
		.member<&Transform2D::rotation>("rotation");

	describe<Color>()
		.name("Color")
		.member<&Color::r>("r")
		.member<&Color::g>("g")
		.member<&Color::b>("b")
		.member<&Color::a>("a")
		.member<&Color::as_u32>("u32")
		.custom_write(&write_Color)
		.custom_read(&read_Color);

	describe<aabb2D>()
		.name("aabb2D")
		.member<&aabb2D::min>("min")
		.member<&aabb2D::max>("max");

	describe<EntityMeta>()
		.name("EntityMeta")
		.prop("is_component", true)
		.member<&EntityMeta::name>("name")/*
		.member<&EntityMeta::parent>("parent")*/;

	describe<Sprite>()
		.name("Sprite")
		.prop("is_component", true)
		.member<&Sprite::tint>    ("tint")
		.member<&Sprite::source>  ("source")
		.member<&Sprite::uvOffset>("uvOffset").prop("min", 0.f).prop("max", 1.f)
		.member<&Sprite::uvScale> ("uvScale") .prop("min", 0.f).prop("max", 1.f);

	describe<Particle>()
		.name("Particle")
		.prop("is_component", true)
		.member<&Particle::atlas>("atlas")
		.member<&Particle::framesPerSecond>("framesPerSecond")
		.member<&Particle::frameCurrent>("frameCurrent")
		.member<&Particle::life>("life")
		.member<&Particle::lifeCurrent>("lifeCurrent")
		.member<&Particle::original>("original")
		.member<&Particle::tints>("tints")
		.member<&Particle::tint>("tint");

	describe<ParticleSpawner>()
		.name("ParticleSpawner")
		.prop("is_component", true)
		.member<&ParticleSpawner::particle>("particle")
		.member<&ParticleSpawner::weight>("weight");

	describe<ParticleEmitter>()
		.name("ParticleEmitter")
		.prop("is_component", true)
		.member<&ParticleEmitter::timeBetweenSpawn>("timeBetweenSpawn")
		.member<&ParticleEmitter::currentTime>("currentTime")
		.member<&ParticleEmitter::enableAutoEmit>("enableAutoEmit")
		.member<&ParticleEmitter::spawners>("spanners");

	describe<Camera>()
		.name("Camera")
		.prop("is_component", true)
		.member<&Camera::x>("x")
		.member<&Camera::y>("y")
		.member<&Camera::z>("z")
		.member<&Camera::width>("width")
		.member<&Camera::height>("height")
		.member<&Camera::near>("near")
		.member<&Camera::far>("far")
		.member<&Camera::aspect>("aspect")
		.member<&Camera::is_ortho>("is_ortho");

	describe<b2Vec2>()
		.name("b2Vec2")
		.member<&b2Vec2::x>("x")
		.member<&b2Vec2::y>("y");

	describe<b2BodyDef>()
		.name("b2BodyDef")
		.member<&b2BodyDef::type>("type")
		.member<&b2BodyDef::position>("position")
		.member<&b2BodyDef::angle>("angle")
		.member<&b2BodyDef::linearVelocity>("linearVelocity")
		.member<&b2BodyDef::angularVelocity>("angularVelocity")
		.member<&b2BodyDef::linearDamping>("linearDamping")
		.member<&b2BodyDef::angularDamping>("angularDamping")
		.member<&b2BodyDef::allowSleep>("allowSleep")
		.member<&b2BodyDef::awake>("awake")
		.member<&b2BodyDef::fixedRotation>("fixedRotation")
		.member<&b2BodyDef::bullet>("bullet")
		.member<&b2BodyDef::enabled>("enabled")
		//.member<&b2BodyDef::userData>("userData")
		.member<&b2BodyDef::gravityScale>("gravityScale");
        
    describe<b2CircleShape>()
        .name("b2CircleShape")
        .member<&b2CircleShape::m_type>("m_type")
        .member<&b2CircleShape::m_radius>("m_radius")
        .member<&b2CircleShape::m_p>("m_p");
        
	describe<Rigidbody2D>()
		.name("Rigidbody2D")
		.prop("is_component", true)
		.custom_read(&read_Rigidbody2D)
		.custom_write(&write_Rigidbody2D);

	describe<CircleCollider>()
		.name("CircleCollider")
		.custom_write(&write_CircleCollider)
		.custom_read(&read_CircleCollider);

	describe<Texture>()
		.name("Texture")
		.custom_write(&write_Texture)
		.custom_read(&read_Texture);

	describe<TextureAtlas::Bounds>()
		.name("TextureAtlas::Bounds")
		.member<&TextureAtlas::Bounds::uvOffset>("uvOffset")
		.member<&TextureAtlas::Bounds::uvScale>("uvScale");

	describe<TextureAtlas>()
		.name("TextureAtlas")
		.member<&TextureAtlas::source>("source")
		.member<&TextureAtlas::bounds>("bounds")
		.custom_write(&write_TextureAtlas)
		.custom_read(&read_TextureAtlas);

	describe<EntityPrefab>()
		.name("EntityPrefab")
		.custom_write(&write_EntityPrefab)
		.custom_read(&read_EntityPrefab);

	//describe<b2BodyUserData>()
	//	.name("b2BodyUserData")
	//	.member<&b2BodyUserData::entityId>("entityId")
	//	.member<&b2BodyUserData::entityId>("entityId");
}
