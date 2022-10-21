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
	class EventDescription;
	class EventInstance;
}}

struct AudioVCA;
struct AudioSource;
struct Audio;
struct AudioWorld;

struct AudioWorld
{
private:
	FMOD::Studio::System* m_system;
	std::unordered_map<std::string, FMOD::Studio::Bank*> m_bank;
	
	std::unordered_map<std::string, AudioVCA>    m_vca;
	std::unordered_map<std::string, AudioSource> m_desc;

public:
	void Init();
	void Dnit();
	void Tick();

	void LoadBank     (const std::string& bankFilePath);
	void FreeBank     (const std::string& bankName);
	bool IsBankLoaded (const std::string& bankName);

	AudioVCA GetVCA(const std::string& vcaName);

	AudioSource GetAudioSource(const std::string& eventName);

	// create a paused audio instance
	Audio CreateAudio(const std::string& eventName);

private:
	std::vector<FMOD::Studio::VCA*>              GetVCAs             (FMOD::Studio::Bank* bank);
	std::vector<FMOD::Studio::EventDescription*> GetEventDescriptions(FMOD::Studio::Bank* bank);
};

// the most basic amp
struct AudioVCA
{
private:
	FMOD::Studio::VCA* m_vca;

public:
	AudioVCA();
	AudioVCA(FMOD::Studio::VCA* vca);

	bool IsAlive() const;

	AudioVCA& SetVolume(float volume);
	float     GetVolume() const;
};

// an event description used to spawn new instances
struct AudioSource
{
private:
	FMOD::Studio::EventDescription* m_desc;
	r<std::vector<FMOD::Studio::EventInstance*>> m_insts;

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
};

// a single instance of audio playing in the world
struct Audio
{
private:
	FMOD::Studio::EventInstance* m_inst;
	AudioSource m_parent;

public:
	Audio();
	Audio(AudioSource& parent);
	Audio(FMOD::Studio::EventInstance* inst, AudioSource& parent);

	// return the parent AudioSource, doesnt necessarily exist
	AudioSource GetParent() const;

	// start or restart (from the beginning) the instance
	Audio& Start();

	// stop and reset params, will fade
	Audio& Stop();

	// stops, then frees the audio instance
	void Destroy();

	bool IsAlive() const;
	bool IsPlaying() const;
	bool HasParent() const;

	// immedietly pauses audio instance
	Audio& Pause();

	// resume playback from when audio instance was paused
	Audio& Resume();

	// pause or resume audio instance based on IsPlaying
	Audio& TogglePause();

	Audio& SetVolume(float volume);
	float  GetVolume()  const;

	Audio& SetParam(const std::string& paramName, float volume);
	float  GetParam(const std::string& paramName) const;

	Audio& SetTimeline(float milliseconds);
	float  GetTimeline();

	bool operator==(const Audio& other) const;
	bool operator!=(const Audio& other) const;
};