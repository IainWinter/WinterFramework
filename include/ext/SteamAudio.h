#pragma once

#include "Audio.h"
#include "Rendering.h"

// Could put this in a class called SteamAudioSimulator which is DI in to SteamAudio
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>

// simple wrapper around steam audio
// forward declare, is this what the _interfaces file is for?

struct _IPLContext_t;
struct _IPLSimulator_t;
struct _IPLScene_t;
struct _IPLStaticMesh_t;
struct _IPLSource_t;

class SteamAudio;

// these are all handles

struct SteamAudioSourceSimulationResults
{
	float airAbsorption[3];
	float transmission[3];

	float directivity;
	float distanceAttenuation;
	float occlusion;

	int directFlags;
	int transmissionType;
};

// A source of audio in a 3D scene
class SteamAudioSource : public Audio
{
public:
	SteamAudioSource();
	SteamAudioSource(Audio audio, _IPLSource_t* source, SteamAudio* world);

	void Destroy() override;

	const SteamAudioSourceSimulationResults& GetSimulationResults() const;

	// internal
	void _UpdateSourceInputs();
	void _UpdateDSPParams();
	bool _IsSource(_IPLSource_t* source) const;

private:
	_IPLSource_t* m_source;
	SteamAudio* m_world;

	// for debug
	r<SteamAudioSourceSimulationResults> m_outputs;
};

class SteamAudioGeometry
{
public:
	SteamAudioGeometry();
	SteamAudioGeometry(_IPLStaticMesh_t* mesh, SteamAudio* world);

	void Destroy();

private:
	_IPLStaticMesh_t* m_mesh;
	SteamAudio* m_world;
};

//class SteamAudioSimulator
//{
//public:
//	virtual void UpdateData();
//	virtual void GetData();
//
//	virtual void Run();
//	virtual void Stop();
//};

class SteamAudio
{
public:
	SteamAudio(AudioWorld& audio);

	void Init();
	void Dnit();
	void RunSimulation();

	SteamAudioSource CreateSource(const std::string& eventName);
	SteamAudioGeometry CreateStaticMesh(const Mesh& mesh);

	void SetListenerPosition(vec3 position);

	void SaveObjFile();

	// internal
	void _RemoveSource(_IPLSource_t* source);
	void _RemoveGeometry(_IPLStaticMesh_t* geometry);

private:
	void WriteSimulationInputs();
	void ReadSimulationResults();

private:
	AudioWorld& m_audio;

	_IPLContext_t* m_steam;
	_IPLSimulator_t* m_simulator;
	_IPLScene_t* m_scene;

	std::vector<SteamAudioSource> m_simulateDirect;

	// This is for reverb calculations
	vec3 m_listenerPosition;
	_IPLSource_t* m_listener;

	// todo:
	//
	// Running the simulation in a separate thread
	// Could put this in a class called SteamAudioSimulator which is DI in to SteamAudio
	//

	// This is so I can run the simulation in another thread
	std::thread m_simulationThread;

	std::atomic<bool> m_simInputsReady;
	std::atomic<bool> m_simResultsReady;
	std::atomic<bool> m_simRunning;
	std::mutex m_simMut;
	std::condition_variable m_simWait;
};