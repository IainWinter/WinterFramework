#pragma once

#include "app/System.h"
#include "Physics.h"
#include "Rendering.h"
#include "ext/Time.h"
#include "Components/KeepOnScreen.h"

struct System_KeepOnScreen : SystemBase
{
	vec4 screen; // (min x/y, max x/y)

	void Init()
	{
		vec2 camera = First<Camera>().ScreenSize();

		// needs to be adjusted for UI

		screen.x = camera.x;// *.7; // .15 * 2 for the UI size in PlayerHUD.h
		screen.y = camera.y;
		screen.z = camera.x;
		screen.w = camera.y;
	}

	void FixedUpdate()
	{
		for (auto [body, keepOnScreen] : Query<Rigidbody2D, KeepOnScreen>())
		{
			vec2 m = keepOnScreen.margin;
			vec2 pos = body.GetPosition();
			vec2 vel = body.GetVelocity();

			if (pos.x + vel.x * Time::DeltaTime() < -screen.x + m.x) { pos.x = -screen.x + m.x; vel.x = 0.f; }
			if (pos.x + vel.x * Time::DeltaTime() >  screen.z - m.x) { pos.x =  screen.z - m.x; vel.x = 0.f; }
			if (pos.y + vel.y * Time::DeltaTime() < -screen.y + m.y) { pos.y = -screen.y + m.y; vel.y = 0.f; }
			if (pos.y + vel.y * Time::DeltaTime() >  screen.w - m.y) { pos.y =  screen.w - m.y; vel.y = 0.f; }
			
			if (body.GetPosition() != pos) body.SetPosition(pos);
			if (body.GetVelocity() != vel) body.SetVelocity(vel);
		}

		for (auto [transform, body, wrapOnScreen] : Query<Transform2D, Rigidbody2D, WrapOnScreen>())
		{
			vec2 m = wrapOnScreen.margin;
			vec2 pos = transform.position;
		
			if (pos.x < -screen.x - m.x) { pos.x =  screen.x; }
			if (pos.x >  screen.z + m.x) { pos.x = -screen.z; }
			if (pos.y < -screen.y - m.y) { pos.y =  screen.y; }
			if (pos.y >  screen.w + m.y) { pos.y = -screen.w; }

			if (pos != transform.position)
			{
				transform.position = pos;
				transform.UpdateLastFrameData();
				body.SetTransform(transform);
			}
		}
	}
};