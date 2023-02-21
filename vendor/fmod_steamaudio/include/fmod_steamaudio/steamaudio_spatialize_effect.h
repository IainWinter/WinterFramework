#pragma once

enum SpatializeEffectParams
{
    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_DATA`
     * 
     *  World-space position of the source. Automatically written by FMOD Studio.
     */
    SOURCE_POSITION,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_DATA`
     * 
     *  Overall linear gain of this effect. Automatically read by FMOD Studio.
     */
    OVERALL_GAIN,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_INT`
     * 
     *  **Range**: 0 to 2.
     * 
     *  How to render distance attenuation.
     * 
     *  -   `0`: Don't render distance attenuation.
     *  -   `1`: Use a distance attenuation value calculated using the default physics-based model.
     *  -   `2`: Use a distance attenuation value calculated using the curve specified in the FMOD Studio UI.
     */
    APPLY_DISTANCEATTENUATION,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_INT`
     *
     *  **Range**: 0 to 2.
     *
     *  How to render air absorption.
     * 
     *  -   `0`: Don't render air absorption.
     *  -   `1`: Use air absorption values calculated using the default exponential decay model.
     *  -   `2`: Use air absorption values specified in the \c AIRABSORPTION_LOW, \c AIRABSORPTION_MID, and
     *           \c AIRABSORPTION_HIGH parameters.
     */
    APPLY_AIRABSORPTION,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_INT`
     *
     *  **Range**: 0 to 2.
     *
     *  How to render directivity.
     * 
     *  -   `0`: Don't render directivity.
     *  -   `1`: Use a directivity value calculated using the default dipole model, driven by the 
     *           \c DIRECTIVITY_DIPOLEWEIGHT and \c DIRECTIVITY_DIPOLEPOWER parameters.
     *  -   `2`: Use the directivity value specified in the \c DIRECTIVITY parameter.
     */
    APPLY_DIRECTIVITY,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_INT`
     *
     *  **Range**: 0 to 2.
     *
     *  How to render occlusion.
     * 
     *  -   `0`: Don't render occlusion.
     *  -   `1`: Use the occlusion value calculated by the game engine using simulation, and provided via the
     *           \c SIMULATION_OUTPUTS parameter.
     *  -   `2`: Use the occlusion value specified in the \c OCCLUSION parameter.
     */
    APPLY_OCCLUSION,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_INT`
     *
     *  **Range**: 0 to 2.
     *
     *  How to render transmission.
     * 
     *  -   `0`: Don't render transmission.
     *  -   `1`: Use the transmission values calculated by the game engine using simulation, and provided via the
     *           \c SIMULATION_OUTPUTS parameter.
     *  -   `2`: Use the transmission values specified in the \c TRANSMISSION_LOW, \c TRANSMISSION_MID, and
     *           \c TRANSMISSION_HIGH parameters.
     */
    APPLY_TRANSMISSION,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_BOOL`
     * 
     *  If true, reflections are rendered, using the data calculated by the game engine using simulation, and provided
     *  via the \c SIMULATION_OUTPUTS parameter.
     */
    APPLY_REFLECTIONS,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_BOOL`
     *
     *  If true, pathing is rendered, using the data calculated by the game engine using simulation, and provided
     *  via the \c SIMULATION_OUTPUTS parameter.
     */
    APPLY_PATHING,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_INT`
     *
     *  **Range**: 0 to 1.
     *
     *  Controls how HRTFs are interpolated when the source moves relative to the listener.
     *
     *  - `0`: Nearest-neighbor interpolation.
     *  - `1`: Bilinear interpolation. 
     */
    HRTF_INTERPOLATION,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_FLOAT`
     * 
     *  Not currently used.
     */
    DISTANCEATTENUATION,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_INT`
     *
     *  **Range**: 0 to 4.
     *
     *  Type of distance attenuation curve preset to use when \c APPLY_DISTANCEATTENUATION is \c 1.
     * 
     *  - `0`: Linear squared rolloff.
     *  - `1`: Linear rolloff.
     *  - `2`: Inverse rolloff.
     *  - `3`: Inverse squared rolloff.
     *  - `4`: Custom rolloff.
     */
    DISTANCEATTENUATION_ROLLOFFTYPE,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_FLOAT`
     *
     *  **Range**: 0 to 10000.
     *
     *  Minimum distance value for the distance attenuation curve.
     */
    DISTANCEATTENUATION_MINDISTANCE,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_FLOAT`
     *
     *  **Range**: 0 to 10000.
     *
     *  Maximum distance value for the distance attenuation curve.
     */
    DISTANCEATTENUATION_MAXDISTANCE,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_FLOAT`
     *
     *  **Range**: 0 to 1.
     *
     *  The low frequency (up to 800 Hz) EQ value for air absorption. Only used if \c APPLY_AIRABSORPTION is set to 
     *  \c 2. 0 = low frequencies are completely attenuated, 1 = low frequencies are not attenuated at all.
     */
    AIRABSORPTION_LOW,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_FLOAT`
     *
     *  **Range**: 0 to 1.
     *
     *  The middle frequency (800 Hz - 8 kHz) EQ value for air absorption. Only used if \c APPLY_AIRABSORPTION is set 
     *  to \c 2. 0 = middle frequencies are completely attenuated, 1 = middle frequencies are not attenuated at all.
     */
    AIRABSORPTION_MID,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_FLOAT`
     *
     *  **Range**: 0 to 1.
     *
     *  The high frequency (8 kHz and above) EQ value for air absorption. Only used if \c APPLY_AIRABSORPTION is set to 
     *  \c 2. 0 = high frequencies are completely attenuated, 1 = high frequencies are not attenuated at all.
     */
    AIRABSORPTION_HIGH,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_FLOAT`
     *
     *  **Range**: 0 to 1.
     *
     *  The directivity attenuation value. Only used if \c APPLY_DIRECTIVITY is set to \c 2. 0 = sound is completely 
     *  attenuated, 1 = sound is not attenuated at all.
     */
    DIRECTIVITY,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_FLOAT`
     *
     *  **Range**: 0 to 1.
     *
     *  Blends between monopole (omnidirectional) and dipole directivity patterns. 0 = pure monopole (sound is emitted 
     *  in all directions with equal intensity), 1 = pure dipole (sound is focused to the front and back of the source). 
     *  At 0.5, the source has a cardioid directivity, with most of the sound emitted to the front of the source. Only 
     *  used if \c APPLY_DIRECTIVITY is set to \c 1.
     */
    DIRECTIVITY_DIPOLEWEIGHT,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_FLOAT`
     *
     *  **Range**: 0 to 4.
     *
     *  Controls how focused the dipole directivity is. Higher values result in sharper directivity patterns. Only used 
     *  if \c APPLY_DIRECTIVITY is set to \c 1.
     */
    DIRECTIVITY_DIPOLEPOWER,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_FLOAT`
     *
     *  **Range**: 0 to 1.
     *
     *  The occlusion attenuation value. Only used if \c APPLY_OCCLUSION is set to \c 2. 0 = sound is completely 
     *  attenuated, 1 = sound is not attenuated at all.
     */
    OCCLUSION,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_INT`
     *
     *  **Range**: 0 to 1.
     *
     *  Specifies how the transmission filter is applied.
     *
     * - `0`: Transmission is modeled as a single attenuation factor.
     * - `1`: Transmission is modeled as a 3-band EQ.
     */
    TRANSMISSION_TYPE,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_FLOAT`
     *
     *  **Range**: 0 to 1.
     *
     *  The low frequency (up to 800 Hz) EQ value for transmission. Only used if \c APPLY_TRANSMISSION is set to \c 2. 
     *  0 = low frequencies are completely attenuated, 1 = low frequencies are not attenuated at all.
     */
    TRANSMISSION_LOW,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_FLOAT`
     *
     *  **Range**: 0 to 1.
     *
     *  The middle frequency (800 Hz to 8 kHz) EQ value for transmission. Only used if \c APPLY_TRANSMISSION is set to 
     *  \c 2. 0 = middle frequencies are completely attenuated, 1 = middle frequencies are not attenuated at all.
     */
    TRANSMISSION_MID,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_FLOAT`
     *
     *  **Range**: 0 to 1.
     *
     *  The high frequency (8 kHz and above) EQ value for transmission. Only used if \c APPLY_TRANSMISSION is set to 
     *  \c 2. 0 = high frequencies are completely attenuated, 1 = high frequencies are not attenuated at all.
     */
    TRANSMISSION_HIGH,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_FLOAT`
     *
     *  **Range**: 0 to 1.
     *
     *  The contribution of the direct sound path to the overall mix for this event. Lower values reduce the 
     *  contribution more.
     */
    DIRECT_MIXLEVEL,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_BOOL`
     * 
     *  If true, applies HRTF-based 3D audio rendering to reflections. Results in an improvement in spatialization 
     *  quality when using convolution or hybrid reverb, at the cost of slightly increased CPU usage.
     */
    REFLECTIONS_BINAURAL,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_FLOAT`
     *
     *  **Range**: 0 to 10.
     *
     *  The contribution of reflections to the overall mix for this event. Lower values reduce the contribution more.
     */
    REFLECTIONS_MIXLEVEL,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_BOOL`
     * 
     *  If true, applies HRTF-based 3D audio rendering to pathing. Results in an improvement in spatialization 
     *  quality, at the cost of slightly increased CPU usage.
     */
    PATHING_BINAURAL,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_FLOAT`
     *
     *  **Range**: 0 to 10.
     *
     *  The contribution of pathing to the overall mix for this event. Lower values reduce the contribution more.
     */
    PATHING_MIXLEVEL,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_DATA`
     * 
     *  Pointer to the `IPLSimulationOutputs` structure containing simulation results.
     */
    SIMULATION_OUTPUTS,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_BOOL`
     * 
     *  If true, applies HRTF-based 3D audio rendering to the direct sound path. Otherwise, sound is panned based on
     *  the speaker configuration.
     */
    DIRECT_BINAURAL,

    /**
     *  **Type**: `FMOD_DSP_PARAMETER_TYPE_DATA`
     *  
     *  (FMOD Studio 2.02+) The event's min/max distance range. Automatically set by FMOD Studio.
     */
    DISTANCE_ATTENUATION_RANGE,

    /** The number of parameters in this effect. */
    NUM_PARAMS
};