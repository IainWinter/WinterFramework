#pragma once

struct SceneUpdateGroupNode;
class Scene;
class SystemBase;

// A SceneUpdateGroup holds a list of systems which get
// executed in order of insertion. Systems cannot be removed
// individually, so should be put in their own group if
// that is needed.
class SceneUpdateGroup
{
public:
	SceneUpdateGroup();
	SceneUpdateGroup(SceneUpdateGroupNode* node);

public:
	template<typename _t>
	SceneUpdateGroup& Then();
	
private:
	void TakeOwnershipOfSystem(SystemBase* system);

private:
	friend class Scene;

private:
	SceneUpdateGroupNode* m_node;
};

template<typename _t>
inline SceneUpdateGroup& SceneUpdateGroup::Then()
{
	_t* t = new _t();
	TakeOwnershipOfSystem(t);
	return *this;
}