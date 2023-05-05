#pragma once

#include <vector>

// Keep it simple!
// Just add support for single layer of groups, doesnt have to be an actual tree

class SystemBase;
class SceneUpdate;
class SceneNode;

class UpdateGroup
{
public:
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
};

class SceneUpdate
{
public:
	~SceneUpdate();

	UpdateGroup& CreateGroup(const char* name);

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