#include "Audio.h"

#include "fmod/fmod.hpp"
#include "fmod/fmod_studio.hpp"

#include <time.h> // for seed

#define CHECK_HANDLE(h) if (!IsLoaded(h)) { return audio_log_error(ENGINE_FAILED_HANDLE_NOT_LOADED); }
#define CHECK(stmt, n) if (audio_error_check(stmt)) { return audio_log_error(n); }
#define CHECK_NO_RETURN(stmt, n) if (audio_error_check(stmt)) { audio_log_error(n); }

#define H_BANK 1
#define H_EVENT 2
#define H_INSTANCE 3
#define H_BUS 4
#define H_VCA 5

using namespace FMOD::Studio;

std::unordered_map<int, const char*> audio_error_translation =
{
	{ ENGINE_OK,               "Engine is ok" },
	{ ENGINE_FAILED_CREATE,    "Engine failed to create fmod system" },
	{ ENGINE_FAILED_INIT,      "Engine failed to init fmod system" },
	{ ENGINE_FAILED_UPDATE,    "Engine failed to update fmod system" },
	{ ENGINE_FAILED_LOAD_BANK, "Engine failed to load a bank" },
	{ ENGINE_ALREADY_LOADED,   "Engine tried to load a bank that was already loaded" },

	{ ENGINE_FAILED_MAKE_INSTANCE,     "Engine failed to create an instance" },
	{ ENGINE_FAILED_LOAD_INSTANCE,     "Engine failed to load an instance" },
	{ ENGINE_FAILED_INVALID_HANDLE,    "Engine was passed an invalid handle" },
	{ ENGINE_FAILED_HANDLE_NOT_LOADED, "Engine was passed a handle that hasn't been loaded" },
	{ ENGINE_FAILED_SET_PARAM,         "Engine failed to set an instance parameter" },
	{ ENGINE_FAILED_GET_PARAM,         "Engine failed to get an instance parameter" }
};

std::unordered_map<int, const char*> fmod_error_translation =
{
	{ FMOD_OK,                            "FMOD_OK" },
	{ FMOD_ERR_BADCOMMAND,                "FMOD_ERR_BADCOMMAND" },
	{ FMOD_ERR_CHANNEL_ALLOC,             "FMOD_ERR_CHANNEL_ALLOC" },
	{ FMOD_ERR_CHANNEL_STOLEN,            "FMOD_ERR_CHANNEL_STOLEN" },
	{ FMOD_ERR_DMA,                       "FMOD_ERR_DMA" },
	{ FMOD_ERR_DSP_CONNECTION,            "FMOD_ERR_DSP_CONNECTION" },
	{ FMOD_ERR_DSP_DONTPROCESS,           "FMOD_ERR_DSP_DONTPROCESS" },
	{ FMOD_ERR_DSP_FORMAT,                "FMOD_ERR_DSP_FORMAT" },
	{ FMOD_ERR_DSP_INUSE,                 "FMOD_ERR_DSP_INUSE" },
	{ FMOD_ERR_DSP_NOTFOUND,              "FMOD_ERR_DSP_NOTFOUND" },
	{ FMOD_ERR_DSP_RESERVED,              "FMOD_ERR_DSP_RESERVED" },
	{ FMOD_ERR_DSP_SILENCE,               "FMOD_ERR_DSP_SILENCE" },
	{ FMOD_ERR_DSP_TYPE,                  "FMOD_ERR_DSP_TYPE" },
	{ FMOD_ERR_FILE_BAD,                  "FMOD_ERR_FILE_BAD" },
	{ FMOD_ERR_FILE_COULDNOTSEEK,         "FMOD_ERR_FILE_COULDNOTSEEK" },
	{ FMOD_ERR_FILE_DISKEJECTED,          "FMOD_ERR_FILE_DISKEJECTED" },
	{ FMOD_ERR_FILE_EOF,                  "FMOD_ERR_FILE_EOF" },
	{ FMOD_ERR_FILE_ENDOFDATA,            "FMOD_ERR_FILE_ENDOFDATA" },
	{ FMOD_ERR_FILE_NOTFOUND,             "FMOD_ERR_FILE_NOTFOUND" },
	{ FMOD_ERR_FORMAT,                    "FMOD_ERR_FORMAT" },
	{ FMOD_ERR_HEADER_MISMATCH,           "FMOD_ERR_HEADER_MISMATCH" },
	{ FMOD_ERR_HTTP,                      "FMOD_ERR_HTTP" },
	{ FMOD_ERR_HTTP_ACCESS,               "FMOD_ERR_HTTP_ACCESS" },
	{ FMOD_ERR_HTTP_PROXY_AUTH,           "FMOD_ERR_HTTP_PROXY_AUTH" },
	{ FMOD_ERR_HTTP_SERVER_ERROR,         "FMOD_ERR_HTTP_SERVER_ERROR" },
	{ FMOD_ERR_HTTP_TIMEOUT,              "FMOD_ERR_HTTP_TIMEOUT" },
	{ FMOD_ERR_INITIALIZATION,            "FMOD_ERR_INITIALIZATION" },
	{ FMOD_ERR_INITIALIZED,               "FMOD_ERR_INITIALIZED" },
	{ FMOD_ERR_INTERNAL,                  "FMOD_ERR_INTERNAL" },
	{ FMOD_ERR_INVALID_FLOAT,             "FMOD_ERR_INVALID_FLOAT" },
	{ FMOD_ERR_INVALID_HANDLE,            "FMOD_ERR_INVALID_HANDLE" },
	{ FMOD_ERR_INVALID_PARAM,             "FMOD_ERR_INVALID_PARAM" },
	{ FMOD_ERR_INVALID_POSITION,          "FMOD_ERR_INVALID_POSITION" },
	{ FMOD_ERR_INVALID_SPEAKER,           "FMOD_ERR_INVALID_SPEAKER" },
	{ FMOD_ERR_INVALID_SYNCPOINT,         "FMOD_ERR_INVALID_SYNCPOINT" },
	{ FMOD_ERR_INVALID_THREAD,            "FMOD_ERR_INVALID_THREAD" },
	{ FMOD_ERR_INVALID_VECTOR,            "FMOD_ERR_INVALID_VECTOR" },
	{ FMOD_ERR_MAXAUDIBLE,                "FMOD_ERR_MAXAUDIBLE" },
	{ FMOD_ERR_MEMORY,                    "FMOD_ERR_MEMORY" },
	{ FMOD_ERR_MEMORY_CANTPOINT,          "FMOD_ERR_MEMORY_CANTPOINT" },
	{ FMOD_ERR_NEEDS3D,                   "FMOD_ERR_NEEDS3D" },
	{ FMOD_ERR_NEEDSHARDWARE,             "FMOD_ERR_NEEDSHARDWARE" },
	{ FMOD_ERR_NET_CONNECT,               "FMOD_ERR_NET_CONNECT" },
	{ FMOD_ERR_NET_SOCKET_ERROR,          "FMOD_ERR_NET_SOCKET_ERROR" },
	{ FMOD_ERR_NET_URL,                   "FMOD_ERR_NET_URL" },
	{ FMOD_ERR_NET_WOULD_BLOCK,           "FMOD_ERR_NET_WOULD_BLOCK" },
	{ FMOD_ERR_NOTREADY,                  "FMOD_ERR_NOTREADY" },
	{ FMOD_ERR_OUTPUT_ALLOCATED,          "FMOD_ERR_OUTPUT_ALLOCATED" },
	{ FMOD_ERR_OUTPUT_CREATEBUFFER,       "FMOD_ERR_OUTPUT_CREATEBUFFER" },
	{ FMOD_ERR_OUTPUT_DRIVERCALL,         "FMOD_ERR_OUTPUT_DRIVERCALL" },
	{ FMOD_ERR_OUTPUT_FORMAT,             "FMOD_ERR_OUTPUT_FORMAT" },
	{ FMOD_ERR_OUTPUT_INIT,               "FMOD_ERR_OUTPUT_INIT" },
	{ FMOD_ERR_OUTPUT_NODRIVERS,          "FMOD_ERR_OUTPUT_NODRIVERS" },
	{ FMOD_ERR_PLUGIN,                    "FMOD_ERR_PLUGIN" },
	{ FMOD_ERR_PLUGIN_MISSING,            "FMOD_ERR_PLUGIN_MISSING" },
	{ FMOD_ERR_PLUGIN_RESOURCE,           "FMOD_ERR_PLUGIN_RESOURCE" },
	{ FMOD_ERR_PLUGIN_VERSION,            "FMOD_ERR_PLUGIN_VERSION" },
	{ FMOD_ERR_RECORD,                    "FMOD_ERR_RECORD" },
	{ FMOD_ERR_REVERB_CHANNELGROUP,       "FMOD_ERR_REVERB_CHANNELGROUP" },
	{ FMOD_ERR_REVERB_INSTANCE,           "FMOD_ERR_REVERB_INSTANCE" },
	{ FMOD_ERR_SUBSOUNDS,                 "FMOD_ERR_SUBSOUNDS" },
	{ FMOD_ERR_SUBSOUND_ALLOCATED,        "FMOD_ERR_SUBSOUND_ALLOCATED" },
	{ FMOD_ERR_SUBSOUND_CANTMOVE,         "FMOD_ERR_SUBSOUND_CANTMOVE" },
	{ FMOD_ERR_TAGNOTFOUND,               "FMOD_ERR_TAGNOTFOUND" },
	{ FMOD_ERR_TOOMANYCHANNELS,           "FMOD_ERR_TOOMANYCHANNELS" },
	{ FMOD_ERR_TRUNCATED,                 "FMOD_ERR_TRUNCATED" },
	{ FMOD_ERR_UNIMPLEMENTED,             "FMOD_ERR_UNIMPLEMENTED" },
	{ FMOD_ERR_UNINITIALIZED,             "FMOD_ERR_UNINITIALIZED" },
	{ FMOD_ERR_UNSUPPORTED,               "FMOD_ERR_UNSUPPORTED" },
	{ FMOD_ERR_VERSION,                   "FMOD_ERR_VERSION" },
	{ FMOD_ERR_EVENT_ALREADY_LOADED,      "FMOD_ERR_EVENT_ALREADY_LOADED" },
	{ FMOD_ERR_EVENT_LIVEUPDATE_BUSY,     "FMOD_ERR_EVENT_LIVEUPDATE_BUSY" },
	{ FMOD_ERR_EVENT_LIVEUPDATE_MISMATCH, "FMOD_ERR_EVENT_LIVEUPDATE_MISMATCH" },
	{ FMOD_ERR_EVENT_LIVEUPDATE_TIMEOUT,  "FMOD_ERR_EVENT_LIVEUPDATE_TIMEOUT" },
	{ FMOD_ERR_EVENT_NOTFOUND,            "FMOD_ERR_EVENT_NOTFOUND" },
	{ FMOD_ERR_STUDIO_UNINITIALIZED,      "FMOD_ERR_STUDIO_UNINITIALIZED" },
	{ FMOD_ERR_STUDIO_NOT_LOADED,         "FMOD_ERR_STUDIO_NOT_LOADED" },
	{ FMOD_ERR_INVALID_STRING,            "FMOD_ERR_INVALID_STRING" },
	{ FMOD_ERR_ALREADY_LOCKED,            "FMOD_ERR_ALREADY_LOCKED" },
	{ FMOD_ERR_NOT_LOCKED,                "FMOD_ERR_NOT_LOCKED" },
	{ FMOD_ERR_RECORD_DISCONNECTED,       "FMOD_ERR_RECORD_DISCONNECTED" },
	{ FMOD_ERR_TOOMANYSAMPLES,            "FMOD_ERR_TOOMANYSAMPLES" }
};

const char* audio_error_string(audio_error code)
{
	return audio_error_translation.at(code);
}

void audio_log(const char* fmt, ...)
{
	printf("[Audio] ");

	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	printf("\n");
}

audio_error audio_log_error(audio_error error)
{
	audio_log("%s", audio_error_string(error));
	return error;
}

bool audio_error_check(int result)
{
	bool hasError = result != FMOD_OK;
	if (hasError)
	{
		audio_log("[FMOD] (%d) %s", result, fmod_error_translation.at(result));
	}

	return hasError;
}

int audio_set_type(int high, int low) { return low + (high << sizeof(int) * 8 / 2); }
int audio_get_type(int handle) { return handle >> (sizeof(int) * 8 / 2); }
int audio_make_handle(int type, int index) { return audio_set_type(type, index); }

Audio::Audio()
{
	audio_log("Initializing audio space using FMOD");

	CHECK_NO_RETURN(
		System::create(&m_system),
		ENGINE_FAILED_CREATE
	);

	FMOD_ADVANCEDSETTINGS settings;
	settings.randomSeed = (unsigned int)time(nullptr);

	FMOD::System* core;
	m_system->getCoreSystem(&core);
	core->setAdvancedSettings(&settings);

	CHECK_NO_RETURN(
		m_system->initialize(16, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr),
		ENGINE_FAILED_INIT
	);
}

Audio::~Audio()
{
	CHECK_NO_RETURN(m_system->release(), ENGINE_FAILED_DNIT);
}

int Audio::Tick()
{
	CHECK(m_system->update(), ENGINE_FAILED_UPDATE);
	return ENGINE_OK;
}

// only loads bank?

int Audio::Load(const std::string& path)
{
	if (IsLoaded(path))
	{
		return audio_log_error(ENGINE_ALREADY_LOADED);
	}

	FMOD::Studio::Bank* bank;

	CHECK(
		m_system->loadBankFile(_a(path).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank),
		ENGINE_FAILED_LOAD_BANK
	);

	int handle = PutLoaded(path, bank);

	// enumerate all items in bank
	// not sure if this is needed?

	LoadFromBank<EventDescription>(
		bank,
		[](Bank* bank, int& count) {
			return bank->getEventCount(&count);
		},
		[](Bank* bank, EventDescription** items, int count) {
			int _;
			return bank->getEventList(items, count, &_);
		}
	);

	LoadFromBank<Bus>(
		bank,
		[](Bank* bank, int& count) {
			return bank->getBusCount(&count);
		},
		[](Bank* bank, Bus** items, int count) {
			int _;
			return bank->getBusList(items, count, &_);
		}
	);
			
	LoadFromBank<VCA>(
		bank,
		[](Bank* bank, int& count) {
			return bank->getVCACount(&count);
		},
		[](Bank* bank, VCA** items, int count) {
			int _;
			return bank->getVCAList(items, count, &_);
		}
	);

	return handle;
}

int Audio::Play(const std::string& path)
{
	if (path.find("event:/") == 0)
	{
		int eventHandle = GetHandle(path);

		if (eventHandle == ENGINE_FAILED_HANDLE_NOT_LOADED)
		{
			return audio_log_error(ENGINE_FAILED_LOAD_INSTANCE);
		}

		EventInstance* instance;
		
		CHECK(
			m_events.at(eventHandle)->createInstance(&instance),
			ENGINE_FAILED_MAKE_INSTANCE
		);

		FMOD_STUDIO_EVENT_CALLBACK callback_stopped = [](
			FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
			FMOD_STUDIO_EVENTINSTANCE* einstance,
			void* parameters)
		{
			EventInstance* instance = (EventInstance*)einstance;
				
			InstanceUserData* data;
			instance->getUserData((void**)&data);
			auto [inst, me] = *data;
			me->m_instances.erase(inst);
			instance->release();
			delete data;

			return FMOD_OK;
		};

		// this overwrites the Loaded list everytime one spawns,
		// that might be ok? GetHandle(path) gives you the last spawned
		// but all the handles are still valid
		
		int instanceHandle = PutLoaded(path, instance);

		CHECK(
			instance->setUserData(new InstanceUserData(instanceHandle, this)),
			ENGINE_FAILED_MAKE_INSTANCE
		);

		CHECK(
			instance->setCallback(callback_stopped, FMOD_STUDIO_EVENT_CALLBACK_STOPPED),
			ENGINE_FAILED_MAKE_INSTANCE
		);

		Start(instanceHandle);

		return instanceHandle;
	}

	// this is where playing raw mp3s would be, but currently only supporting banks

	return ENGINE_OK;
}

int Audio::Set(int handle, const std::string& parameter, float value)
{
	CHECK_HANDLE(handle);

	if (audio_get_type(handle) == H_INSTANCE)
	{
		CHECK(
			m_instances[handle]->setParameterByName(parameter.c_str(), value),
			ENGINE_FAILED_SET_PARAM
		);

		audio_log("Setting instance parameter '%s' on '%d' to '%f'", parameter.c_str(), handle, value);
	}

	return ENGINE_OK;
}

int Audio::Get(int handle, const std::string& parameter, float& value)
{
	CHECK_HANDLE(handle);

	if (audio_get_type(handle) == H_INSTANCE)
	{
		CHECK(
			m_instances[handle]->getParameterByName(parameter.c_str(), &value),
			ENGINE_FAILED_GET_PARAM
		);
	}

	return ENGINE_OK;
}

int Audio::Start(int handle)
{
	CHECK_HANDLE(handle);

	if (audio_get_type(handle) != H_INSTANCE)
	{
		return audio_log_error(ENGINE_FAILED_INVALID_HANDLE);
	}

	CHECK(m_instances[handle]->start(), ENGINE_FAILED_START_INSTANCE);
	return ENGINE_OK;
}

int Audio::Stop(int handle)
{
	CHECK_HANDLE(handle);

	if (audio_get_type(handle) != H_INSTANCE)
	{
		return audio_log_error(ENGINE_FAILED_INVALID_HANDLE);
	}

	CHECK(m_instances[handle]->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT), ENGINE_FAILED_STOP_INSTANCE);
	return ENGINE_OK;
}

int Audio::Free(int handle)
{
	CHECK_HANDLE(handle);

	if (audio_get_type(handle) != H_INSTANCE)
	{
		return audio_log_error(ENGINE_FAILED_INVALID_HANDLE);
	}

	CHECK(m_instances[handle]->release(), ENGINE_FAILED_FREE_INSTANCE);
	return ENGINE_OK;
}

int Audio::SetVolume(int handle, float volume)
{
	CHECK_HANDLE(handle);

	switch (audio_get_type(handle))
	{
		case H_INSTANCE: m_instances[handle]->setVolume(volume); break;
		case H_BUS:      m_buses    [handle]->setVolume(volume); break;
		case H_VCA:      m_vcas     [handle]->setVolume(volume); break;
		default:
		{
			return audio_log_error(ENGINE_FAILED_INVALID_HANDLE);
		}
	}

	return ENGINE_OK;
}

int Audio::GetVolume(int handle, float& volume)
{
	CHECK_HANDLE(handle);

	switch (audio_get_type(handle))
	{
		case H_INSTANCE: m_instances[handle]->getVolume(&volume); break;
		case H_BUS:      m_buses    [handle]->getVolume(&volume); break;
		case H_VCA:      m_vcas     [handle]->getVolume(&volume); break;
		default:
		{
			return audio_log_error(ENGINE_FAILED_INVALID_HANDLE);
		}
	}

	return ENGINE_OK;
}

bool Audio::IsLoaded(int handle) const
{
	switch (audio_get_type(handle))
	{
		case H_BANK:     return m_banks    .find(handle) != m_banks    .end();
		case H_EVENT:    return m_events   .find(handle) != m_events   .end();
		case H_INSTANCE: return m_instances.find(handle) != m_instances.end();
		case H_BUS:      return m_buses    .find(handle) != m_buses    .end();
		case H_VCA:      return m_vcas     .find(handle) != m_vcas     .end();
		default:
		{
			return audio_log_error(ENGINE_FAILED_INVALID_HANDLE);
		}
	}

	return 0;
}

bool Audio::IsLoaded(const std::string& path) const
{
	return m_loaded.find(path) != m_loaded.end();
}

int Audio::GetHandle(const std::string& path) const
{
	auto itr = m_loaded.find(path);
	if (itr == m_loaded.end())
	{
		return audio_log_error(ENGINE_FAILED_HANDLE_NOT_LOADED);
	}

	return itr->second;
}

int Audio::MakeHandle(int type) const
{
	return audio_set_type(type, (int)m_loaded.size() + 1);
}

void Audio::PutLoaded(const std::string& path, int handle)
{
	m_loaded.emplace(path, handle);
	audio_log("Loaded '%s' (handle: %d)", path.c_str(), handle);
}

int Audio::PutLoaded(const std::string& path, Bank* bank)
{
	int handle = MakeHandle(H_BANK);
	PutLoaded(path, handle);
	m_banks[handle] = bank;
	return handle;
}

int Audio::PutLoaded(const std::string& path, EventDescription* event)
{
	int handle = MakeHandle(H_EVENT);
	PutLoaded(path, handle);
	m_events[handle] = event;
	return handle;
}

int Audio::PutLoaded(const std::string& path, EventInstance* instance)
{
	int handle = MakeHandle(H_INSTANCE);
	PutLoaded(path, handle);
	m_instances[handle] = instance;
	return handle;
}

int Audio::PutLoaded(const std::string& path, Bus* bus)
{
	int handle = MakeHandle(H_BUS);
	PutLoaded(path, handle);
	m_buses[handle] = bus;
	return handle;
}

int Audio::PutLoaded(const std::string& path, VCA* vca)
{
	int handle = MakeHandle(H_VCA);
	PutLoaded(path, handle);
	m_vcas[handle] = vca;
	return handle;
}

//void AudioEmitter::_SetAudio(Audio* audio)
//{
//	m_audio = audio; 
//}
//
//AudioEmitter& AudioEmitter::SetEvent(const std::string& event)
//{
//	this->event = event;
//	return *this;
//}
//
//bool AudioEmitter::IsPlaying() const
//{
//	return instance != 0;
//}
//
//int AudioEmitter::GetInstance() const
//{
//	return instance;
//}
//
//int AudioEmitter::PlayAndForget()
//{
//	return m_audio->Play(event);
//}
//
//AudioEmitter& AudioEmitter::Play()  { instance = PlayAndForget(); return *this; }
//AudioEmitter& AudioEmitter::Start() { m_audio->Start(instance); return *this; }
//AudioEmitter& AudioEmitter::Stop()  { m_audio->Stop(instance); return *this; }
//AudioEmitter& AudioEmitter::Free()  { m_audio->Free(instance); return *this; }
//
//AudioEmitter& AudioEmitter::Set(const std::string& param, float value)
//{
//	m_audio->Set(instance, param, value);
//	return *this;
//}
//
//float AudioEmitter::Get(const std::string& param)
//{
//	float x = 0.f;
//	m_audio->Get(instance, param, x);
//	return x;
//}
//
//AudioEmitter& AudioEmitter::SetVolume(float volume)
//{
//	m_audio->SetVolume(instance, volume);
//	return *this;
//}
//
//float AudioEmitter::GetVolume() {
//	float x = 0.f;
//	m_audio->GetVolume(instance, x);
//	return x;
//}