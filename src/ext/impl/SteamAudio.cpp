#include "ext/SteamAudio.h"

// shouldnt need this put this in premake5.lua
//#define IPL_OS_WINDOWS

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
	// Load the phonon dll

	m_audio.LoadPlugin("phonon_fmod.dll");

	// Create context

	IPLContextSettings vsettings = { };
	vsettings.version = STEAMAUDIO_VERSION;
	vsettings.logCallback = steam_audio_log;

	iplContextCreate(&vsettings, &m_steam);

	// Create hrtf
	
	IPLAudioSettings audioSettings = {};
	audioSettings.samplingRate = 44100;
	audioSettings.frameSize = 1024;

	IPLHRTFSettings hrtfSettings = {};
	hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;

	IPLHRTF hrtf = nullptr;
	iplHRTFCreate(m_steam, &audioSettings, &hrtfSettings, &hrtf);

	// Create a simulator for direct and reflection simulations

	IPLSimulationSettings simulationSettings = {};
	simulationSettings.flags = IPL_SIMULATIONFLAGS_REFLECTIONS;
	simulationSettings.sceneType = IPL_SCENETYPE_DEFAULT;
	simulationSettings.reflectionType = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
	simulationSettings.maxNumRays = 4096;
	simulationSettings.numDiffuseSamples = 32;
	simulationSettings.maxDuration = 2.0f;
	simulationSettings.maxOrder = 1;
	simulationSettings.maxNumSources = 8;
	simulationSettings.numThreads = 2;
	simulationSettings.samplingRate = audioSettings.samplingRate;
	simulationSettings.frameSize = audioSettings.frameSize;

	iplSimulatorCreate(m_steam, &simulationSettings, &m_simulator);

	// Create scene

	IPLSceneSettings sceneSettings = {};
	sceneSettings.type = simulationSettings.sceneType;

	iplSceneCreate(m_steam, &sceneSettings, &m_scene);
	iplSimulatorSetScene(m_simulator, m_scene);

	// Create a source for listening for reflections

	IPLSourceSettings sourceSettings = {};
	sourceSettings.flags = IPL_SIMULATIONFLAGS_REFLECTIONS;

	iplSourceCreate(m_simulator, &sourceSettings, &m_listenerSource);
	iplSourceAdd(m_listenerSource, m_simulator);

	// Tell fmod about the state of phonon

	iplFMODInitialize(m_steam);
	iplFMODSetHRTF(hrtf);
	iplFMODSetSimulationSettings(simulationSettings);
	iplFMODSetReverbSource(m_listenerSource);
}

void SteamAudio::Tick_temp()
{
	m_audio.Tick();
}

void SteamAudio::RunSimulation()
{
	// Apply any changes to the scene / simulator

	iplSceneCommit(m_scene);
	iplSimulatorCommit(m_simulator);

	IPLCoordinateSpace3 listenerPos = tocs(m_listenerPosition);
	IPLCoordinateSpace3 testPos = tocs(m_listenerPosition + vec3(0, 0, 0));

	// Set source for reflections
	// the docs say to put this at the listener position and enable reflection sim

	IPLSimulationInputs listenerInputs = {};
    listenerInputs.source = listenerPos;
    listenerInputs.flags = IPL_SIMULATIONFLAGS_REFLECTIONS;

    iplSourceSetInputs(m_listenerSource, IPL_SIMULATIONFLAGS_REFLECTIONS, &listenerInputs);

	// Set listener position for direct sources
	// Set listener position and ray tracing settings for reflection sources

	IPLSimulationSharedInputs sharedDirect = {};
	sharedDirect.listener = listenerPos;

    IPLSimulationSharedInputs sharedReflection = {};
    sharedReflection.listener = testPos;
    sharedReflection.duration = 1.05;
    sharedReflection.numRays = 3000;
    sharedReflection.numBounces = 5;
    sharedReflection.irradianceMinDistance = .1f;
    sharedReflection.order = 1;

    iplSimulatorSetSharedInputs(m_simulator, IPL_SIMULATIONFLAGS_DIRECT, &sharedDirect);
    iplSimulatorSetSharedInputs(m_simulator, IPL_SIMULATIONFLAGS_REFLECTIONS, &sharedReflection);

	// Set direct audio settings

	for (SteamAudioSource& source : m_simulateDirect)
	{
		IPLSource ptr = source.GetSource();

		IPLCoordinateSpace3 sourcePos = tocs(source.GetProps3D().position);
		
		IPLSimulationInputs inputsDirect = {};
		inputsDirect.flags = IPL_SIMULATIONFLAGS_DIRECT;
		inputsDirect.directFlags = IPL_DIRECTSIMULATIONFLAGS_OCCLUSION;
		inputsDirect.occlusionType = IPL_OCCLUSIONTYPE_RAYCAST;
		inputsDirect.source = sourcePos;

		iplSourceSetInputs(ptr, IPL_SIMULATIONFLAGS_DIRECT, &inputsDirect);

		IPLSimulationInputs inputsReflection = {};
		inputsReflection.flags = IPL_SIMULATIONFLAGS_REFLECTIONS;
		inputsReflection.source = sourcePos;

		iplSourceSetInputs(ptr, IPL_SIMULATIONFLAGS_REFLECTIONS, &inputsReflection);
	}

	// Run simulations

	iplSimulatorRunDirect(m_simulator);
	//iplSimulatorRunReflections(m_simulator);

	// Set the dsp parameters for the phonon fmod plugins

	for (SteamAudioSource& source : m_simulateDirect)
	{
		IPLSource ptr = source.GetSource();

		source.SetParamDSP(0, APPLY_OCCLUSION, 1);
		source.SetParamDSP(0, SIMULATION_OUTPUTS, &ptr, sizeof(IPLSource*));
	}
}

SteamAudioSource SteamAudio::CreateSource(const std::string& eventName)
{
	Audio audio = m_audio.CreateAudio(eventName);
	
	IPLSourceSettings sourceSettings = {};
	sourceSettings.flags = (IPLSimulationFlags)(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS);

	IPLSource source;
	iplSourceCreate(m_simulator, &sourceSettings, &source);
	iplSourceAdd(source, m_simulator);

	SteamAudioSource sas = SteamAudioSource(audio, source);
	m_simulateDirect.push_back(sas);

	return sas;
}

void SteamAudio::CreateStaticMesh(const Mesh& _mesh)
{
	// Copy the mesh into the phonon format

	auto verts = _mesh.View<vec3>(Mesh::aPosition);
	auto index = _mesh.View<int>(Mesh::aIndexBuffer);

	int triCount = index.size() / 3;

	IPLMaterial material = { 0.13f, 0.20f, 0.24f, 0.05f, 0.015f, 0.002f, 0.001f };

	IPLStaticMeshSettings meshSettings = {};

	meshSettings.numTriangles = triCount;
	meshSettings.numMaterials = 1;

	meshSettings.vertices = (IPLVector3*)verts.data();
	meshSettings.numVertices = verts.size();

	meshSettings.materials = new IPLMaterial[1]{ material };

	meshSettings.triangles = new IPLTriangle[triCount];
	meshSettings.materialIndices = new IPLint32[triCount];

	for (int i = 0; i < triCount; i++)
	{
		meshSettings.triangles[i].indices[0] = index.at(i * 3 + 0);
		meshSettings.triangles[i].indices[1] = index.at(i * 3 + 1);
		meshSettings.triangles[i].indices[2] = index.at(i * 3 + 2);

		meshSettings.materialIndices[i] = 0;
	}

	// Create mesh

	IPLStaticMesh mesh;

	iplStaticMeshCreate(m_scene, &meshSettings, &mesh);
	iplStaticMeshAdd(mesh, m_scene);
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
    space.right = IPLVector3{1, 0, 0};
    space.up = IPLVector3{0, 1, 0};
    space.ahead = IPLVector3{0, 0, 1};

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

_IPLSource_t* SteamAudioSource::GetSource()
{
	return m_source;
}
