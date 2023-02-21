#pragma once

#include "Audio.h"
#include "Rendering.h"

// simple wrapper around steam audio
// forward declare, is this what the _interfaces file is for?

struct _IPLContext_t;
struct _IPLSimulator_t;
struct _IPLScene_t;
struct _IPLStaticMesh_t;
struct _IPLSource_t;

// A source of audio in a 3D scene
class SteamAudioSource
{
public:
	SteamAudioSource(Audio audio, _IPLSource_t* source);

	// Pass the simulation results to FMOD
	void UpdateAudio();

private:
	Audio m_audio;
	_IPLSource_t* m_source;
};

class SteamAudio
{
public:
	SteamAudio(AudioWorld& audio);

	void Init();
	void SetSimulationScene(const Mesh& mesh);

	void RunSimulation();

	SteamAudioSource CreateSource(const std::string& eventName);

private:
	AudioWorld& m_audio;

	_IPLContext_t* m_steam;
	_IPLSimulator_t* m_simulator;
	_IPLScene_t* m_scene;

	std::vector<_IPLStaticMesh_t*> m_meshes;
	//std::vector<SteamAudioSource> m_sources;
};