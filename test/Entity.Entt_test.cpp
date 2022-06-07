#include "Entity.Entt.h"

struct Transform2D
{
	float x, y, z;
};

struct PlayerController : System
{
	void Update()
	{
		for (auto [entity, transform] : QueryWithEntity<Transform2D>())
		{
			printf("%d\n", entity.Id());
		}
	}
};

int main()
{
	EntityWorld world;
	world.Create().Add<Transform2D>();
	world.Create().Add<Transform2D>();
	world.Create().Add<Transform2D>();
	world.Create().Add<Transform2D>();
	world.Create().Add<Transform2D>();
	world.Create().Add<Transform2D>();

	world.AddSystem(PlayerController());

	world.TickSystems();
}

