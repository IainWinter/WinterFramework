#pragma once

#include "Entity.h"
#include "Common.h"

// This holds the global state for the program
// each module represents a plugin / major piece of the framework

// An example would be

// 1. Window / Rendering context
// 2. Physics World
// 3. Audio Engine
// 4. Sand World
// 5. Sprite Renderer
// 6. Triangle Renderer
// 7. Level Manager

// This allows the user to enable / disable the functionality they need

struct Application
{
private:
	Entity m_modules;
	EntityWorld m_root;

public:
	 Application() { m_modules = m_root.Create(); }
	~Application() { m_modules.Destroy(); }

	template<typename    _t> _t&                    GetModule()                { return m_modules.Get<_t>(); }
	template<typename... _t> std::tuple<_t&...>     GetModules()               { return m_modules.GetAll<_t...>(); }
	template<typename    _t> void                   RemoveModule()             { m_modules.Remove<_t>(); }
	template<typename    _t, typename... _args> _t& AddModule(_args&&... args) { return m_modules.Add<_t>(std::forward<_args>(args)...); }
};

struct Level;

struct System
{
private:
	EntityWorld* m_world;  // get set in Level::NewSystem
	Application* m_owning;
	friend struct Level;

protected:
	template<typename... _t> EntityQuery<_t...>           Query()           { return m_world->Query<_t...>(); }
	template<typename... _t> EntityQueryWithEntity<_t...> QueryWithEntity() { return m_world->QueryWithEntity<_t...>(); }
	template<typename    _t> _t&                          GetModule()       { return m_owning->GetModule<_t>(); }
	template<typename... _t> std::tuple<_t&...>           GetModules()      { return m_owning->GetModules<_t...>(); }

public:
	virtual void Update() {}
	virtual void FixedUpdate() {}
	virtual void UI() {}
};

struct Level
{
private:
	int m_levelId; // not really needed was for global ecs to mark entities
	Application* m_owning;

	EntityWorld m_world;
	std::vector<System*> m_systems; // updater functions

public:
	Level(int levelId, Application* owning)
		: m_levelId (levelId)
		, m_owning  (owning)
	{}
	
	~Level()
	{
		// delete all systems, we made copies so this is always valid
		for (System* system : m_systems) delete system;
	}

	int Id() const
	{
		return m_levelId;
	}

	Application* App() const
	{
		return m_owning;
	}

	Entity CreateEntity()
	{
		return m_world.Create();
	}

	// Systems

	const std::vector<System*>& GetSystems() const
	{
		return m_systems;
	}

	// system constructors should be only default init of values
	// wait until Start to do any work

	template<typename _t>
	Order AddSystem(const _t& system_toCopy)
	{
		return (Order)m_systems.emplace_back(NewSystem(system_toCopy));
	}

	template<typename _t>
	Order AddSystemAfter(Order after, const _t& system_toCopy)
	{
		auto itr = m_systems.begin();
		for (; itr != m_systems.end(); ++itr) if (*itr == after) break;

		return (Order)*m_systems.emplace(itr, NewSystem(system_toCopy));
	}

private:
	template<typename _t>
	System* NewSystem(const _t& system_toCopy)
	{
		System* system = new _t(system_toCopy);
		system->m_world = &m_world;
		system->m_owning = m_owning;
		return system;
	}
};

// manages subletting the ECS for moving between levels
// this will be useful for moving between main menu / game play without needing to delete everything yourself

struct LevelManager
{
private:
	int m_nextLevelId;
	Application* m_owning;

	std::unordered_map<int, r<Level>> m_levels;
	inline static r<Level> m_current;

public:
	LevelManager(Application& owning)
		: m_nextLevelId (1)
		, m_owning      (&owning)
	{}

	// Creating levels
	// this is where they would be loaded form file

	r<Level> CreateLevel()
	{
		r<Level> level = std::make_shared<Level>(m_nextLevelId, m_owning);
		m_levels.emplace(m_nextLevelId, level);
		m_nextLevelId += 1;

		if (!m_current) m_current = level;

		return level;
	}

	static r<Level> CurrentLevel()
	{
		return m_current;
	}
};

inline static r<Level> m_current = {};