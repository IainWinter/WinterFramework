#pragma once

struct UserSettings
{
	std::string name = "";

	int fullscreenMode = 0;
	int vsyncMode      = false;

	float musicVol  = 1.f;
	float effectVol = 1.f;
};