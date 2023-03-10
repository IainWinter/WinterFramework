#pragma once

#include "Log.h"
#include "Common.h"

#include <string>
#include <vector>
#include <unordered_map>

// simple wrapper around fmod. missing many features, but I am exposing only as I need
// forward declare

namespace FMOD {
namespace Studio 
{
	class System;
	class Bank;
	class VCA;
	class Bus;
	class EventDescription;
	class EventInstance;
}}

struct FMOD_3D_ATTRIBUTES;

class AudioVCA;
class AudioSource;
class Audio;
class AudioWorld;

struct AudioProps3D
{
	vec3 position = vec3(0, 0, 0);
	vec3 velocity = vec3(0, 0, 0);
	vec3 forward  = vec3(0, 0, 1);
	vec3 up       = vec3(0, 1, 0);
	
	FMOD_3D_ATTRIBUTES ToFMOD() const;
	void FromFMOD(const FMOD_3D_ATTRIBUTES& fmod);
};

// the most basic amp
class AudioVCA
{
public:
	AudioVCA();
	AudioVCA(FMOD::Studio::VCA* vca);

	bool IsAlive() const;

	AudioVCA& SetVolume(float volume);
	float     GetVolume() const;

private:
	FMOD::Studio::VCA* m_vca;
};

// an event description used to spawn new instances
class AudioSource
{
public:
	AudioSource();
	AudioSource(FMOD::Studio::EventDescription* desc);

	// create a new instance
	Audio Spawn();

	// return the last spawned instance
	// if there are no instances, returns a null audio
	Audio LastInstance();

	// stop all instances
	void StopAll();

	bool IsAlive() const;
	int NumberOfInstances() const;

	// internal
	void _RemoveInstance(FMOD::Studio::EventInstance* inst);

private:
	FMOD::Studio::EventDescription* m_desc;
	r<std::vector<FMOD::Studio::EventInstance*>> m_insts;
};

// a single instance of audio playing in the world
class Audio
{
public:
	Audio();
	Audio(AudioSource& parent);
	Audio(FMOD::Studio::EventInstance* inst, AudioSource& parent);

	// return the parent AudioSource, doesn't necessarily exist
	AudioSource GetParent() const;

	// start or restart (from the beginning) the instance
	Audio& Start();

	// stop and reset params, will fade
	Audio& Stop();

	// stops, then frees the audio instance
	virtual void Destroy();

	bool IsAlive() const;
	bool IsPlaying() const;
	bool HasParent() const;

	// immediately pauses audio instance
	Audio& Pause();

	// resume playback from when audio instance was paused
	Audio& Resume();

	// pause or resume audio instance based on IsPlaying
	Audio& TogglePause();

	Audio& SetVolume(float volume);
	float  GetVolume() const;

	Audio& SetParam(const std::string& paramName, float volume);
	float  GetParam(const std::string& paramName) const;

	Audio& SetParamDSP(int dspIndex, int paramIndex, float value);
	Audio& SetParamDSP(int dspIndex, int paramIndex, bool value);
	Audio& SetParamDSP(int dspIndex, int paramIndex, int value);
	Audio& SetParamDSP(int dspIndex, int paramIndex, void* data, int size);

	Audio& GetParamDSP(int dspIndex, int paramIndex, void** ptr);
	Audio& GetParamDSP(int dspIndex, int paramIndex, int* ptr);

	Audio&       SetProps3D(const AudioProps3D& props);
	AudioProps3D GetProps3D() const;

	Audio& SetTimeline(float milliseconds);
	float  GetTimeline();

	void ListAllDspParamNames();

	bool operator==(const Audio& other) const;
	bool operator!=(const Audio& other) const;

private:
	FMOD::Studio::EventInstance* m_inst;
	AudioSource m_parent;
};

class AudioWorld
{
public:
	AudioWorld();

	void Init();
	void Dnit();
	void Tick();

	void LoadBank     (const std::string& bankFilePath);
	void FreeBank     (const std::string& bankName);
	bool IsBankLoaded (const std::string& bankName);

	bool LoadPlugin(const std::string& filepath, unsigned int* handle = nullptr);

	AudioVCA GetVCA(const std::string& vcaName);

	AudioSource GetAudioSource(const std::string& eventName);

	// create a paused audio instance
	Audio CreateAudio(const std::string& eventName);

	void SetListenerProps3D(const AudioProps3D& props);
	AudioProps3D GetListenerProps3D() const;

private:
	std::vector<FMOD::Studio::VCA*>              GetVCAs             (const FMOD::Studio::Bank* bank) const;
	std::vector<FMOD::Studio::EventDescription*> GetEventDescriptions(const FMOD::Studio::Bank* bank) const;
	std::vector<FMOD::Studio::Bus*>              GetBuses            (const FMOD::Studio::Bank* bank) const;

private:
	FMOD::Studio::System* m_system;
	std::unordered_map<std::string, FMOD::Studio::Bank*> m_bank;

	std::unordered_map<std::string, AudioVCA>    m_vca;
	std::unordered_map<std::string, AudioSource> m_desc;
};