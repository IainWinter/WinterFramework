#include "ext/SteamAudio.h"

// shouldnt need this put this in premake5.lua
#define IPL_OS_WINDOWS

#include <assert.h>
#include <string.h>

#include <algorithm>
#include <atomic>

#include "fmod_steamaudio/phonon.h"
#include "fmod_steamaudio/steamaudio_fmod.h"
#include "fmod_steamaudio/steamaudio_spatialize_effect.h"

bool sa(IPLerror result);
void steam_audio_log(IPLLogLevel level, const char* log);
IPLVector3 tov3(vec3 v);
IPLCoordinateSpace3 tocs(vec3 v);

SteamAudio::SteamAudio(AudioWorld& audio)
	: m_audio     (audio)
	, m_steam     (nullptr)
	, m_simulator (nullptr)
	, m_scene     (nullptr)
{
	m_listenerPosition = vec3(0.f);
}

void SteamAudio::Init()
{
	m_audio.LoadPlugin("phonon_fmod.dll");

	IPLContextSettings vsettings = { };
	vsettings.version = STEAMAUDIO_VERSION;
	vsettings.logCallback = steam_audio_log;

	if (sa(iplContextCreate(&vsettings, &m_steam)))
	{
		return;
	}

	IPLHRTFSettings hrtfSettings{};
	hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;

	IPLAudioSettings audioSettings{};
	audioSettings.samplingRate = 44100;
	audioSettings.frameSize = 1024;

	IPLHRTF hrtf = nullptr;
	if (sa(iplHRTFCreate(m_steam, &audioSettings, &hrtfSettings, &hrtf)))
	{
		return;
	}

	iplFMODInitialize(m_steam);
	iplFMODSetHRTF(hrtf);

	IPLSimulationSettings simulationSettings = {};
    simulationSettings.flags = (IPLSimulationFlags)(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS);
    simulationSettings.sceneType = IPL_SCENETYPE_DEFAULT;
	simulationSettings.reflectionType = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
    simulationSettings.maxNumOcclusionSamples = 16;
    simulationSettings.maxNumRays = 4096;
    simulationSettings.numDiffuseSamples = 32;
    simulationSettings.maxDuration = 10.0;
    simulationSettings.maxOrder = 5;
    simulationSettings.maxNumSources = 10;
    simulationSettings.numThreads = 8;
    simulationSettings.rayBatchSize = 512;
    simulationSettings.numVisSamples = 128;
    simulationSettings.samplingRate = audioSettings.samplingRate;
    simulationSettings.frameSize = audioSettings.frameSize;
    simulationSettings.openCLDevice = nullptr;
    simulationSettings.radeonRaysDevice = nullptr;
	simulationSettings.tanDevice = nullptr;

	iplFMODSetSimulationSettings(simulationSettings);

	IPLSceneSettings sceneSettings = {};
	sceneSettings.type = simulationSettings.sceneType;

	if (sa(iplSceneCreate(m_steam, &sceneSettings, &m_scene)))
	{
		return;
	}

	if (sa(iplSimulatorCreate(m_steam, &simulationSettings, &m_simulator)))
	{
		return;
	}

	iplSimulatorSetScene(m_simulator, m_scene);

	// create reverb source

	IPLSourceSettings sourceSettings = {};
	sourceSettings.flags = IPL_SIMULATIONFLAGS_REFLECTIONS;

	if (sa(iplSourceCreate(m_simulator, &sourceSettings, &m_listenerSource)))
	{
		return;
	}

	iplSourceAdd(m_listenerSource, m_simulator);
	iplFMODSetReverbSource(m_listenerSource);
}

void SteamAudio::SetSimulationScene(const Mesh& _mesh)
{
	// For now just reset all meshes
	for (IPLStaticMesh mesh : m_meshes)
		iplStaticMeshRemove(mesh, m_scene);
	m_meshes.clear();

	// get views into new mesh

	auto verts = _mesh.View<vec3>(Mesh::aPosition);
	auto index = _mesh.View<int>(Mesh::aIndexBuffer);

	IPLMaterial material = { 0.13f, 0.20f, 0.24f, 0.05f, 0.015f, 0.002f, 0.001f };
	
	IPLStaticMeshSettings meshSettings = {};
	meshSettings.vertices = (IPLVector3*)verts.data();
	meshSettings.numVertices = verts.size();

	meshSettings.materials = new IPLMaterial[1]{ material };
	meshSettings.numMaterials = 1;

	int triCount = index.size() / 3;

	meshSettings.triangles = new IPLTriangle[triCount];
	meshSettings.materialIndices = new IPLint32[triCount];
	meshSettings.numTriangles = triCount;

	for (int i = 0; i < triCount; i++)
	{
		meshSettings.triangles[i].indices[0] = index.at(i * 3 + 0);
		meshSettings.triangles[i].indices[1] = index.at(i * 3 + 1);
		meshSettings.triangles[i].indices[2] = index.at(i * 3 + 2);

		meshSettings.materialIndices[i] = 0;
	}

	IPLStaticMesh mesh;

	if (sa(iplStaticMeshCreate(m_scene, &meshSettings, &mesh)))
	{
		return;
	}

	iplStaticMeshAdd(mesh, m_scene);
}

void SteamAudio::Tick_temp()
{
	m_audio.Tick();
}

void SteamAudio::RunSimulation()
{
	IPLCoordinateSpace3 pos = tocs(m_listenerPosition);

	IPLSimulationInputs listenerInputs = {};
	listenerInputs.flags = IPL_SIMULATIONFLAGS_REFLECTIONS;
	listenerInputs.source = pos;

	iplSourceSetInputs(m_listenerSource, IPL_SIMULATIONFLAGS_REFLECTIONS, &listenerInputs);

	//IPLSimulationSharedInputs sharedInputs{};
	//sharedInputs.listener = pos;

	//iplSimulatorSetSharedInputs(m_simulator, IPL_SIMULATIONFLAGS_DIRECT, &sharedInputs);

	iplSceneCommit(m_scene);
	iplSimulatorCommit(m_simulator);

	for (SteamAudioSource& source : m_sources)
		source.UpdateAudioPre();

	iplSimulatorRunDirect(m_simulator);
	iplSimulatorRunReflections(m_simulator);

	for (SteamAudioSource& source : m_sources)
		source.UpdateAudio();
}

SteamAudioSource SteamAudio::CreateSource(const std::string& eventName)
{
	Audio audio = m_audio.CreateAudio(eventName);

	if (!audio.IsAlive())
		return SteamAudioSource(audio, nullptr);

	IPLSource source;
	IPLSourceSettings sourceSettings = {};
	sourceSettings.flags = IPLSimulationFlags::IPL_SIMULATIONFLAGS_DIRECT;

	if (sa(iplSourceCreate(m_simulator, &sourceSettings, &source)))
	{
		audio.Destroy();
		return SteamAudioSource({}, nullptr);
	}

	iplSourceAdd(source, m_simulator);

	SteamAudioSource sas = SteamAudioSource(audio, source);
	m_sources.push_back(sas);

	return sas;
}

void SteamAudio::SetListenerPosition(vec3 position)
{
	m_listenerPosition = position;
}

bool sa(IPLerror result)
{
	bool err = result != IPLerror::IPL_STATUS_SUCCESS;
	if (err) log_audio("e~Steam Audio error: %d", result);
	return err;
}

void steam_audio_log(IPLLogLevel level, const char* log)
{
	log_audio("[Steam %d] %s", (int)level, log);
}

IPLVector3 tov3(vec3 v)
{
	return IPLVector3 {
		v.x, v.y, v.z
	};
}

IPLCoordinateSpace3 tocs(vec3 v)
{
	IPLCoordinateSpace3 space;
	space.origin = tov3(v);
	space.right = IPLVector3(1, 0, 0);
	space.up = IPLVector3(0, 1, 0);
	space.ahead = IPLVector3(0, 0, 1);

	return space;
}

SteamAudioSource::SteamAudioSource()
	: Audio    ()
	, m_source (nullptr)
{}

SteamAudioSource::SteamAudioSource(Audio audio, _IPLSource_t* source)
	: Audio    (audio)
	, m_source (source)
{}

void SteamAudioSource::UpdateAudioPre()
{
	vec3 position = GetProps3D().position;

	IPLSimulationInputs inputs = {};
	inputs.flags = IPL_SIMULATIONFLAGS_DIRECT;
	inputs.occlusionRadius = 0.1f;

	inputs.directFlags = (IPLDirectSimulationFlags)(
		  IPL_DIRECTSIMULATIONFLAGS_OCCLUSION
		| IPL_DIRECTSIMULATIONFLAGS_AIRABSORPTION 
		| IPL_DIRECTSIMULATIONFLAGS_DISTANCEATTENUATION 
		| IPL_DIRECTSIMULATIONFLAGS_DIRECTIVITY
		| IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION);

	inputs.source = tocs(position);

	iplSourceSetInputs(m_source, IPL_SIMULATIONFLAGS_DIRECT, &inputs);
}

void SteamAudioSource::UpdateAudio()
{
	IPLSimulationOutputs output = {};
	output.direct.flags = IPL_DIRECTEFFECTFLAGS_APPLYOCCLUSION;

	iplSourceGetOutputs(m_source, IPLSimulationFlags::IPL_SIMULATIONFLAGS_DIRECT, &output);

	SetParamDSP(0, APPLY_OCCLUSION, 1);
	SetParamDSP(0, APPLY_AIRABSORPTION, 1);
	SetParamDSP(0, APPLY_DIRECTIVITY, 1);
	SetParamDSP(0, APPLY_TRANSMISSION, 1);
	SetParamDSP(0, SIMULATION_OUTPUTS, &m_source, sizeof(IPLSource*));
}
