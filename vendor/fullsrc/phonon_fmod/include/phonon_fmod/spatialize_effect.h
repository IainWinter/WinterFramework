#pragma once

//
// Copyright 2017 Valve Corporation. All rights reserved. Subject to the following license:
// https://valvesoftware.github.io/steam-audio/license.html
//

#include <assert.h>
#include <string.h>

#include <algorithm>
#include <atomic>

#include "steamaudio_fmod.h"
#include "dsp_names.h"

namespace SpatializeEffect {

    struct State
    {
        FMOD_DSP_PARAMETER_3DATTRIBUTES source;
        FMOD_DSP_PARAMETER_OVERALLGAIN overallGain;
        ParameterApplyType applyDistanceAttenuation;
        ParameterApplyType applyAirAbsorption;
        ParameterApplyType applyDirectivity;
        ParameterApplyType applyOcclusion;
        ParameterApplyType applyTransmission;
        bool applyReflections;
        bool applyPathing;
        bool directBinaural;
        IPLHRTFInterpolation hrtfInterpolation;
        float distanceAttenuation;
        FMOD_DSP_PAN_3D_ROLLOFF_TYPE distanceAttenuationRolloffType;
        float distanceAttenuationMinDistance;
        float distanceAttenuationMaxDistance;
        float airAbsorption[3];
        float directivity;
        float dipoleWeight;
        float dipolePower;
        float occlusion;
        IPLTransmissionType transmissionType;
        float transmission[3];
        float directMixLevel;
        bool reflectionsBinaural;
        float reflectionsMixLevel;
        bool pathingBinaural;
        float pathingMixLevel;
        FMOD_DSP_PARAMETER_ATTENUATION_RANGE attenuationRange;
        std::atomic<bool> attenuationRangeSet;

        IPLSource simulationSource[2];
        std::atomic<bool> newSimulationSourceWritten;

        float prevDirectMixLevel;
        float prevReflectionsMixLevel;
        float prevPathingMixLevel;

        IPLAudioBuffer inBuffer;
        IPLAudioBuffer outBuffer;
        IPLAudioBuffer directBuffer;
        IPLAudioBuffer monoBuffer;
        IPLAudioBuffer reflectionsBuffer;
        IPLAudioBuffer reflectionsSpatializedBuffer;

        IPLPanningEffect panningEffect;
        IPLBinauralEffect binauralEffect;
        IPLDirectEffect directEffect;
        IPLReflectionEffect reflectionEffect;
        IPLPathEffect pathEffect;
        IPLAmbisonicsDecodeEffect ambisonicsEffect;
    };

    enum InitFlags
    {
        INIT_NONE = 0,
        INIT_DIRECTAUDIOBUFFERS = 1 << 0,
        INIT_REFLECTIONAUDIOBUFFERS = 1 << 1,
        INIT_DIRECTEFFECT = 1 << 2,
        INIT_BINAURALEFFECT = 1 << 3,
        INIT_REFLECTIONEFFECT = 1 << 4,
        INIT_PATHEFFECT = 1 << 5,
        INIT_AMBISONICSEFFECT = 1 << 6
    };

    InitFlags lazyInit(FMOD_DSP_STATE* state,
                       int numChannelsIn,
                       int numChannelsOut);

    void reset(FMOD_DSP_STATE* state);

    FMOD_RESULT F_EXPORT F_CALL create(FMOD_DSP_STATE* state);

    FMOD_RESULT F_CALL release(FMOD_DSP_STATE* state);

    FMOD_RESULT F_CALL getBool(FMOD_DSP_STATE* state,
                               int index,
                               FMOD_BOOL* value,
                               char*);

    FMOD_RESULT F_CALL getInt(FMOD_DSP_STATE* state,
                              int index,
                              int* value,
                              char*);

    FMOD_RESULT F_CALL getFloat(FMOD_DSP_STATE* state,
                                int index,
                                float* value,
                                char*);
    FMOD_RESULT F_CALL getData(FMOD_DSP_STATE* state,
                               int index,
                               void** value,
                               unsigned int* length,
                               char*);

    FMOD_RESULT F_CALL setBool(FMOD_DSP_STATE* state,
                               int index,
                               FMOD_BOOL value);

    FMOD_RESULT F_CALL setInt(FMOD_DSP_STATE* state,
                              int index,
                              int value);

    FMOD_RESULT F_CALL setFloat(FMOD_DSP_STATE* state,
                                int index,
                                float value);

    void setSource(FMOD_DSP_STATE* state,
                   IPLSource source);

    FMOD_RESULT F_CALL setData(FMOD_DSP_STATE* state,
                               int index,
                               void* value,
                               unsigned int length);
    IPLDirectEffectParams getDirectParams(FMOD_DSP_STATE* state,
                                          IPLCoordinateSpace3 source,
                                          IPLCoordinateSpace3 listener,
                                          bool updatingOverallGain);

    void updateOverallGain(FMOD_DSP_STATE* state,
                           IPLCoordinateSpace3 source,
                           IPLCoordinateSpace3 listener);

    FMOD_RESULT F_EXPORT F_CALL process(FMOD_DSP_STATE* state,
                               unsigned int length,
                               const FMOD_DSP_BUFFER_ARRAY* inBuffers,
                               FMOD_DSP_BUFFER_ARRAY* outBuffers,
                               FMOD_BOOL inputsIdle,
                               FMOD_DSP_PROCESS_OPERATION operation);

}