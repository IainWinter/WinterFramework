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
class SteamAudioSource : public Audio
{
public:
	SteamAudioSource();
	SteamAudioSource(Audio audio, _IPLSource_t* source);

	_IPLSource_t* GetSource();

private:
	_IPLSource_t* m_source;
};

class SteamAudio
{
public:
	SteamAudio(AudioWorld& audio);

	void Init();

	void Tick_temp();

	void RunSimulation();

	SteamAudioSource CreateSource(const std::string& eventName);
	
	void CreateStaticMesh(const Mesh& mesh);

	void SetListenerPosition(vec3 position);

private:

	AudioWorld& m_audio;

	_IPLContext_t* m_steam;
	_IPLSimulator_t* m_simulator;
	_IPLScene_t* m_scene;

	std::vector<_IPLStaticMesh_t*> m_meshes;
	std::vector<SteamAudioSource> m_simulateDirect;

	vec3 m_listenerPosition;
	_IPLSource_t* m_listenerSource;
};