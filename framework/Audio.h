#pragma once

#include "Defines.h"
#include "Log.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdarg.h>

#undef PlaySound
#undef CreateEvent

// all calls return a handle or an error code
// all error codes are negitive
// 0 is a special handle representing global actions

enum audio_error : int
{
	ENGINE_OK               =  0,
	ENGINE_FAILED_CREATE    = -1,
	ENGINE_FAILED_INIT      = -2,
	ENGINE_FAILED_DNIT      = -3,
	ENGINE_FAILED_UPDATE    = -4,
	ENGINE_FAILED_LOAD_BANK = -5,
	ENGINE_ALREADY_LOADED   = -6,

	ENGINE_FAILED_INVALID_HANDLE    = -7,
	ENGINE_FAILED_HANDLE_NOT_LOADED = -8,

	ENGINE_FAILED_MAKE_INSTANCE  = -9,
	ENGINE_FAILED_LOAD_INSTANCE  = -10,
	ENGINE_FAILED_START_INSTANCE = -11,
	ENGINE_FAILED_STOP_INSTANCE  = -12,
	
	ENGINE_FAILED_FREE_INSTANCE  = -13,
	ENGINE_FAILED_FREE_BANK  = -14,
	
	ENGINE_FAILED_SET_PARAM = -15,
	ENGINE_FAILED_GET_PARAM = -16,
};

const char* audio_error_string(audio_error code);
bool audio_error_check(int result);

int audio_set_type(int high, int low);
int audio_get_type(int handle);
int audio_make_handle(int type, int index);

// forward declare fmod

namespace FMOD { 
namespace Studio 
{
	class System;
	class Bank;
	class EventDescription;
	class EventInstance;
	class Bus;
	class VCA;
}}

struct Audio
{
	std::unordered_map<std::string, int> m_loaded; // name, handle - hashed paths of loaded objects

	Audio();
	~Audio();

	int Tick();

	int Load(const std::string& path);
	int Play(const std::string& path);

	int Set(int handle, const std::string& parameter, float  value);
	int Get(int handle, const std::string& parameter, float& value);

	int Start(int handle);
	int Stop (int handle);
	int Free (int handle);

	int SetVolume(int handle, float  volume);
	int GetVolume(int handle, float& volume);

	bool IsLoaded(int handle) const;
	bool IsLoaded(const std::string& path) const;

	int GetHandle(const std::string& path) const;

private:
	int MakeHandle(int type) const;
	void PutLoaded(const std::string& path, int handle);

// FMOD stuff

private:
	FMOD::Studio::System* m_system;

	std::unordered_map<int, FMOD::Studio::Bank*>             m_banks;
	std::unordered_map<int, FMOD::Studio::Bus*>              m_buses;
	std::unordered_map<int, FMOD::Studio::VCA*>              m_vcas;
	std::unordered_map<int, FMOD::Studio::EventDescription*> m_events;
	std::unordered_map<int, FMOD::Studio::EventInstance*>    m_instances;

	using InstanceUserData = std::pair<int, Audio*>;

	int PutLoaded(const std::string& path, FMOD::Studio::Bank* bank);
	int PutLoaded(const std::string& path, FMOD::Studio::EventDescription* event);
	int PutLoaded(const std::string& path, FMOD::Studio::EventInstance* instance);
	int PutLoaded(const std::string& path, FMOD::Studio::Bus* bus);
	int PutLoaded(const std::string& path, FMOD::Studio::VCA* vca);

	template<typename _t, typename _f1, typename _f2>
	int LoadFromBank(FMOD::Studio::Bank* bank, _f1&& getCount, _f2&& getItems)
	{
		int count;

		if (audio_error_check(getCount(bank, count)))
		{
			return ENGINE_FAILED_LOAD_BANK;
		}

		if (count > 0)
		{
			_t** items = new _t*[count];

			if (audio_error_check(getItems(bank, items, count)))
			{
				delete[] items;
				return ENGINE_FAILED_LOAD_BANK;
			}

			for (int i = 0; i < count; i++)
			{
				std::string name(1024, '\0');
				int size;

				if (audio_error_check(items[i]->getPath(name.data(), 1024, &size)))
				{
					delete[] items;
					return ENGINE_FAILED_LOAD_BANK;
				}

				name.resize(size - 1);
				PutLoaded(name, items[i]);
			}

			delete[] items;
		}

		return ENGINE_OK;
	}
};

// Components to talk to Audio through ECS
// and update mixing

//struct AudioEmitter
//{
//private:
//	Audio* m_audio;
//	int instance = 0; // gets populated when playing
//
//public:
//	std::string event;
//
//public:
//	void _SetAudio(Audio* audio);
//
//	AudioEmitter& SetEvent(const std::string& event);
//
//	bool IsPlaying()   const;
//	int  GetInstance() const;
//
//	int PlayAndForget();
//
//	AudioEmitter& Play();
//	AudioEmitter& Start();
//	AudioEmitter& Stop();
//	AudioEmitter& Free();
//
//	AudioEmitter&  Set(const std::string& param, float value);
//	float          Get(const std::string& param);
//
//	AudioEmitter& SetVolume(float volume);
//	float         GetVolume();
//};