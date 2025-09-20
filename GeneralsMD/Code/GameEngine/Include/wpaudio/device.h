#pragma once

#include "wpaudio/attributes.h"

struct AudioDeviceTag {};

inline void AudioDeviceAttribsAdd(AudioDeviceTag*, AudioAttribs*) {}

inline void AudioDeviceAttribsRemove(AudioDeviceTag*, AudioAttribs*) {}
