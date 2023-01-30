#pragma once

#include "Common.h"
#include "Entity.h"

inline Transform2D WorldTransform(const Entity& e, const Transform2D& transform)
{
	// end recursion on no transform
	// this effectivly says that even if you are parented,
	// the parenting for position breaks at the first without a
	// transform component
	
	Entity parent = e.Get<EntityMeta>().parent;
	
	if (!parent.IsAlive() || !parent.Has<Transform2D>())
	{
		return transform;
	}

	else
	{
		return WorldTransform(parent, parent.Get<Transform2D>()) * transform;
	}

	//if (!e.IsAlive() || !e.Has<Transform2D>())
	//{
	//	return Transform2D();
	//}

	//Transform2D me = e.Get<Transform2D>();
	//Entity parent = e.Get<EntityMeta>().parent;

	//return WorldTransform(parent) * me;
}