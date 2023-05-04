#pragma once

#include <vector>

class SystemBase;
class UpdateTree;

class UpdateTreeNode
{
public:
	~UpdateTreeNode();

	template<typename _t>
	UpdateTreeNode& Then();

	template<typename _callable>
	void Walk(_callable&& callable);

private:
	UpdateTreeNode& Then(SystemBase* system);

private:
	SystemBase* m_update = nullptr;
	std::vector<UpdateTreeNode*> m_children;
};

class UpdateTree
{
public:
	UpdateTreeNode& CreateGroup(const char* name);

	std::vector<SystemBase*> GetOrderedList();
	
	template<typename _callable>
	void Walk(_callable&& callable);

private:
	std::vector<UpdateTreeNode*> m_roots;
};

//
// Template impl
//

template<typename _t>
inline UpdateTreeNode& UpdateTreeNode::Then()
{
	return Then(new _t());
}

template<typename _callable>
void UpdateTreeNode::Walk(_callable&& callable)
{
	callable(m_update);
	for (UpdateTreeNode* node : m_children)
		node->Walk(callable);
}

template<typename _callable>
void UpdateTree::Walk(_callable&& callable)
{
	for (UpdateTreeNode* node : m_roots)
		node->Walk(callable);
}