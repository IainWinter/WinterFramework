#pragma once

#include "app/System.h"

void DrawComponent(const Transform2D& transform)
{
	ImGui::SliderFloat2("Position", (float*)&transform.position, -32, 32);
	ImGui::SliderFloat2("Scale",    (float*)&transform.scale,      0, 20);
	ImGui::SliderFloat ("Rotation", (float*)&transform.rotation, -w2PI, w2PI);
}

struct System_Inspector : SystemBase
{
	void UI() override
	{
		ImGui::Begin("World Inspector");

		for (World* world : GetWorld()->GetApplication()->GetWorlds())
		{
			if (ImGui::CollapsingHeader(world->GetName()))
			{
				DrawWorld(world);
			}
		}

		ImGui::End();


		ImGui::Begin("Entity Inspector");

		if (selected.IsAlive())
		{
			DrawComponent<Transform2D>(selected);
		}

		ImGui::End();
	}

private:
	
	std::unordered_map<Entity, std::vector<Entity>> children;
	Entity selected;

	template<typename _t>
	void DrawComponent(Entity entity)
	{
		if (entity.Has<_t>())
		{
			::DrawComponent((const _t&)entity.Get<_t>());
		}
	}

	void DrawWorld(World* world)
	{
		children = {};

		for (auto [entity, meta] : world->GetEntityWorld().QueryWithEntity<EntityMeta>())
		{
			children[entity.GetParent()].push_back(entity);
		}

		for (Entity& e : children[Entity()]) // loop over entities at root
		{
			DrawEntity(e);
		}
	}

	void DrawEntity(Entity current)
	{
		ImGui::Indent(10.f);

		EntityMeta& meta = current.Get<EntityMeta>();

		std::stringstream ss;
		if (meta.name) ss << meta.name << "######";
		else           ss << current.raw_id();

		auto itr = children.find(current);

		if (itr == children.end())
		{
			if (ImGui::Selectable(ss.str().c_str(), current == selected))
			{
				selected = current;
			}
		}

		else
		{
			if (ImGui::CollapsingHeader(ss.str().c_str()))
			{
				ImGui::Indent(10.f);

				for (Entity& e : itr->second) // recurse
				{
					DrawEntity(e);
				}
			}

		}

		ImGui::Indent(-10.f);
	}
};
