#pragma once

#include "Leveling.h"
#include "Physics.h"
#include "Rendering.h"
#include "ext/Time.h"
#include "Components/KeepOnScreen.h"

struct System_KeepOnScreen : SystemBase
{
	vec2 screen;

	void Init()
	{
		screen = GetModule<Camera>().ScreenSize();
	}

	void FixedUpdate()
	{
		for (auto [body, keepOnScreen] : Query<Rigidbody2D, KeepOnScreen>())
		{
			vec2 m = keepOnScreen.margin;
			vec2 pos = body.GetPosition();
			vec2 vel = body.GetVelocity();

			if (pos.x + vel.x * Time::DeltaTime() < -screen.x + m.x) { pos.x = -screen.x + m.x; vel.x = 0.f; }
			if (pos.x + vel.x * Time::DeltaTime() >  screen.x - m.x) { pos.x =  screen.x - m.x; vel.x = 0.f; }
			if (pos.y + vel.y * Time::DeltaTime() < -screen.y + m.y) { pos.y = -screen.y + m.y; vel.y = 0.f; }
			if (pos.y + vel.y * Time::DeltaTime() >  screen.y - m.y) { pos.y =  screen.y - m.y; vel.y = 0.f; }
			
			if (body.GetPosition() != pos) body.SetPosition(pos);
			if (body.GetVelocity() != vel) body.SetVelocity(vel);
		}
	}
};