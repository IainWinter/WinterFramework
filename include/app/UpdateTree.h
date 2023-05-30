#pragma once

#include <vector>
#include <string>

class SystemBase;
class SceneUpdate;
class SceneNode;

class UpdateGroup
{
public:
	UpdateGroup(const std::string& name);
	~UpdateGroup();

	template<typename _t>
	UpdateGroup& Then();

private:
	void AddUpdate(SystemBase* update);

private:
	// So user cannot see updates, but SceneUpdate can iterate them
	friend class SceneUpdate;

private:
	std::vector<SystemBase*> m_updates;
	std::string m_name;
};

class SceneUpdate
{
public:
	~SceneUpdate();

	UpdateGroup& CreateGroup(const std::string& name);
	void DestroyGroup(const std::string& name);

	std::vector<SystemBase*> GetUpdateOrder(); // this is bad should return an iterator

private:
	std::vector<UpdateGroup*> m_groups;
};

//
// Template impl
//

template<typename _t>
inline UpdateGroup& UpdateGroup::Then()
{
	AddUpdate(new _t());
	return *this;
}