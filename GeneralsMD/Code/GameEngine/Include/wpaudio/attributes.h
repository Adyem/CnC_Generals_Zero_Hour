
#include "wpaudio/altypes.h"

struct AudioAttribs {
    Int volume = AUDIO_VOLUME_MAX;
};

inline void AudioAttribsInit(AudioAttribs* attribs) {
    if (attribs != nullptr) {
        attribs->volume = AUDIO_VOLUME_MAX;
    }
}

inline void AudioAttribsAdjustVolume(AudioAttribs* attribs, Int volume) {
    if (attribs != nullptr) {
        attribs->volume = std::clamp(volume, AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    }
}

inline void AudioAttribsSetVolume(AudioAttribs* attribs, Int volume) {
    AudioAttribsAdjustVolume(attribs, volume);
}

inline Bool AudioAttribsVolumeAdjusted(const AudioAttribs*) {
    return TRUE;
}
