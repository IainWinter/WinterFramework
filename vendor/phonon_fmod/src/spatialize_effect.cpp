//
// Copyright 2017 Valve Corporation. All rights reserved. Subject to the following license:
// https://valvesoftware.github.io/steam-audio/license.html
//

#include "spatialize_effect.h"

namespace SpatializeEffect {

FMOD_DSP_PARAMETER_DESC gParams[] = {
    { FMOD_DSP_PARAMETER_TYPE_DATA, "SourcePos", "", "Position of the source." },
    { FMOD_DSP_PARAMETER_TYPE_DATA, "OverallGain", "", "Overall gain." },
    { FMOD_DSP_PARAMETER_TYPE_INT, "ApplyDA", "", "Apply distance attenuation." },
    { FMOD_DSP_PARAMETER_TYPE_INT, "ApplyAA", "", "Apply air absorption." },
    { FMOD_DSP_PARAMETER_TYPE_INT, "ApplyDir", "", "Apply directivity." },
    { FMOD_DSP_PARAMETER_TYPE_INT, "ApplyOccl", "", "Apply occlusion." },
    { FMOD_DSP_PARAMETER_TYPE_INT, "ApplyTrans", "", "Apply transmission." },
    { FMOD_DSP_PARAMETER_TYPE_BOOL, "ApplyRefl", "", "Apply reflections." },
    { FMOD_DSP_PARAMETER_TYPE_BOOL, "ApplyPath", "", "Apply pathing." },
    { FMOD_DSP_PARAMETER_TYPE_INT, "Interpolation", "", "HRTF interpolation." },
    { FMOD_DSP_PARAMETER_TYPE_FLOAT, "DistAtt", "", "Distance attenuation." },
    { FMOD_DSP_PARAMETER_TYPE_INT, "DAType", "", "Distance attenuation rolloff type." },
    { FMOD_DSP_PARAMETER_TYPE_FLOAT, "DAMinDist", "", "Distance attenuation min distance." },
    { FMOD_DSP_PARAMETER_TYPE_FLOAT, "DAMaxDist", "", "Distance attenuation max distance." },
    { FMOD_DSP_PARAMETER_TYPE_FLOAT, "AirAbsLow", "", "Air absorption (low frequency)." },
    { FMOD_DSP_PARAMETER_TYPE_FLOAT, "AirAbsMid", "", "Air absorption (mid frequency)." },
    { FMOD_DSP_PARAMETER_TYPE_FLOAT, "AirAbsHigh", "", "Air absorption (high frequency)." },
    { FMOD_DSP_PARAMETER_TYPE_FLOAT, "Directivity", "", "Directivity." },
    { FMOD_DSP_PARAMETER_TYPE_FLOAT, "DipoleWeight", "", "Directivity dipole weight." },
    { FMOD_DSP_PARAMETER_TYPE_FLOAT, "DipolePower", "", "Directivity dipole power." },
    { FMOD_DSP_PARAMETER_TYPE_FLOAT, "Occlusion", "", "Occlusion." },
    { FMOD_DSP_PARAMETER_TYPE_INT, "TransType", "", "Transmission type." },
    { FMOD_DSP_PARAMETER_TYPE_FLOAT, "TransLow", "", "Transmission (low frequency)." },
    { FMOD_DSP_PARAMETER_TYPE_FLOAT, "TransMid", "", "Transmission (mid frequency)." },
    { FMOD_DSP_PARAMETER_TYPE_FLOAT, "TransHigh", "", "Transmission (high frequency)." },
    { FMOD_DSP_PARAMETER_TYPE_FLOAT, "DirMixLevel", "", "Direct mix level." },
    { FMOD_DSP_PARAMETER_TYPE_BOOL, "ReflBinaural", "", "Apply HRTF to reflections." },
    { FMOD_DSP_PARAMETER_TYPE_FLOAT, "ReflMixLevel", "", "Reflections mix level." },
    { FMOD_DSP_PARAMETER_TYPE_BOOL, "PathBinaural", "", "Apply HRTF to pathing." },
    { FMOD_DSP_PARAMETER_TYPE_FLOAT, "PathMixLevel", "", "Pathing mix level." },
    { FMOD_DSP_PARAMETER_TYPE_DATA, "SimOutputs", "", "Simulation outputs." },
    { FMOD_DSP_PARAMETER_TYPE_BOOL, "DirectBinaural", "", "Apply HRTF to direct path." },
    { FMOD_DSP_PARAMETER_TYPE_DATA, "DistRange", "", "Distance attenuation range." },
};

FMOD_DSP_PARAMETER_DESC* gParamsArray[NUM_PARAMS];

const char* gParameterApplyTypeValues[] = {"Off", "Simulation-Defined", "User-Defined"};
const char* gDistanceAttenuationTypeValues[] = {"Off", "Physics-Based", "Curve-Driven"};
const char* gHRTFInterpolationValues[] = {"Nearest", "Bilinear"};
const char* gTransmissionTypeValues[] = {"Frequency Independent", "Frequency Dependent"};
const char* gRolloffTypeValues[] = {"Linear Squared", "Linear", "Inverse", "Inverse Squared", "Custom"};

void initParamDescs()
{
    for (auto i = 0; i < NUM_PARAMS; ++i)
    {
        gParamsArray[i] = &gParams[i];
    }

    gParams[SOURCE_POSITION].datadesc = {FMOD_DSP_PARAMETER_DATA_TYPE_3DATTRIBUTES};
    gParams[OVERALL_GAIN].datadesc = {FMOD_DSP_PARAMETER_DATA_TYPE_OVERALLGAIN};
    gParams[APPLY_DISTANCEATTENUATION].intdesc = {0, 2, 0, false, gDistanceAttenuationTypeValues};
    gParams[APPLY_AIRABSORPTION].intdesc = {0, 2, 0, false, gParameterApplyTypeValues};
    gParams[APPLY_DIRECTIVITY].intdesc = {0, 2, 0, false, gParameterApplyTypeValues};
    gParams[APPLY_OCCLUSION].intdesc = {0, 2, 0, false, gParameterApplyTypeValues};
    gParams[APPLY_TRANSMISSION].intdesc = {0, 2, 0, false, gParameterApplyTypeValues};
    gParams[APPLY_REFLECTIONS].booldesc = {false};
    gParams[APPLY_PATHING].booldesc = {false};
    gParams[HRTF_INTERPOLATION].intdesc = {0, 1, 0, false, gHRTFInterpolationValues};
    gParams[DISTANCEATTENUATION].floatdesc = {0.0f, 1.0f, 1.0f};
    gParams[DISTANCEATTENUATION_ROLLOFFTYPE].intdesc = {0, 4, 2, false, gRolloffTypeValues};
    gParams[DISTANCEATTENUATION_MINDISTANCE].floatdesc = {0.0f, 10000.0f, 1.0f};
    gParams[DISTANCEATTENUATION_MAXDISTANCE].floatdesc = {0.0f, 10000.0f, 20.0f};
    gParams[AIRABSORPTION_LOW].floatdesc = {0.0f, 1.0f, 1.0f};
    gParams[AIRABSORPTION_MID].floatdesc = {0.0f, 1.0f, 1.0f};
    gParams[AIRABSORPTION_HIGH].floatdesc = {0.0f, 1.0f, 1.0f};
    gParams[DIRECTIVITY].floatdesc = {0.0f, 1.0f, 1.0f};
    gParams[DIRECTIVITY_DIPOLEWEIGHT].floatdesc = {0.0f, 1.0f, 1.0f};
    gParams[DIRECTIVITY_DIPOLEPOWER].floatdesc = {1.0f, 4.0f, 1.0f};
    gParams[OCCLUSION].floatdesc = {0.0f, 1.0f, 1.0f};
    gParams[TRANSMISSION_TYPE].intdesc = {0, 1, 0, false, gTransmissionTypeValues};
    gParams[TRANSMISSION_LOW].floatdesc = {0.0f, 1.0f, 1.0f};
    gParams[TRANSMISSION_MID].floatdesc = {0.0f, 1.0f, 1.0f};
    gParams[TRANSMISSION_HIGH].floatdesc = {0.0f, 1.0f, 1.0f};
    gParams[DIRECT_MIXLEVEL].floatdesc = {0.0f, 1.0f, 1.0f};
    gParams[REFLECTIONS_BINAURAL].booldesc = {false};
    gParams[REFLECTIONS_MIXLEVEL].floatdesc = {0.0f, 10.0f, 1.0f};
    gParams[PATHING_BINAURAL].booldesc = {false};
    gParams[PATHING_MIXLEVEL].floatdesc = {0.0f, 10.0f, 1.0f};
    gParams[SIMULATION_OUTPUTS].datadesc = {FMOD_DSP_PARAMETER_DATA_TYPE_USER};
    gParams[DIRECT_BINAURAL].booldesc = {true};
    gParams[DISTANCE_ATTENUATION_RANGE].datadesc = {FMOD_DSP_PARAMETER_DATA_TYPE_ATTENUATION_RANGE};
}

//struct State
//{
//    FMOD_DSP_PARAMETER_3DATTRIBUTES source;
//    FMOD_DSP_PARAMETER_OVERALLGAIN overallGain;
//    ParameterApplyType applyDistanceAttenuation;
//    ParameterApplyType applyAirAbsorption;
//    ParameterApplyType applyDirectivity;
//    ParameterApplyType applyOcclusion;
//    ParameterApplyType applyTransmission;
//    bool applyReflections;
//    bool applyPathing;
//    bool directBinaural;
//    IPLHRTFInterpolation hrtfInterpolation;
//    float distanceAttenuation;
//    FMOD_DSP_PAN_3D_ROLLOFF_TYPE distanceAttenuationRolloffType;
//    float distanceAttenuationMinDistance;
//    float distanceAttenuationMaxDistance;
//    float airAbsorption[3];
//    float directivity;
//    float dipoleWeight;
//    float dipolePower;
//    float occlusion;
//    IPLTransmissionType transmissionType;
//    float transmission[3];
//    float directMixLevel;
//    bool reflectionsBinaural;
//    float reflectionsMixLevel;
//    bool pathingBinaural;
//    float pathingMixLevel;
//    FMOD_DSP_PARAMETER_ATTENUATION_RANGE attenuationRange;
//    std::atomic<bool> attenuationRangeSet;
//
//    IPLSource simulationSource[2];
//    std::atomic<bool> newSimulationSourceWritten;
//
//    float prevDirectMixLevel;
//    float prevReflectionsMixLevel;
//    float prevPathingMixLevel;
//
//    IPLAudioBuffer inBuffer;
//    IPLAudioBuffer outBuffer;
//    IPLAudioBuffer directBuffer;
//    IPLAudioBuffer monoBuffer;
//    IPLAudioBuffer reflectionsBuffer;
//    IPLAudioBuffer reflectionsSpatializedBuffer;
//
//    IPLPanningEffect panningEffect;
//    IPLBinauralEffect binauralEffect;
//    IPLDirectEffect directEffect;
//    IPLReflectionEffect reflectionEffect;
//    IPLPathEffect pathEffect;
//    IPLAmbisonicsDecodeEffect ambisonicsEffect;
//};
//
//enum InitFlags
//{
//    INIT_NONE = 0,
//    INIT_DIRECTAUDIOBUFFERS = 1 << 0,
//    INIT_REFLECTIONAUDIOBUFFERS = 1 << 1,
//    INIT_DIRECTEFFECT = 1 << 2,
//    INIT_BINAURALEFFECT = 1 << 3,
//    INIT_REFLECTIONEFFECT = 1 << 4,
//    INIT_PATHEFFECT = 1 << 5,
//    INIT_AMBISONICSEFFECT = 1 << 6
//};

InitFlags lazyInit(FMOD_DSP_STATE* state,
                   int numChannelsIn,
                   int numChannelsOut)
{
    auto initFlags = INIT_NONE;

    IPLAudioSettings audioSettings;
    state->functions->getsamplerate(state, &audioSettings.samplingRate);
    state->functions->getblocksize(state, reinterpret_cast<unsigned int*>(&audioSettings.frameSize));

    if (!gContext && isRunningInEditor())
    {
        initContextAndDefaultHRTF(audioSettings);
    }

    if (!gContext)
        return initFlags;

    if (!gHRTF[1])
        return initFlags;

    auto effect = reinterpret_cast<State*>(state->plugindata);

    auto status = IPL_STATUS_SUCCESS;

    if (numChannelsOut > 0)
    {
        if (!effect->panningEffect)
        {
            IPLPanningEffectSettings effectSettings{};
            effectSettings.speakerLayout = speakerLayoutForNumChannels(numChannelsOut);

            status = iplPanningEffectCreate(gContext, &audioSettings, &effectSettings, &effect->panningEffect);
        }

        if (status == IPL_STATUS_SUCCESS)
        {
            if (!effect->binauralEffect)
            {
                IPLBinauralEffectSettings effectSettings;
                effectSettings.hrtf = gHRTF[1];

                status = iplBinauralEffectCreate(gContext, &audioSettings, &effectSettings, &effect->binauralEffect);
            }
        }

        if (status == IPL_STATUS_SUCCESS)
            initFlags = static_cast<InitFlags>(initFlags | INIT_BINAURALEFFECT);
    }

    if (numChannelsIn > 0)
    {
        status = IPL_STATUS_SUCCESS;
        if (!effect->directEffect)
        {
            IPLDirectEffectSettings effectSettings;
            effectSettings.numChannels = numChannelsIn;

            status = iplDirectEffectCreate(gContext, &audioSettings, &effectSettings, &effect->directEffect);
        }

        if (status == IPL_STATUS_SUCCESS)
            initFlags = static_cast<InitFlags>(initFlags | INIT_DIRECTEFFECT);
    }

    if (effect->applyReflections && gIsSimulationSettingsValid)
    {
        status = IPL_STATUS_SUCCESS;

        if (!effect->reflectionEffect)
        {
            IPLReflectionEffectSettings effectSettings;
            effectSettings.type = gSimulationSettings.reflectionType;
            effectSettings.numChannels = numChannelsForOrder(gSimulationSettings.maxOrder);
            effectSettings.irSize = numSamplesForDuration(gSimulationSettings.maxDuration, audioSettings.samplingRate);

            status = iplReflectionEffectCreate(gContext, &audioSettings, &effectSettings, &effect->reflectionEffect);
        }

        if (status == IPL_STATUS_SUCCESS)
            initFlags = static_cast<InitFlags>(initFlags | INIT_REFLECTIONEFFECT);
    }

    if (effect->applyPathing && gIsSimulationSettingsValid)
    {
        status = IPL_STATUS_SUCCESS;

        if (!effect->pathEffect)
        {
            IPLPathEffectSettings effectSettings;
            effectSettings.maxOrder = gSimulationSettings.maxOrder;

            status = iplPathEffectCreate(gContext, &audioSettings, &effectSettings, &effect->pathEffect);
        }

        if (status == IPL_STATUS_SUCCESS)
            initFlags = static_cast<InitFlags>(initFlags | INIT_PATHEFFECT);
    }

    if (numChannelsOut > 0 && gIsSimulationSettingsValid)
    {
        status = IPL_STATUS_SUCCESS;

        if (!effect->ambisonicsEffect)
        {
            IPLAmbisonicsDecodeEffectSettings effectSettings;
            effectSettings.speakerLayout = speakerLayoutForNumChannels(numChannelsOut);
            effectSettings.hrtf = gHRTF[1];
            effectSettings.maxOrder = gSimulationSettings.maxOrder;

            status = iplAmbisonicsDecodeEffectCreate(gContext, &audioSettings, &effectSettings, &effect->ambisonicsEffect);
        }

        if (status == IPL_STATUS_SUCCESS)
            initFlags = static_cast<InitFlags>(initFlags | INIT_AMBISONICSEFFECT);
    }

    if (numChannelsIn > 0 && numChannelsOut > 0)
    {
        if (!effect->inBuffer.data)
            iplAudioBufferAllocate(gContext, numChannelsIn, audioSettings.frameSize, &effect->inBuffer);

        if (!effect->outBuffer.data)
            iplAudioBufferAllocate(gContext, numChannelsOut, audioSettings.frameSize, &effect->outBuffer);

        if (!effect->directBuffer.data)
            iplAudioBufferAllocate(gContext, numChannelsIn, audioSettings.frameSize, &effect->directBuffer);

        if (!effect->monoBuffer.data)
            iplAudioBufferAllocate(gContext, 1, audioSettings.frameSize, &effect->monoBuffer);

        initFlags = static_cast<InitFlags>(initFlags | INIT_DIRECTAUDIOBUFFERS);

        if ((effect->applyReflections || effect->applyPathing) && gIsSimulationSettingsValid)
        {
            auto numAmbisonicChannels = numChannelsForOrder(gSimulationSettings.maxOrder);

            if (!effect->reflectionsBuffer.data)
                iplAudioBufferAllocate(gContext, numAmbisonicChannels, audioSettings.frameSize, &effect->reflectionsBuffer);

            if (!effect->reflectionsSpatializedBuffer.data)
                iplAudioBufferAllocate(gContext, numChannelsOut, audioSettings.frameSize, &effect->reflectionsSpatializedBuffer);

            initFlags = static_cast<InitFlags>(initFlags | INIT_REFLECTIONAUDIOBUFFERS);
        }
    }

    return initFlags;
}

void reset(FMOD_DSP_STATE* state)
{
    auto effect = reinterpret_cast<State*>(state->plugindata);
    if (!effect)
        return;

    effect->applyDistanceAttenuation = PARAMETER_USERDEFINED;
    effect->applyAirAbsorption = PARAMETER_DISABLE;
    effect->applyDirectivity = PARAMETER_DISABLE;
    effect->applyOcclusion = PARAMETER_DISABLE;
    effect->applyTransmission = PARAMETER_DISABLE;
    effect->applyReflections = false;
    effect->applyPathing = false;
    effect->directBinaural = true;
    effect->hrtfInterpolation = IPL_HRTFINTERPOLATION_NEAREST;
    effect->distanceAttenuation = 1.0f;
    effect->distanceAttenuationRolloffType = FMOD_DSP_PAN_3D_ROLLOFF_INVERSE;
    effect->distanceAttenuationMinDistance = 1.0f;
    effect->distanceAttenuationMaxDistance = 20.0f;
    effect->airAbsorption[0] = 1.0f;
    effect->airAbsorption[1] = 1.0f;
    effect->airAbsorption[2] = 1.0f;
    effect->directivity = 1.0f;
    effect->dipoleWeight = 0.0f;
    effect->dipolePower = 1.0f;
    effect->occlusion = 1.0f;
    effect->transmissionType = IPL_TRANSMISSIONTYPE_FREQINDEPENDENT;
    effect->transmission[0] = 1.0f;
    effect->transmission[1] = 1.0f;
    effect->transmission[2] = 1.0f;
    effect->reflectionsBinaural = false;
    effect->reflectionsMixLevel = 1.0f;
    effect->pathingBinaural = false;
    effect->pathingMixLevel = 1.0f;
    effect->attenuationRange.min = 1.0f;
    effect->attenuationRange.max = 20.0f;
    effect->attenuationRangeSet = false;

    effect->simulationSource[0] = nullptr;
    effect->simulationSource[1] = nullptr;
    effect->newSimulationSourceWritten = false;

    effect->prevDirectMixLevel = 1.0f;
    effect->prevReflectionsMixLevel = 0.0f;
    effect->prevPathingMixLevel = 0.0f;
}

FMOD_RESULT F_CALL create(FMOD_DSP_STATE* state)
{
    state->plugindata = new State();
    reset(state);
    lazyInit(state, 0, 0);
    return FMOD_OK;
}

FMOD_RESULT F_CALL release(FMOD_DSP_STATE* state)
{
    auto effect = reinterpret_cast<State*>(state->plugindata);

    iplAudioBufferFree(gContext, &effect->inBuffer);
    iplAudioBufferFree(gContext, &effect->outBuffer);
    iplAudioBufferFree(gContext, &effect->directBuffer);
    iplAudioBufferFree(gContext, &effect->monoBuffer);
    iplAudioBufferFree(gContext, &effect->reflectionsBuffer);
    iplAudioBufferFree(gContext, &effect->reflectionsSpatializedBuffer);

    iplPanningEffectRelease(&effect->panningEffect);
    iplBinauralEffectRelease(&effect->binauralEffect);
    iplDirectEffectRelease(&effect->directEffect);
    iplReflectionEffectRelease(&effect->reflectionEffect);
    iplPathEffectRelease(&effect->pathEffect);
    iplAmbisonicsDecodeEffectRelease(&effect->ambisonicsEffect);

    effect->newSimulationSourceWritten = false;
    iplSourceRelease(&effect->simulationSource[0]);
    iplSourceRelease(&effect->simulationSource[1]);

    delete state->plugindata;

    return FMOD_OK;
}

FMOD_RESULT F_CALL getBool(FMOD_DSP_STATE* state,
                           int index,
                           FMOD_BOOL* value,
                           char*)
{
    auto effect = reinterpret_cast<State*>(state->plugindata);

    switch (index)
    {
    case DIRECT_BINAURAL:
        *value = effect->directBinaural;
        break;
    case APPLY_REFLECTIONS:
        *value = effect->applyReflections;
        break;
    case APPLY_PATHING:
        *value = effect->applyPathing;
        break;
    case REFLECTIONS_BINAURAL:
        *value = effect->reflectionsBinaural;
        break;
    case PATHING_BINAURAL:
        *value = effect->pathingBinaural;
        break;
    default:
        return FMOD_ERR_INVALID_PARAM;
    }

    return FMOD_OK;
}

FMOD_RESULT F_CALL getInt(FMOD_DSP_STATE* state,
                          int index,
                          int* value,
                          char*)
{
    auto effect = reinterpret_cast<State*>(state->plugindata);

    switch (index)
    {
    case APPLY_DISTANCEATTENUATION:
        *value = static_cast<int>(effect->applyDistanceAttenuation);
        break;
    case APPLY_AIRABSORPTION:
        *value = static_cast<int>(effect->applyAirAbsorption);
        break;
    case APPLY_DIRECTIVITY:
        *value = static_cast<int>(effect->applyDirectivity);
        break;
    case APPLY_OCCLUSION:
        *value = static_cast<int>(effect->applyOcclusion);
        break;
    case APPLY_TRANSMISSION:
        *value = static_cast<int>(effect->applyTransmission);
        break;
    case HRTF_INTERPOLATION:
        *value = static_cast<int>(effect->hrtfInterpolation);
        break;
    case DISTANCEATTENUATION_ROLLOFFTYPE:
        *value = static_cast<int>(effect->distanceAttenuationRolloffType);
        break;
    case TRANSMISSION_TYPE:
        *value = static_cast<int>(effect->transmissionType);
        break;
    default:
        return FMOD_ERR_INVALID_PARAM;
    }

    return FMOD_OK;
}

FMOD_RESULT F_CALL getFloat(FMOD_DSP_STATE* state,
                            int index,
                            float* value,
                            char*)
{
    auto effect = reinterpret_cast<State*>(state->plugindata);

    switch (index)
    {
    case DISTANCEATTENUATION:
        *value = effect->distanceAttenuation;
        break;
    case DISTANCEATTENUATION_MINDISTANCE:
        *value = effect->distanceAttenuationMinDistance;
        break;
    case DISTANCEATTENUATION_MAXDISTANCE:
        *value = effect->distanceAttenuationMaxDistance;
        break;
    case AIRABSORPTION_LOW:
        *value = effect->airAbsorption[0];
        break;
    case AIRABSORPTION_MID:
        *value = effect->airAbsorption[1];
        break;
    case AIRABSORPTION_HIGH:
        *value = effect->airAbsorption[2];
        break;
    case DIRECTIVITY:
        *value = effect->directivity;
        break;
    case DIRECTIVITY_DIPOLEWEIGHT:
        *value = effect->dipoleWeight;
        break;
    case DIRECTIVITY_DIPOLEPOWER:
        *value = effect->dipolePower;
        break;
    case OCCLUSION:
        *value = effect->occlusion;
        break;
    case TRANSMISSION_LOW:
        *value = effect->transmission[0];
        break;
    case TRANSMISSION_MID:
        *value = effect->transmission[1];
        break;
    case TRANSMISSION_HIGH:
        *value = effect->transmission[2];
        break;
    case DIRECT_MIXLEVEL:
        *value = effect->directMixLevel;
        break;
    case REFLECTIONS_MIXLEVEL:
        *value = effect->reflectionsMixLevel;
        break;
    case PATHING_MIXLEVEL:
        *value = effect->pathingMixLevel;
        break;
    default:
        return FMOD_ERR_INVALID_PARAM;
    }

    return FMOD_OK;
}

FMOD_RESULT F_CALL getData(FMOD_DSP_STATE* state,
                           int index,
                           void** value,
                           unsigned int* length,
                           char*)
{
    auto effect = reinterpret_cast<State*>(state->plugindata);

    switch (index)
    {
    case OVERALL_GAIN:
        *value = static_cast<void*>(&effect->overallGain);
        *length = sizeof(effect->overallGain);
        break;
    default:
        return FMOD_ERR_INVALID_PARAM;
    }

    return FMOD_OK;
}

FMOD_RESULT F_CALL setBool(FMOD_DSP_STATE* state,
                           int index,
                           FMOD_BOOL value)
{
    auto effect = reinterpret_cast<State*>(state->plugindata);

    switch (index)
    {
    case DIRECT_BINAURAL:
        effect->directBinaural = value;
        break;
    case APPLY_REFLECTIONS:
        effect->applyReflections = value;
        break;
    case APPLY_PATHING:
        effect->applyPathing = value;
        break;
    case REFLECTIONS_BINAURAL:
        effect->reflectionsBinaural = value;
        break;
    case PATHING_BINAURAL:
        effect->pathingBinaural = value;
        break;
    default:
        return FMOD_ERR_INVALID_PARAM;
    }

    return FMOD_OK;
}

FMOD_RESULT F_CALL setInt(FMOD_DSP_STATE* state,
                          int index,
                          int value)
{
    auto effect = reinterpret_cast<State*>(state->plugindata);

    switch (index)
    {
    case APPLY_DISTANCEATTENUATION:
        effect->applyDistanceAttenuation = static_cast<ParameterApplyType>(value);
        break;
    case APPLY_AIRABSORPTION:
        effect->applyAirAbsorption = static_cast<ParameterApplyType>(value);
        break;
    case APPLY_DIRECTIVITY:
        effect->applyDirectivity = static_cast<ParameterApplyType>(value);
        break;
    case APPLY_OCCLUSION:
        effect->applyOcclusion = static_cast<ParameterApplyType>(value);
        break;
    case APPLY_TRANSMISSION:
        effect->applyTransmission = static_cast<ParameterApplyType>(value);
        break;
    case HRTF_INTERPOLATION:
        effect->hrtfInterpolation = static_cast<IPLHRTFInterpolation>(value);
        break;
    case DISTANCEATTENUATION_ROLLOFFTYPE:
        effect->distanceAttenuationRolloffType = static_cast<FMOD_DSP_PAN_3D_ROLLOFF_TYPE>(value);
        break;
    case TRANSMISSION_TYPE:
        effect->transmissionType = static_cast<IPLTransmissionType>(value);
        break;
    default:
        return FMOD_ERR_INVALID_PARAM;
    }

    return FMOD_OK;
}

FMOD_RESULT F_CALL setFloat(FMOD_DSP_STATE* state,
                            int index,
                            float value)
{
    auto effect = reinterpret_cast<State*>(state->plugindata);

    switch (index)
    {
    case DISTANCEATTENUATION:
        effect->distanceAttenuation = value;
        break;
    case DISTANCEATTENUATION_MINDISTANCE:
        effect->distanceAttenuationMinDistance = value;
        break;
    case DISTANCEATTENUATION_MAXDISTANCE:
        effect->distanceAttenuationMaxDistance = value;
        break;
    case AIRABSORPTION_LOW:
        effect->airAbsorption[0] = value;
        break;
    case AIRABSORPTION_MID:
        effect->airAbsorption[1] = value;
        break;
    case AIRABSORPTION_HIGH:
        effect->airAbsorption[2] = value;
        break;
    case DIRECTIVITY:
        effect->directivity = value;
        break;
    case DIRECTIVITY_DIPOLEWEIGHT:
        effect->dipoleWeight = value;
        break;
    case DIRECTIVITY_DIPOLEPOWER:
        effect->dipolePower = value;
        break;
    case OCCLUSION:
        effect->occlusion = value;
        break;
    case TRANSMISSION_LOW:
        effect->transmission[0] = value;
        break;
    case TRANSMISSION_MID:
        effect->transmission[1] = value;
        break;
    case TRANSMISSION_HIGH:
        effect->transmission[2] = value;
        break;
    case DIRECT_MIXLEVEL:
        effect->directMixLevel = value;
        break;
    case REFLECTIONS_MIXLEVEL:
        effect->reflectionsMixLevel = value;
        break;
    case PATHING_MIXLEVEL:
        effect->pathingMixLevel = value;
        break;
    default:
        return FMOD_ERR_INVALID_PARAM;
    }

    return FMOD_OK;
}

void setSource(FMOD_DSP_STATE* state,
               IPLSource source)
{
    auto effect = reinterpret_cast<State*>(state->plugindata);

    if (source == effect->simulationSource[1])
        return;

    if (!effect->newSimulationSourceWritten)
    {
        iplSourceRelease(&effect->simulationSource[1]);
        effect->simulationSource[1] = iplSourceRetain(source);

        effect->newSimulationSourceWritten = true;
    }
}

FMOD_RESULT F_CALL setData(FMOD_DSP_STATE* state,
                           int index,
                           void* value,
                           unsigned int length)
{
    auto effect = reinterpret_cast<State*>(state->plugindata);

    IPLSource simulationSource = nullptr;

    switch (index)
    {
    case SOURCE_POSITION:
        memcpy(&effect->source, value, length);
        break;
    case SIMULATION_OUTPUTS:
        memcpy(&simulationSource, value, length);
        setSource(state, simulationSource);
        break;
    case DISTANCE_ATTENUATION_RANGE:
        memcpy(&effect->attenuationRange, value, length);
        effect->attenuationRangeSet = true;
        break;
    default:
        return FMOD_ERR_INVALID_PARAM;
    }

    return FMOD_OK;
}

IPLDirectEffectParams getDirectParams(FMOD_DSP_STATE* state,
                                      IPLCoordinateSpace3 source,
                                      IPLCoordinateSpace3 listener,
                                      bool updatingOverallGain)
{
    auto effect = reinterpret_cast<State*>(state->plugindata);

    auto hasSource = false;
    IPLSimulationOutputs simulationOutputs{};
    if (effect->simulationSource[0])
    {
        iplSourceGetOutputs(effect->simulationSource[0], IPL_SIMULATIONFLAGS_DIRECT, &simulationOutputs);
        hasSource = true;
    }

    auto params = simulationOutputs.direct;
    params.transmissionType = effect->transmissionType;

    params.flags = static_cast<IPLDirectEffectFlags>(0);
    if (effect->applyDistanceAttenuation == PARAMETER_DISABLE)
    {
        params.distanceAttenuation = 1.0f;
    }
    else
    {
        params.flags = static_cast<IPLDirectEffectFlags>(params.flags | IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION);
        if (effect->applyDistanceAttenuation == PARAMETER_USERDEFINED)
        {
            auto minDistance = effect->attenuationRangeSet ? effect->attenuationRange.min : effect->distanceAttenuationMinDistance;
            auto maxDistance = effect->attenuationRangeSet ? effect->attenuationRange.max : effect->distanceAttenuationMaxDistance;

            state->functions->pan->getrolloffgain(state, effect->distanceAttenuationRolloffType,
                                                  distance(source.origin, listener.origin),
                                                  minDistance, maxDistance, &params.distanceAttenuation);
        }
        else
        {
            IPLDistanceAttenuationModel distanceAttenuationModel{};
            distanceAttenuationModel.type = IPL_DISTANCEATTENUATIONTYPE_DEFAULT;

            params.distanceAttenuation = iplDistanceAttenuationCalculate(gContext, source.origin, listener.origin, &distanceAttenuationModel);
        }
    }

    if (effect->applyAirAbsorption == PARAMETER_DISABLE)
    {
        params.airAbsorption[0] = 1.0f;
        params.airAbsorption[1] = 1.0f;
        params.airAbsorption[2] = 1.0f;
    }
    else
    {
        params.flags = static_cast<IPLDirectEffectFlags>(params.flags | IPL_DIRECTEFFECTFLAGS_APPLYAIRABSORPTION);
        if (effect->applyAirAbsorption == PARAMETER_USERDEFINED)
        {
            memcpy(params.airAbsorption, effect->airAbsorption, 3 * sizeof(float));
        }
        else
        {
            IPLAirAbsorptionModel airAbsorptionModel{};
            airAbsorptionModel.type = IPL_AIRABSORPTIONTYPE_DEFAULT;

            iplAirAbsorptionCalculate(gContext, source.origin, listener.origin, &airAbsorptionModel, params.airAbsorption);
        }
    }

    if (effect->applyDirectivity == PARAMETER_DISABLE)
    {
        params.directivity = 1.0f;
    }
    else
    {
        params.flags = static_cast<IPLDirectEffectFlags>(params.flags | IPL_DIRECTEFFECTFLAGS_APPLYDIRECTIVITY);
        if (effect->applyDirectivity == PARAMETER_USERDEFINED)
        {
            params.directivity = effect->directivity;
        }
        else
        {
            IPLDirectivity directivity{};
            directivity.dipoleWeight = effect->dipoleWeight;
            directivity.dipolePower = effect->dipolePower;

            params.directivity = iplDirectivityCalculate(gContext, source, listener.origin, &directivity);
        }
    }

    if (effect->applyOcclusion == PARAMETER_DISABLE)
    {
        params.occlusion = 1.0f;
    }
    else
    {
        params.flags = static_cast<IPLDirectEffectFlags>(params.flags | IPL_DIRECTEFFECTFLAGS_APPLYOCCLUSION);

        if (effect->applyOcclusion == PARAMETER_USERDEFINED)
        {
            params.occlusion = effect->occlusion;
        }
        else
        {
            if (updatingOverallGain && !hasSource)
            {
                params.occlusion = 1.0f;
            }
        }
    }

    if (effect->applyTransmission == PARAMETER_DISABLE)
    {
        params.transmission[0] = 1.0f;
        params.transmission[1] = 1.0f;
        params.transmission[2] = 1.0f;
    }
    else
    {
        if (effect->applyTransmission == PARAMETER_USERDEFINED || hasSource)
            params.flags = static_cast<IPLDirectEffectFlags>(params.flags | IPL_DIRECTEFFECTFLAGS_APPLYTRANSMISSION);

        if (effect->applyTransmission == PARAMETER_USERDEFINED)
        {
            memcpy(params.transmission, effect->transmission, 3 * sizeof(float));
        }
        else
        {
            if (updatingOverallGain && !hasSource)
            {
                params.transmission[0] = 1.0f;
                params.transmission[1] = 1.0f;
                params.transmission[2] = 1.0f;
            }
        }
    }

    return params;
}

void updateOverallGain(FMOD_DSP_STATE* state,
                       IPLCoordinateSpace3 source,
                       IPLCoordinateSpace3 listener)
{
    auto effect = reinterpret_cast<State*>(state->plugindata);
    auto directParams = getDirectParams(state, source, listener, true);

    auto level = effect->directMixLevel;
    level *= directParams.distanceAttenuation;
    level *= *std::max_element(directParams.airAbsorption, directParams.airAbsorption + 3);
    level *= directParams.directivity;
    level *= (directParams.occlusion + (1.0f - directParams.occlusion) * *std::max_element(directParams.transmission, directParams.transmission + 3));

    if (effect->applyReflections)
        level += effect->reflectionsMixLevel;

    if (effect->applyPathing)
        level += effect->pathingMixLevel;

    effect->overallGain.linear_gain = std::min(1.0f, level);
    effect->overallGain.linear_gain_additive = 0.0f; // this is 0, as this is a volume Fmod sends to "behind the scenes" cooperative plugins, and we don't currently have that
}

FMOD_RESULT F_CALL process(FMOD_DSP_STATE* state,
                           unsigned int length,
                           const FMOD_DSP_BUFFER_ARRAY* inBuffers,
                           FMOD_DSP_BUFFER_ARRAY* outBuffers,
                           FMOD_BOOL inputsIdle,
                           FMOD_DSP_PROCESS_OPERATION operation)
{
    auto effect = reinterpret_cast<State*>(state->plugindata);

    auto sourceCoordinates = calcCoordinates(effect->source.absolute);
    auto listenerCoordinates = calcListenerCoordinates(state);

    if (operation == FMOD_DSP_PROCESS_QUERY)
    {
        if (outBuffers)
        {
            outBuffers->speakermode = FMOD_SPEAKERMODE_STEREO;
            outBuffers->buffernumchannels[0] = 2;
            outBuffers->bufferchannelmask[0] = 0;
        }

        if (inputsIdle)
        {
            // if the sound is idle, we still need to check the expected overall gain to help manage
            // channel counts. updateOverallGain won't do any processing - just determine how loud
            // the sound would be (according to attenuation, etc) if it were playing.
            // Note: the SteamAudio Unity plugin now calculates iplGetDirectSoundPath so this is even lighter
            updateOverallGain(state, sourceCoordinates, listenerCoordinates);
            return FMOD_ERR_DSP_DONTPROCESS;
        }
    }
    else if (operation == FMOD_DSP_PROCESS_PERFORM)
    {
        updateOverallGain(state, sourceCoordinates, listenerCoordinates);

        auto samplingRate = 0;
        auto frameSize = 0u;
        state->functions->getsamplerate(state, &samplingRate);
        state->functions->getblocksize(state, &frameSize);

        auto numChannelsIn = inBuffers->buffernumchannels[0];
        auto numChannelsOut = outBuffers->buffernumchannels[0];
        auto in = inBuffers->buffers[0];
        auto out = outBuffers->buffers[0];

        // Start by clearing the output buffer.
        memset(out, 0, numChannelsOut * frameSize * sizeof(float));

        // Make sure that audio processing state has been initialized. If initialization fails, stop and emit silence.
        // TODO: if nothing is initialized, do some fallback processing (passthrough, panning, or something like that).
        auto initFlags = lazyInit(state, numChannelsIn, numChannelsOut);
        if (!(initFlags & INIT_DIRECTAUDIOBUFFERS) || !(initFlags & INIT_BINAURALEFFECT) || !(initFlags & INIT_DIRECTEFFECT))
            return FMOD_ERR_DSP_SILENCE;

        if (gNewHRTFWritten)
        {
            iplHRTFRelease(&gHRTF[0]);
            gHRTF[0] = iplHRTFRetain(gHRTF[1]);

            gNewHRTFWritten = false;
        }

        if (effect->newSimulationSourceWritten)
        {
            iplSourceRelease(&effect->simulationSource[0]);
            effect->simulationSource[0] = iplSourceRetain(effect->simulationSource[1]);

            effect->newSimulationSourceWritten = false;
        }

        auto sourcePosition = sourceCoordinates.origin;
        auto direction = iplCalculateRelativeDirection(gContext, sourcePosition, listenerCoordinates.origin, listenerCoordinates.ahead, listenerCoordinates.up);

        iplAudioBufferDeinterleave(gContext, in, &effect->inBuffer);

        IPLDirectEffectParams directParams = getDirectParams(state, sourceCoordinates, listenerCoordinates, false);

        iplDirectEffectApply(effect->directEffect, &directParams, &effect->inBuffer, &effect->directBuffer);

        if (effect->directBinaural)
        {
            IPLBinauralEffectParams binauralParams{};
            binauralParams.direction = direction;
            binauralParams.interpolation = effect->hrtfInterpolation;
            binauralParams.spatialBlend = 1.0f;
            binauralParams.hrtf = gHRTF[0];

            iplBinauralEffectApply(effect->binauralEffect, &binauralParams, &effect->directBuffer, &effect->outBuffer);
        }
        else
        {
            iplAudioBufferDownmix(gContext, &effect->directBuffer, &effect->monoBuffer);

            IPLPanningEffectParams panningParams{};
            panningParams.direction = direction;

            iplPanningEffectApply(effect->panningEffect, &panningParams, &effect->monoBuffer, &effect->outBuffer);
        }

        for (auto i = 0; i < numChannelsOut; ++i)
        {
            applyVolumeRamp(effect->prevDirectMixLevel, effect->directMixLevel, frameSize, effect->outBuffer.data[i]);
        }
        effect->prevDirectMixLevel = effect->directMixLevel;

        if (effect->simulationSource[0])
        {
            IPLSimulationOutputs simulationOutputs{};
            iplSourceGetOutputs(effect->simulationSource[0], static_cast<IPLSimulationFlags>(IPL_SIMULATIONFLAGS_REFLECTIONS | IPL_SIMULATIONFLAGS_PATHING), &simulationOutputs);

            if (effect->applyReflections &&
                (initFlags & INIT_REFLECTIONAUDIOBUFFERS) && (initFlags & INIT_REFLECTIONEFFECT) && (initFlags && INIT_AMBISONICSEFFECT))
            {
                iplAudioBufferDownmix(gContext, &effect->inBuffer, &effect->monoBuffer);

                applyVolumeRamp(effect->prevReflectionsMixLevel, effect->reflectionsMixLevel, frameSize, effect->monoBuffer.data[0]);
                effect->prevReflectionsMixLevel = effect->reflectionsMixLevel;

                IPLReflectionEffectParams reflectionParams = simulationOutputs.reflections;
                reflectionParams.type = gSimulationSettings.reflectionType;
                reflectionParams.numChannels = numChannelsForOrder(gSimulationSettings.maxOrder);
                reflectionParams.irSize = numSamplesForDuration(gSimulationSettings.maxDuration, static_cast<int>(samplingRate));
                reflectionParams.tanDevice = gSimulationSettings.tanDevice;

                if (gNewReflectionMixerWritten)
                {
                    iplReflectionMixerRelease(&gReflectionMixer[0]);
                    gReflectionMixer[0] = iplReflectionMixerRetain(gReflectionMixer[1]);

                    gNewReflectionMixerWritten = false;
                }

                if (reflectionParams.irSize == 96000)
                {
                    printf("From FMOD\n");
                }

                iplReflectionEffectApply(effect->reflectionEffect, &reflectionParams, &effect->monoBuffer, &effect->reflectionsBuffer, gReflectionMixer[0]);

                if (gSimulationSettings.reflectionType != IPL_REFLECTIONEFFECTTYPE_TAN && !gReflectionMixer[0])
                {
                    IPLAmbisonicsDecodeEffectParams ambisonicsParams;
                    ambisonicsParams.order = gSimulationSettings.maxOrder;
                    ambisonicsParams.hrtf = gHRTF[0];
                    ambisonicsParams.orientation = listenerCoordinates;
                    ambisonicsParams.binaural = (effect->reflectionsBinaural) ? IPL_TRUE : IPL_FALSE;

                    iplAmbisonicsDecodeEffectApply(effect->ambisonicsEffect, &ambisonicsParams, &effect->reflectionsBuffer, &effect->reflectionsSpatializedBuffer);

                    iplAudioBufferMix(gContext, &effect->reflectionsSpatializedBuffer, &effect->outBuffer);
                }
            }

            if (effect->applyPathing &&
                (initFlags & INIT_REFLECTIONAUDIOBUFFERS) && (initFlags & INIT_PATHEFFECT) && (initFlags && INIT_AMBISONICSEFFECT))
            {
                iplAudioBufferDownmix(gContext, &effect->inBuffer, &effect->monoBuffer);

                applyVolumeRamp(effect->prevPathingMixLevel, effect->pathingMixLevel, frameSize, effect->monoBuffer.data[0]);
                effect->prevPathingMixLevel = effect->pathingMixLevel;

                IPLPathEffectParams pathParams = simulationOutputs.pathing;
                pathParams.order = gSimulationSettings.maxOrder;

                iplPathEffectApply(effect->pathEffect, &pathParams, &effect->monoBuffer, &effect->reflectionsBuffer);

                IPLAmbisonicsDecodeEffectParams ambisonicsParams;
                ambisonicsParams.order = gSimulationSettings.maxOrder;
                ambisonicsParams.hrtf = gHRTF[0];
                ambisonicsParams.orientation = listenerCoordinates;
                ambisonicsParams.binaural = (effect->pathingBinaural) ? IPL_TRUE : IPL_FALSE;

                iplAmbisonicsDecodeEffectApply(effect->ambisonicsEffect, &ambisonicsParams, &effect->reflectionsBuffer, &effect->reflectionsSpatializedBuffer);

                iplAudioBufferMix(gContext, &effect->reflectionsSpatializedBuffer, &effect->outBuffer);
            }
        }
        
        iplAudioBufferInterleave(gContext, &effect->outBuffer, out);
    }

    return FMOD_OK;
}

}

FMOD_DSP_DESCRIPTION gSpatializeEffect =
{
    FMOD_PLUGIN_SDK_VERSION,
    "Steam Audio Spatializer",
    STEAMAUDIO_FMOD_VERSION,
    1,
    1,
    SpatializeEffect::create,
    SpatializeEffect::release,
    nullptr,
    nullptr,
    SpatializeEffect::process,
    nullptr,
    SpatializeEffect::NUM_PARAMS,
    SpatializeEffect::gParamsArray,
    SpatializeEffect::setFloat,
    SpatializeEffect::setInt,
    SpatializeEffect::setBool,
    SpatializeEffect::setData,
    SpatializeEffect::getFloat,
    SpatializeEffect::getInt,
    SpatializeEffect::getBool,
    SpatializeEffect::getData,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};
