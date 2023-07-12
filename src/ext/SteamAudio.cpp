#include "ext/SteamAudio.h"
#include "Log.h"

#include "phonon/phonon.h"
#include "phonon_fmod/steamaudio_fmod.h"
#include "phonon_fmod/dsp_names.h"

#include "io/LibraryName.h"

// commonly used flags 

IPLSimulationFlags IPL_DIRECT_AND_REFLECT = (IPLSimulationFlags)(
	  IPL_SIMULATIONFLAGS_DIRECT
	| IPL_SIMULATIONFLAGS_REFLECTIONS
);

IPLDirectSimulationFlags IPL_ALL_DIRECT = (IPLDirectSimulationFlags)(
	  IPL_DIRECTSIMULATIONFLAGS_DISTANCEATTENUATION
	| IPL_DIRECTSIMULATIONFLAGS_AIRABSORPTION
	| IPL_DIRECTSIMULATIONFLAGS_DIRECTIVITY
	| IPL_DIRECTSIMULATIONFLAGS_OCCLUSION 
	| IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION
);

//	Occlusion seems to need transmission to be enabled as well to work?
//
IPLDirectSimulationFlags IPL_OCCL_TRAN_DIRECT = (IPLDirectSimulationFlags)(
	  IPL_DIRECTSIMULATIONFLAGS_OCCLUSION
	| IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION
);

// Some logging and conversion functions

bool sa(IPLerror result);
void steam_audio_log(IPLLogLevel level, const char* log);
IPLVector3 tov3(vec3 v);
IPLCoordinateSpace3 tocs(vec3 v);

SteamAudio::SteamAudio(AudioWorld& audio)
	: m_audio     (audio)
	, m_steam     (nullptr)
	, m_simulator (nullptr)
	, m_scene     (nullptr)
	, m_listener  (nullptr)
{
	m_listenerPosition = vec3(0.f);
}

void SteamAudio::Init()
{
	// Load the phonon dll

	m_audio.LoadPlugin(GetLibraryName("phonon_fmod"));

	// Create context

	IPLContextSettings vsettings = { };
	vsettings.version = STEAMAUDIO_VERSION;
	vsettings.logCallback = steam_audio_log;

	iplContextCreate(&vsettings, &m_steam);

	// Create hrtf
	
	IPLAudioSettings audioSettings = {};
	audioSettings.samplingRate = 48000;
	audioSettings.frameSize = 1024;

	IPLHRTFSettings hrtfSettings = {};
	hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;

	IPLHRTF hrtf = nullptr;
	iplHRTFCreate(m_steam, &audioSettings, &hrtfSettings, &hrtf);

	// Create a simulator for direct and reflection simulations

	IPLSimulationSettings simulationSettings = {};
	simulationSettings.flags = IPL_DIRECT_AND_REFLECT;
	simulationSettings.sceneType = IPL_SCENETYPE_DEFAULT;
	simulationSettings.reflectionType = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
	simulationSettings.maxNumRays = 4096;
	simulationSettings.numDiffuseSamples = 32;
	simulationSettings.maxDuration = 2.0f;
	simulationSettings.maxOrder = 2;
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

	iplSourceCreate(m_simulator, &sourceSettings, &m_listener);
	iplSourceAdd(m_listener, m_simulator);

	// Tell fmod about the state of phonon

	iplFMODInitialize(m_steam);
	iplFMODSetHRTF(hrtf);
	iplFMODSetSimulationSettings(simulationSettings);
	iplFMODSetReverbSource(m_listener);

	m_simResultsReady = false;
	m_simInputsReady = false;
	m_simRunning = true;

	m_simulationThread = std::thread([this]()
	{
		while (true)
		{
			{ // wait for inputs
				std::unique_lock lock(m_simMut);
				m_simWait.wait(lock, [this]() { return (bool)m_simInputsReady; });
			}

			if (!m_simRunning) // break after for quick dnit
				break;

			iplSimulatorRunDirect(m_simulator);
			iplSimulatorRunReflections(m_simulator);

			m_simResultsReady = true;
			m_simInputsReady = false;
		}
	});
}

void SteamAudio::Dnit()
{
	m_simRunning = false;

	m_simInputsReady = true; // wake up thread
	m_simWait.notify_one();

	if (m_simulationThread.joinable())
		m_simulationThread.join();
}

void SteamAudio::WriteSimulationInputs()
{
	// Apply any changes to the scene / simulator

	iplSceneCommit(m_scene);
	iplSimulatorCommit(m_simulator);

	IPLCoordinateSpace3 listenerPos = tocs(m_listenerPosition);

	// Set source for reflections
	// the docs say to put this at the listener position and enable reflection sim

	IPLSimulationInputs listenerInputs = {};
    listenerInputs.source = listenerPos;
    listenerInputs.flags = IPL_SIMULATIONFLAGS_REFLECTIONS;

    iplSourceSetInputs(m_listener, IPL_SIMULATIONFLAGS_REFLECTIONS, &listenerInputs);

	// Set listener position for direct sources
	// Set listener position and ray tracing settings for reflection sources

	IPLSimulationSharedInputs sharedDirect = {};
	sharedDirect.listener = listenerPos;

    IPLSimulationSharedInputs sharedReflection = {};
    sharedReflection.listener = listenerPos;
    sharedReflection.duration = 1.05f;
    sharedReflection.numRays = 3000;
    sharedReflection.numBounces = 5;
    sharedReflection.irradianceMinDistance = .5f;
    sharedReflection.order = 1;

    iplSimulatorSetSharedInputs(m_simulator, IPL_SIMULATIONFLAGS_DIRECT, &sharedDirect);
    iplSimulatorSetSharedInputs(m_simulator, IPL_SIMULATIONFLAGS_REFLECTIONS, &sharedReflection);

	// Set direct audio settings

	for (SteamAudioSource& source : m_simulateDirect)
		source._UpdateSourceInputs();
}

void SteamAudio::ReadSimulationResults()
{
	// Set the dsp parameters for the phonon fmod plugins

	for (SteamAudioSource& source : m_simulateDirect)
		source._UpdateDSPParams();
}

void SteamAudio::RunSimulation()
{
	if (m_simResultsReady) // read data if sim is done
	{
		ReadSimulationResults();
		m_simResultsReady = false;
	}

	if (!m_simInputsReady) // Upload data if sim is done
	{
		WriteSimulationInputs();
		m_simInputsReady = true;
		m_simWait.notify_one();
	}
}

SteamAudioSource SteamAudio::CreateSource(const std::string& eventName)
{
    if (!m_simulator)
    {
        log_audio("Failed to create a Steam Audio Source");
        return {};
    }
    
	Audio audio = m_audio.CreateAudio(eventName);
	
	IPLSourceSettings sourceSettings = {};
	sourceSettings.flags = IPL_DIRECT_AND_REFLECT;

	IPLSource source;
	iplSourceCreate(m_simulator, &sourceSettings, &source);
	iplSourceAdd(source, m_simulator);

	SteamAudioSource sas = SteamAudioSource(audio, source, this);
	m_simulateDirect.push_back(sas);

	return sas;
}

SteamAudioGeometry SteamAudio::CreateStaticMesh(const Mesh& _mesh)
{
    if (!m_scene)
    {
        log_audio("w~Failed to create a Steam Audio Static Mesh");
        return {};
    }
    
	// Copy the mesh into the phonon format

	auto verts = _mesh.View<vec3>(Mesh::aPosition);
	auto index = _mesh.View<int>(Mesh::aIndexBuffer);

	int triCount = (int)index.size() / 3;

	IPLMaterial material = { 0.13f, 0.20f, 0.24f, 0.05f, 0.015f, 0.002f, 0.001f };

	IPLStaticMeshSettings meshSettings = {};

	meshSettings.numTriangles = triCount;
	meshSettings.numMaterials = 1;

	meshSettings.vertices = (IPLVector3*)verts.data();
	meshSettings.numVertices = (IPLint32)verts.size();

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

	return SteamAudioGeometry(mesh, this);
}

void SteamAudio::SetListenerPosition(vec3 position)
{
	m_listenerPosition = position;
}

void SteamAudio::SaveObjFile()
{
    if (!m_scene)
    {
        log_audio("w~Failed to save obj file");
        return;
    }
    
	iplSceneCommit(m_scene);
	iplSceneSaveOBJ(m_scene, "./steamAudioSceneOut.obj");
}

void SteamAudio::_RemoveSource(_IPLSource_t* source)
{
	auto itr = std::find_if(
		m_simulateDirect.begin(),
		m_simulateDirect.end(),
		[source](const SteamAudioSource& x) { return x._IsSource(source); }
	);

	if (itr == m_simulateDirect.end())
	{
		log_audio("w~Tried to remove a source that does not exist %p", source);
		return;
	}

	iplSourceRemove(source, m_simulator);
	iplSourceRelease(&source);

	m_simulateDirect.erase(itr);
}

void SteamAudio::_RemoveGeometry(_IPLStaticMesh_t* geometry)
{
	iplStaticMeshRemove(geometry, m_scene);
	iplStaticMeshRelease(&geometry);
}

SteamAudioSource::SteamAudioSource()
	: Audio    ()
	, m_source (nullptr)
	, m_world  (nullptr)
{}

SteamAudioSource::SteamAudioSource(Audio audio, _IPLSource_t* source, SteamAudio* world)
	: Audio    (audio)
	, m_source (source)
	, m_world  (world)
{
	m_outputs = mkr<SteamAudioSourceSimulationResults>();
}

void SteamAudioSource::Destroy()
{
	if (!m_source || !m_world)
	{
		log_audio("e~Tried to destroy null SteamAudioSource");
		return;
	}

	m_world->_RemoveSource(m_source);

	m_source = nullptr;
	m_world = nullptr;

	Audio::Destroy();
}

const SteamAudioSourceSimulationResults& SteamAudioSource::GetSimulationResults() const
{
	static SteamAudioSourceSimulationResults _default;

    if (m_outputs)
        return *m_outputs;
    
    return _default;
}

void SteamAudioSource::_UpdateSourceInputs()
{
	// May only need to do this once, and only override the set 3d props functions in audio
	// to set the position?

	// same with dsp params, might be a 1 time thing at the start, but has to be after the
	// audio is started, so its async

	IPLCoordinateSpace3 sourcePos = tocs(GetProps3D().position);
		
	IPLSimulationInputs inputsDirect = {};
	inputsDirect.flags = IPL_SIMULATIONFLAGS_DIRECT;
	inputsDirect.directFlags = IPL_OCCL_TRAN_DIRECT;
	inputsDirect.occlusionType = IPL_OCCLUSIONTYPE_RAYCAST;
	inputsDirect.source = sourcePos;

	iplSourceSetInputs(m_source, IPL_SIMULATIONFLAGS_DIRECT, &inputsDirect);

	IPLSimulationInputs inputsReflection = {};
	inputsReflection.flags = IPL_SIMULATIONFLAGS_REFLECTIONS;
	inputsReflection.source = sourcePos;

	iplSourceSetInputs(m_source, IPL_SIMULATIONFLAGS_REFLECTIONS, &inputsReflection);
}

void SteamAudioSource::_UpdateDSPParams()
{
	// flags for 'simulated-defined' are set in FMOD Studio

	IPLSimulationOutputs outputs;
	iplSourceGetOutputs(m_source, IPL_DIRECT_AND_REFLECT, &outputs);
	
	// Copy the results, mainly for debug

	m_outputs->airAbsorption[0] = outputs.direct.airAbsorption[0];
	m_outputs->airAbsorption[1] = outputs.direct.airAbsorption[1];
	m_outputs->airAbsorption[2] = outputs.direct.airAbsorption[2];

	m_outputs->transmission[0] = outputs.direct.transmission[0];
	m_outputs->transmission[1] = outputs.direct.transmission[1];
	m_outputs->transmission[2] = outputs.direct.transmission[2];

	m_outputs->directivity = outputs.direct.directivity;
	m_outputs->distanceAttenuation = outputs.direct.distanceAttenuation;
	m_outputs->occlusion = outputs.direct.occlusion;
	m_outputs->directFlags = (int)outputs.direct.flags;
	m_outputs->transmissionType = (int)outputs.direct.transmissionType;

	SetParamDSP(0, SpatializeEffect::APPLY_OCCLUSION, 1);
	SetParamDSP(0, SpatializeEffect::SIMULATION_OUTPUTS, &m_source, sizeof(IPLSource*));
}

bool SteamAudioSource::_IsSource(_IPLSource_t* source) const
{
	return m_source == source;
}

SteamAudioGeometry::SteamAudioGeometry()
	: m_mesh  (nullptr)
	, m_world (nullptr)
{}

SteamAudioGeometry::SteamAudioGeometry(_IPLStaticMesh_t* mesh, SteamAudio* world)
	: m_mesh  (mesh)
	, m_world (world)
{}

void SteamAudioGeometry::Destroy()
{
	if (!m_mesh || !m_world)
	{
		log_audio("e~Tried to destroy null SteamAudioGeometry");
		return;
	}

	m_world->_RemoveGeometry(m_mesh);

	m_mesh = nullptr;
	m_world = nullptr;
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
	return IPLVector3{
		v.x, v.y, v.z
	};
}

IPLCoordinateSpace3 tocs(vec3 v)
{
	IPLCoordinateSpace3 space;
	space.origin = tov3(v);
	space.right = IPLVector3{ 1, 0, 0 };
	space.up = IPLVector3{ 0, 1, 0 };
	space.ahead = IPLVector3{ 0, 0, 1 };

	return space;
}
