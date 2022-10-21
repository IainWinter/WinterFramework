#include "Audio.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_studio.hpp"

#include <array>
#include <time.h> // for seed

using namespace FMOD::Studio;

bool fa(int result)
{
	bool err = result != FMOD_OK;
	if (err) log_audio("e~FMOD error: %d", result);
	return err;
}

template<typename _t>
std::string get_name(_t* fmod_thing)
{
	std::string name(1024, '\0');
	int size;

	if (fa(fmod_thing->getPath(name.data(), 1024, &size)))
	{
		log_audio("w~Failed to get name");
		name = "";
	}

	else
	{
		name.resize(size - 1);
	}

	return name;
}

void AudioWorld::Init()
{
	if (fa(System::create(&m_system)))
	{
		log_audio("e~Failed to create audio engine");
		return;
	}

	FMOD_ADVANCEDSETTINGS settings;
	settings.randomSeed = (unsigned int)time(nullptr);

	FMOD::System* core;
	m_system->getCoreSystem(&core);
	core->setAdvancedSettings(&settings);

	if (fa(m_system->initialize(16, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr)))
	{
		log_audio("e~Failed to init audio engine");
		return;
	}

	log_audio("d~Successfully created audio engine");
}

void AudioWorld::Dnit()
{
	if (!m_system)
	{
		log_audio("e~Failed to dnit audio engine. Audio engine has failed to init");
		return;
	}

	if (fa(m_system->release()))
	{
		log_audio("e~Failed to destroy audio engine");
	}
}

void AudioWorld::Tick()
{
	if (!m_system)
	{
		log_audio("e~Failed to tick audio engine. Audio engine has failed to init");
		return;
	}

	if (fa(m_system->update()))
	{
		log_audio("e~Failed to tick audio engine");
	}
}

void AudioWorld::LoadBank(const std::string& bankFilePath)
{
	if (!m_system)
	{
		log_audio("e~Failed to load bank. Audio engine has failed to init");
		return;
	}

	if (IsBankLoaded(bankFilePath))
	{
		log_audio("w~Failed to load bank. Already loaded");
		return;
	}

	Bank* bank = nullptr;

	if (fa(m_system->loadBankFile(_a(bankFilePath).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank)))
	{
		log_audio("e~Failed to load bank");
		return;
	}

	std::string bankName = get_name(bank);
	m_bank[bankName] = bank;
	
	log_audio("i~Successfully loaded bank '%s':", bankName.c_str());
	log_audio("i~- Events");

	for (EventDescription* desc : GetEventDescriptions(bank))
	{
		std::string descName = get_name(desc);
		m_desc[descName] = AudioSource(desc);

		log_audio("i~  > %s", descName.c_str());
	}

	log_audio("i~- VCAs");

	for (VCA* vca : GetVCAs(bank))
	{
		std::string vcaName = get_name(vca);
		m_vca[vcaName] = AudioVCA(vca);

		log_audio("i~  > %s", vcaName.c_str());
	}
}

void AudioWorld::FreeBank(const std::string& bankName)
{
	if (!m_system)
	{
		log_audio("e~Failed to free bank. Audio engine has failed to init");
		return;
	}

	if (!IsBankLoaded(bankName))
	{
		log_audio("e~Failed to free bank. Not loaded");
		return;
	}

	Bank* bank = m_bank[bankName];

	for (EventDescription* desc : GetEventDescriptions(bank))
	{
		m_desc.erase(get_name(desc));
	}

	for (VCA* vca : GetVCAs(bank))
	{
		m_vca.erase(get_name(vca));
	}

	m_bank.erase(bankName);

	bank->unload();
}

bool AudioWorld::IsBankLoaded(const std::string& bankName)
{
	return m_bank.find(bankName) != m_bank.end();
}

AudioVCA AudioWorld::GetVCA(const std::string& vcaName)
{
	auto itr = m_vca.find(vcaName);
	if (itr == m_vca.end())
	{
		log_audio("w~Failed to get VCA. Doesn't exist");
		return AudioVCA();
	}

	return m_vca.find(vcaName)->second;
}

AudioSource AudioWorld::GetAudioSource(const std::string& eventName)
{
	if (!m_system)
	{
		log_audio("e~Failed to get audio source. Audio engine has failed to init");
		return AudioSource();
	}

	auto desc = m_desc.find(eventName);
	if (desc == m_desc.end())
	{
		log_audio("e~Failed to get audio source. Event doesn't exist");
		return AudioSource();
	}

	return desc->second;
}

Audio AudioWorld::CreateAudio(const std::string& eventName)
{
	if (!m_system)
	{
		log_audio("e~Failed to create audio. Audio engine has failed to init");
		return Audio();
	}

	return GetAudioSource(eventName).Spawn();
}

std::vector<VCA*> AudioWorld::GetVCAs(Bank* bank)
{
	std::array<VCA*, 1024> tmp;
	int count;

	if (fa(bank->getVCAList(tmp.data(), 1024, &count)))
	{
		log_audio("e~Failed to load event descriptions from bank");
		return {};
	}

	return std::vector<VCA*>(tmp.begin(), tmp.begin() + count);
}

std::vector<EventDescription*> AudioWorld::GetEventDescriptions(Bank* bank)
{
	std::array<EventDescription*, 1024> tmp;
	int count;

	if (fa(bank->getEventList(tmp.data(), 1024, &count)))
	{
		log_audio("e~Failed to load event descriptions from bank");
		return {};
	}

	return std::vector<EventDescription*>(tmp.begin(), tmp.begin() + count);
}

AudioVCA::AudioVCA()
	: m_vca (nullptr)
{}

AudioVCA::AudioVCA(VCA* vca)
	: m_vca (vca)
{
	if (!m_vca)
	{
		log_audio("w~Created VCA with nullptr");
	}
}

bool AudioVCA::IsAlive() const
{
	return !!m_vca;
}

AudioVCA& AudioVCA::SetVolume(float volume)
{
	if (!IsAlive())
	{
		log_audio("e~Failed to set VCA volume. nullptr");
		return *this;
	}

	if (fa(m_vca->setVolume(volume)))
	{
		log_audio("e~Failed to set VCA volume");
		return *this;
	}

	log_audio("i~Set VCA %s volume to %f", get_name(m_vca).c_str(), volume);

	return *this;
}

float AudioVCA::GetVolume() const
{
	if (!IsAlive())
	{
		log_audio("e~Failed to get VCA volume. nullptr");
		return 0.f;
	}

	float volume;
	if (fa(m_vca->getVolume(&volume)))
	{
		log_audio("e~Failed to get VCA volume");
	}

	return volume;
}

Audio::Audio()
	: m_inst (nullptr)
{}

Audio::Audio(AudioSource& parent)
{
	*this = parent.Spawn();
}

Audio::Audio(EventInstance* inst, AudioSource& parent)
	: m_inst   (inst)
	, m_parent (parent)
{
	if (!m_inst)
	{
		log_audio("w~Created an audio instance with nullptr");
	}

	if (!m_parent.IsAlive())
	{
		log_audio("w~Created an audio instance with a dead parent");
	}
}

AudioSource Audio::GetParent() const
{
	return m_parent;
}

// internal

std::string InstName(EventInstance* inst)
{
	EventDescription* desc;
	inst->getDescription(&desc);
	return get_name(desc);
}

void StopInstance(EventInstance* inst)
{
	if (fa(inst->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT)))
	{
		log_audio("e~Failed to stop audio instance");
		return;
	}

	log_audio("i~Stopped audio instance %s", InstName(inst).c_str());
}

void DestroyInstance(EventInstance* inst)
{
	StopInstance(inst);

	std::string name = InstName(inst);

	if (fa(inst->release()))
	{
		log_audio("e~Failed to destroy audio instance");
		return;
	}

	log_audio("i~Destroied audio instance %s", name.c_str());
}

// end internal

Audio& Audio::Start()
{
	if (!IsAlive())
	{
		log_audio("e~Failed to start audio instance. nullptr");
		return *this;
	}

	if (fa(m_inst->start()))
	{
		log_audio("e~Failed to start audio instance");
	}

	EventDescription* desc;
	m_inst->getDescription(&desc);
	log_audio("i~Started audio instance %s", get_name(desc).c_str());

	return *this;
}

Audio& Audio::Stop()
{
	if (!IsAlive())
	{
		log_audio("e~Failed to stop audio instance. nullptr");
		return *this;
	}

	StopInstance(m_inst);

	return *this;
}

void Audio::Destroy()
{
	if (!IsAlive())
	{
		log_audio("e~Failed to destroy audio instance. Is has already been destroied");
		return;
	}

	DestroyInstance(m_inst);

	if (HasParent())
	{
		m_parent._RemoveInstance(m_inst);
	}

	m_inst = nullptr;
}

bool Audio::IsAlive() const
{
	return !!m_inst;
}

bool Audio::IsPlaying() const
{
	if (!IsAlive())
	{
		log_audio("e~Failed to get audio instance playback state. Is has already been destroied");
		return false;
	}

	FMOD_STUDIO_PLAYBACK_STATE state;
	m_inst->getPlaybackState(&state);

	return state == FMOD_STUDIO_PLAYBACK_PLAYING
		|| state == FMOD_STUDIO_PLAYBACK_STARTING;
}

bool Audio::HasParent() const
{
	return m_parent.IsAlive();
}

Audio& Audio::Pause()
{
	if (!IsAlive())
	{
		log_audio("e~Failed to pause audio instance. nullptr");
		return *this;
	}

	if (fa(m_inst->setPaused(true)))
	{
		log_audio("e~Failed to pause audio instance");
	}

	EventDescription* desc;
	m_inst->getDescription(&desc);
	log_audio("i~Paused audio instance %s", get_name(desc).c_str());

	return *this;
}

Audio& Audio::Resume()
{
	if (!IsAlive())
	{
		log_audio("e~Failed to resume audio instance. nullptr");
		return *this;
	}

	if (fa(m_inst->setPaused(false)))
	{
		log_audio("e~Failed to resume audio instance");
	}

	EventDescription* desc;
	m_inst->getDescription(&desc);
	log_audio("i~Resumed audio instance %s", get_name(desc).c_str());

	return *this;
}

Audio& Audio::TogglePause()
{
	if (IsPlaying()) Pause();
	else             Resume();
}

Audio& Audio::SetVolume(float volume)
{
	if (!IsAlive())
	{
		log_audio("e~Failed to set audio instance volume. nullptr");
		return *this;
	}

	if (fa(m_inst->setVolume(volume)))
	{
		log_audio("e~Failed to set audio instance volume");
	}

	EventDescription* desc;
	m_inst->getDescription(&desc);
	log_audio("i~Set audio instance %s volume to %f", get_name(desc).c_str(), volume);

	return *this;
}

float Audio::GetVolume() const
{
	if (!IsAlive())
	{
		log_audio("e~Failed to get audio instance volume. nullptr");
		return 0.f;
	}

	float volume;
	if (fa(m_inst->getVolume(&volume)))
	{
		log_audio("e~Failed to get audio instance volume");
	}

	return volume;
}

Audio& Audio::SetParam(const std::string& paramName, float volume)
{
	if (!IsAlive())
	{
		log_audio("e~Failed to set audio instance param '%s'. nullptr");
		return *this;
	}

	if (fa(m_inst->setParameterByName(paramName.c_str(), volume)))
	{
		log_audio("e~Failed to set audio instance param '%s'", paramName.c_str());
	}

	EventDescription* desc;
	m_inst->getDescription(&desc);
	log_audio("i~Set audio instance %s param %s to %f", get_name(desc).c_str(), paramName.c_str(), volume);

	return *this;
}

float Audio::GetParam(const std::string& paramName) const
{
	if (!IsAlive())
	{
		log_audio("e~Failed to get audio instance param. nullptr");
		return 0.f;
	}

	float param;
	if (fa(m_inst->getParameterByName(paramName.c_str(), &param)))
	{
		log_audio("e~Failed to get audio instance param '%s'", paramName.c_str());
	}

	return param;
}

Audio& Audio::SetTimeline(float milliseconds)
{
	if (!IsAlive())
	{
		log_audio("e~Failed to set audio instance timeline position. nullptr");
		return *this;
	}

	int millis = (int)floor(milliseconds * 1000.f);

	if (fa(m_inst->setTimelinePosition(millis)))
	{
		log_audio("e~Failed to set audio instance timeline position");
	}

	EventDescription* desc;
	m_inst->getDescription(&desc);
	log_audio("i~Set audio instance %s timeline position to %dms", get_name(desc).c_str(), millis);

	return *this;
}

float Audio::GetTimeline()
{
	if (!IsAlive())
	{
		log_audio("e~Failed to get audio instance timeline position. nullptr");
		return 0.f;
	}

	int timeline;
	if (fa(m_inst->getTimelinePosition(&timeline)))
	{
		log_audio("e~Failed to get audio instance timeline position");
	}

	return timeline / 1000.f;
}

bool Audio::operator==(const Audio& other) const { return m_inst == other.m_inst; }
bool Audio::operator!=(const Audio& other) const { return m_inst != other.m_inst; }

AudioSource::AudioSource()
	: m_desc  (nullptr)
	, m_insts (nullptr)
{}

AudioSource::AudioSource(EventDescription* desc)
	: m_desc  (desc)
	, m_insts (mkr<std::vector<EventInstance*>>())
{}

FMOD_RESULT WhenDone(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE* inst_in, void* user)
{
	if (type == FMOD_STUDIO_EVENT_CALLBACK_STOPPED)
	{
		EventInstance* inst = (EventInstance*)inst_in;
		
		std::vector<EventInstance*>* insts;
		inst->getUserData((void**)&insts);
		
		// audio could have been removed manually, this is async so just check if its in the list
		// before removing

		auto itr = std::find(insts->begin(), insts->end(), inst);
		if (itr != insts->end())
		{
			insts->erase(itr);
			log_audio("i~Cleaned up audio instance %s", InstName(inst).c_str());
		}
	}

	return FMOD_OK;
}

Audio AudioSource::Spawn()
{
	EventInstance* instance = nullptr;

	if (fa(m_desc->createInstance(&instance)))
	{
		log_audio("e~Failed to create audio");
		return Audio();
	}

	instance->setUserData((void*)m_insts.get());
	instance->setCallback(WhenDone);

	m_insts->push_back(instance);

	return Audio(instance, *this);
}

Audio AudioSource::LastInstance()
{
	if (NumberOfInstances() > 0)
	{
		return Audio(m_insts->back(), *this);
	}

	log_audio("w~Last instance returned empty Audio");

	return Audio();
}

void AudioSource::StopAll()
{
	for (EventInstance* insts : *m_insts)
	{
		DestroyInstance(insts);
	}

	// inst list should be empty now because of callbacks, these might be async though as some audio fades...
	// todo: investigate behaviour
}

bool AudioSource::IsAlive() const
{
	return !!m_desc;
}

int AudioSource::NumberOfInstances() const
{
	return (int)m_insts->size();
}

void AudioSource::_RemoveInstance(EventInstance* inst)
{
	auto itr = std::find(m_insts->begin(), m_insts->end(), inst);

	if (itr == m_insts->end())
	{
		log_audio("e~Failed to remove audio instance, doesn't exist in source");
		return;
	}

	m_insts->erase(itr);
}