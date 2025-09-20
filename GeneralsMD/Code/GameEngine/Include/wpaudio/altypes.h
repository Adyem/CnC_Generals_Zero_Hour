#pragma once

#include "always.h"

#include <algorithm>
#include <chrono>

using TimeStamp = UnsignedInt;

constexpr int AUDIO_VOLUME_MIN = 0;
constexpr int AUDIO_VOLUME_MAX = 1000;
constexpr int AUDIO_PAN_CENTER = 0;

inline TimeStamp MSECONDS(Int value) {
    return static_cast<TimeStamp>(value);
}

inline Int IN_MSECONDS(TimeStamp value) {
    return static_cast<Int>(value);
}

inline TimeStamp AudioGetTime() {
    using namespace std::chrono;
    static const steady_clock::time_point kStart = steady_clock::now();
    return static_cast<TimeStamp>(
        duration_cast<milliseconds>(steady_clock::now() - kStart).count());
}

inline float ClampVolumeFraction(int volume) {
    const int clamped = std::clamp(volume, AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    return static_cast<float>(clamped) / static_cast<float>(AUDIO_VOLUME_MAX);
}
