#include "ext/SteamAudio.h"

// shouldnt need this put this in premake5.lua
#define IPL_OS_WINDOWS

#include "fmod_steamaudio/phonon.h"
#include "fmod_steamaudio/steamaudio_fmod.h"
#include "fmod_steamaudio/steamaudio_spatialize_effect.h"

bool sa(IPLerror result);
void steam_audio_log(IPLLogLevel level, const char* log);

SteamAudio::SteamAudio(AudioWorld& audio)
	: m_audio     (audio)
	, m_steam     (nullptr)
	, m_simulator (nullptr)
	, m_scene     (nullptr)
{}

void SteamAudio::Init()
{
	// Steam audio, could make optional

	m_audio.LoadPlugin("phonon_fmod.dll");

	IPLContextSettings vsettings = { };
	vsettings.version = STEAMAUDIO_VERSION;
	vsettings.logCallback = steam_audio_log;

	m_steam = nullptr;

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
    simulationSettings.flags = IPLSimulationFlags::IPL_SIMULATIONFLAGS_DIRECT;
    simulationSettings.sceneType = IPLSceneType::IPL_SCENETYPE_DEFAULT;
	simulationSettings.reflectionType = IPLReflectionEffectType::IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
    simulationSettings.maxNumOcclusionSamples = 16;
    simulationSettings.maxNumRays = 4096;
    simulationSettings.numDiffuseSamples = 32;
    simulationSettings.maxDuration = 10.0;
    simulationSettings.maxOrder = 5;
    simulationSettings.maxNumSources = 10;
    simulationSettings.numThreads = 6;
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

	IPLSource source;
	IPLSourceSettings sourceSettings = {};
	sourceSettings.flags = IPLSimulationFlags::IPL_SIMULATIONFLAGS_DIRECT;

	if (sa(iplSourceCreate(m_simulator, &sourceSettings, &source)))
	{
		return;
	}

	iplSourceAdd(source, m_simulator);

	iplSimulatorCommit(m_simulator);
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
	iplSceneCommit(m_scene);
}

void SteamAudio::RunSimulation()
{
	iplSceneCommit(m_scene);
	iplSimulatorCommit(m_simulator);

	iplSimulatorRunDirect(m_simulator);
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


	return SteamAudioSource(audio, source);
}

void steam_audio_log(IPLLogLevel level, const char* log)
{
	log_audio("[STEAMM %d] %s", (int)level, log);
}

bool sa(IPLerror result)
{
	bool err = result != IPLerror::IPL_STATUS_SUCCESS;
	if (err) log_audio("e~Steam Audio error: %d", result);
	return err;
}

SteamAudioSource::SteamAudioSource(Audio audio, _IPLSource_t* source)
	: m_audio  (audio)
	, m_source (source)
{}

void SteamAudioSource::UpdateAudio()
{
	IPLSimulationOutputs output;
	iplSourceGetOutputs(m_source, IPLSimulationFlags::IPL_SIMULATIONFLAGS_DIRECT, &output);

	m_audio.SetParamDSP(1, APPLY_OCCLUSION, 1);
	m_audio.SetParamDSP(1, SIMULATION_OUTPUTS, &output, sizeof(IPLSimulationOutputs*));
}
