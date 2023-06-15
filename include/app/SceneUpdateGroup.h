#pragma once

struct SceneUpdateGroupNode;
struct SceneNode;
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
	SceneUpdateGroup(SceneUpdateGroupNode* node, SceneNode* sceneNode);

public:
	template<typename _t>
	SceneUpdateGroup& Then();

	// Call this if you need to init systems before attaching
	// another group. Not needed most of the time
	SceneUpdateGroup& InitNow();

private:
	void TakeOwnershipOfSystem(SystemBase* system, const char* name);

private:
	friend class Scene;

private:
	SceneUpdateGroupNode* m_node;
	SceneNode* m_sceneNode;
};

template<typename _t>
inline SceneUpdateGroup& SceneUpdateGroup::Then()
{
	TakeOwnershipOfSystem(new _t(), typeid(_t).name());
	return *this;
}