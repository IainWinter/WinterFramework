#pragma once

#include "Audio.h"
#include "Input.h"

#include "app/SceneNode.h"
#include "app/Application.h"

#include "ext/AssetItem.h"

enum SystemBreak
{
	ON_NONE        = 0,
	ON_INIT        = 1 << 0,
	ON_DNIT        = 1 << 1,
	ON_ATTACH      = 1 << 2,
	ON_DETACH      = 1 << 3,
	ON_UPDATE      = 1 << 4,
	ON_FIXEDUPDATE = 1 << 5,
	ON_UI          = 1 << 6,
	ON_DEBUG       = 1 << 7
};

enum SystemState
{
	SYSTEM_CREATED,
	SYSTEM_INIT,
	SYSTEM_ATTACHED,
	SYSTEM_DETACHED,
	SYSTEM_DNIT
};

struct SceneUpdateGroupNode;

class SystemBase
{
public:
	virtual ~SystemBase() {}

protected:

// interface

	virtual void Init() {} // alloc resources
	virtual void Dnit() {} // free resources

	virtual void OnAttach() {} // attach events
	virtual void OnDetach() {} // detach events

	virtual void Update() {}
	virtual void FixedUpdate() {}

	virtual void UI() {}
	virtual void Debug() {}

// creating entities

	//Entity CreateEntity();
	//Entity CreateEntity(const EntityPrefab& prefab);
	//Entity CreateEntity(const a<EntityPrefab>& prefab); // short hand for *asset.ptr()
	//Entity WrapEntity(u32 entityId);

// entity queries

	//template<typename... _t> EntityQuery<_t...>           Query();
	//template<typename... _t> EntityQueryWithEntity<_t...> QueryWithEntity();
	//template<typename    _t> _t&                          First();
	//template<typename... _t> Entity                       FirstEntity();
	//template<typename... _t> int                          GetNumberOf();
	
// sending events

	template<typename _e> void Send  (_e&& event, const char* _fromFile = nullptr, int _fromLine = 0);
	template<typename _e> void SendUp(_e&& event, const char* _fromFile = nullptr, int _fromLine = 0);

// audio

	Audio       CreateAudio   (const std::string& eventPath);
	AudioSource GetAudioSource(const std::string& eventPath);

// physics

	RayQueryResult   QueryRay  (vec2 pos, vec2 end);
	RayQueryResult   QueryRay  (vec2 pos, vec2 direction, float distance);
	PointQueryResult QueryPoint(vec2 pos, float radius);

// input

	vec2  GetAxis  (const InputName& name);
	float GetButton(const InputName& name);
	bool  GetOnce  (const InputName& name);
    
	float GetRawState(InputCode code);

    bool IsUsingController();
    
// getting the raw framework classes
// ideally, events would be sent to a class which had these in scope, but
// there is no way to attach events to a scene, so this needs to be exposed
// but for now, there is no way to pass the entity world to a function

	v2EntitySceneData& _scene();
	template<typename _t> _t& _scene();

	PhysicsWorld& _physics();
	EventQueue& _events();
	InputMap& _input();

	WindowRef _window();

public:

// debug

	void SetBreak(SystemBreak on);

	SystemBreak GetBreak() const;

private:
	void _Init(SceneNode* scene); // pass scene so every system doesn't need a constructor
	void _Dnit();

	void _OnAttach();
	void _OnDetach();

	void _Update();
	void _FixedUpdate();

	void _UI();
	void _Debug();

	void _SetName(const std::string& name);

private:
	// for Attach and Detach to be able to see m_data->bus
	// without giving direct access to children
	template<typename _t> 
	friend struct System;
	
	// for _XXXMethod
	friend struct SceneUpdateGroupNode;

private:
	SceneNode* m_scene = nullptr;
	
	SystemBreak m_break = ON_NONE;
	std::string m_name;
};

template<typename _t>
class System : public SystemBase
{
public:
	virtual ~System() {}

	template<typename _e> void Attach();
	template<typename _e> void Detach();
};

//
// template impl
//

//template<typename... _t> 
//inline EntityQuery<_t...> SystemBase::Query()
//{
//	return m_scene->entities.Query<_t...>();
//}
//
//template<typename... _t> 
//inline EntityQueryWithEntity<_t...> SystemBase::QueryWithEntity()
//{
//	return m_scene->entities.QueryWithEntity<_t...>();
//}
//
//template<typename _t> 
//inline _t& SystemBase::First()
//{
//	return m_scene->entities.First<_t>();
//}
//
//template<typename... _t> 
//inline Entity SystemBase::FirstEntity()
//{
//	return m_scene->entities.FirstEntity<_t...>();
//}
//
//template<typename... _t>
//inline int SystemBase::GetNumberOf()
//{
//	return m_scene->entities.GetNumberOf<_t...>();
//}

template<typename _e>
inline void SystemBase::Send(_e&& event, const char* _fromFile, int _fromLine)
{
	m_scene->event.Send(std::forward<_e>(event), _fromFile, _fromLine);
}

template<typename _e>
inline void SystemBase::SendUp(_e&& event, const char* _fromFile, int _fromLine)
{
	m_scene->app->event.Send(std::forward<_e>(event), _fromFile, _fromLine);
}

template<typename _t> 
inline _t& SystemBase::_scene()
{
	return *(_t*)m_scene->data;
}

template<typename _t>
template<typename _e>
inline void System<_t>::Attach()
{
	m_scene->bus.Attach<_e, _t>((_t*)this);
}

template<typename _t>
template<typename _e>
inline void System<_t>::Detach()
{
	m_scene->bus.Detach<_e>(this);
}

// Trying this, may not work. I want to be able to use the same function name
// might be a tricky word to replace

#ifdef EVENTS_REPORT_FILE
#	define Send(...) Send(__VA_ARGS__, __FILE__, __LINE__);
#	define SendUp(...) SendUp(__VA_ARGS__, __FILE__, __LINE__);
#endif
