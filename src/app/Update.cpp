#include "app/Update.h"
#include "app/Application.h"

#ifdef _WIN32
#   define _break(on) if ((int)m_break & on) { (int&)m_break &= ~on; __debugbreak(); }
#else
#   define _break(on) ;
#endif

void SystemBase::_Init(SceneNode* scene)
{
	log_app("i~Init system");

	m_scene = scene;
	m_state = SYSTEM_INIT;

	_break(ON_INIT);
	Init();
}

void SystemBase::_Dnit()
{
	log_app("i~Dnit system");

	_break(ON_DNIT);
	Dnit();

	m_state = SYSTEM_DNIT;
}

void SystemBase::_OnAttach()
{
	log_app("i~Attach system");

	m_state = SYSTEM_ATTACHED;

	_break(ON_ATTACH);
	OnAttach();
}

void SystemBase::_OnDetach()
{
	log_app("i~Detach system");

	_break(ON_DETACH);
	OnDetach();

	m_scene->bus.Detach(this);
	m_state = SYSTEM_DETACHED;
}

void SystemBase::_Update()
{
	//log_app("i~Update system");

	_break(ON_UPDATE);
	Update();
}

void SystemBase::_FixedUpdate()
{
	//log_app("i~Fixed update system");

	_break(ON_FIXEDUPDATE);
	FixedUpdate();
}

void SystemBase::_UI()
{
	//log_app("i~UI system");

	_break(ON_UI);
	UI();
}

void SystemBase::_Debug()
{
	//log_app("i~Debug system");

	_break(ON_DEBUG);
	Debug();
}

Entity SystemBase::CreateEntity()
{
	return m_scene->entities.Create();
}

Entity SystemBase::CreateEntity(const EntityPrefab& prefab)
{
	Entity entity = CreateEntity();

	for (const meta::any& any : prefab.GetComponents())
		entity.Add(any);

	return entity;
}

Entity SystemBase::CreateEntity(const a<EntityPrefab>& prefab)
{
	return CreateEntity(*prefab.ptr());
}

Entity SystemBase::WrapEntity(u32 entityId)
{
	return m_scene->entities.Wrap(entityId);
}

Audio SystemBase::CreateAudio(const std::string& eventPath)
{
	return m_scene->app->audio.CreateAudio(eventPath);
}

AudioSource SystemBase::GetAudioSource(const std::string& eventPath)
{
	return m_scene->app->audio.GetAudioSource(eventPath);
}

RayQueryResult SystemBase::QueryRay(vec2 pos, vec2 end)
{
	return m_scene->physics.QueryRay(pos, end);
}

RayQueryResult SystemBase::QueryRay(vec2 pos, vec2 direction, float distance)
{
	return m_scene->physics.QueryRay(pos, direction, distance);
}

PointQueryResult SystemBase::QueryPoint(vec2 pos, float radius)
{
	return m_scene->physics.QueryPoint(pos, radius);
}

vec2 SystemBase::GetAxis(const InputName& name)
{
	return m_scene->app->input.GetAxis(name);
}

float SystemBase::GetButton(const InputName& name)
{
	return m_scene->app->input.GetButton(name);
}

bool SystemBase::Once(const InputName& name)
{
	return m_scene->app->input.Once(name);
}

void SystemBase::SetBreak(SystemBreak on)
{
	m_break = on;
}

SystemBreak SystemBase::GetBreak() const
{
	return m_break;
}

SystemState SystemBase::GetState() const
{
	return m_state;
}