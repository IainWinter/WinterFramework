#include "ext/serial/serial_common.h"

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
    serial->class_begin(meta::get_class<Rigidbody2D>());
        
    b2BodyDef def = instance.GetBodyDef();
	serial->write(def);
        
    serial->class_end();
        
    std::vector<meta::any> colliders;
        
    for (const b2Fixture* fix = instance.GetCollider(); fix; fix = fix->GetNext())
    {
        colliders.push_back(meta::any());
    }
}

void read_Rigidbody2D(meta::serial_reader* serial, Rigidbody2D& instance)
{
	b2BodyDef def;
	serial->read(def);
	instance.SetPreInit(def);
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

	// serial, this should go in the create context

	describe<Transform2D>()
		.name("Transform2D")
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
		.member<&EntityMeta::name>("name")/*
		.member<&EntityMeta::parent>("parent")*/;

	describe<Sprite>()
		.name("Sprite")
		.member<&Sprite::tint>    ("tint")
		.member<&Sprite::uvOffset>("uvOffset").min(0.f).max(1.f)
		.member<&Sprite::uvScale> ("uvScale") .min(0.f).max(1.f);

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
		.custom_read(&read_Rigidbody2D)
		.custom_write(&write_Rigidbody2D);

	//describe<b2BodyUserData>()
	//	.name("b2BodyUserData")
	//	.member<&b2BodyUserData::entityId>("entityId")
	//	.member<&b2BodyUserData::entityId>("entityId");
}
