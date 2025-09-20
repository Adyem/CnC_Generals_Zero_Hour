#include "wpaudio/Streamer.h"

#include <SFML/Audio/Music.hpp>

#include <algorithm>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace {

struct StreamInternal {
    std::unique_ptr<sf::Music> music;
    AudioDeviceTag* device = nullptr;
    TimeStamp buffering = 0;
    std::string name;
    AudioAttribs* attribs = nullptr;
    AudioAttribs* fadeAttribs = nullptr;
    Int priority = 0;
    Int volume = AUDIO_VOLUME_MAX;
    Int maxVolume = AUDIO_VOLUME_MAX;
    TimeStamp startTime = 0;
    TimeStamp duration = 0;
    bool open = false;
    bool loop = false;
};

std::mutex gAllocationMutex;
std::vector<std::unique_ptr<StreamInternal>> gStreams;

float ToSfmlVolume(Int volume, Int maxVolume) {
    const Int clampedMax = std::max(AUDIO_VOLUME_MIN, maxVolume);
    const Int clampedVolume = std::clamp(volume, AUDIO_VOLUME_MIN, clampedMax);
    const float percent = ClampVolumeFraction(clampedVolume);
    return percent * 100.0f;
}

std::string NormalizePath(const char* filename) {
    std::string path = filename != nullptr ? filename : "";
    std::replace(path.begin(), path.end(), '\\', '/');
    return path;
}

void ApplyVolume(StreamInternal& stream) {
    if (stream.music) {
        stream.music->setVolume(ToSfmlVolume(stream.volume, stream.maxVolume));
    }
}

TimeStamp ComputeEndTime(const StreamInternal& stream) {
    if (!stream.open || stream.duration == 0) {
        return AudioGetTime();
    }
    return stream.startTime + stream.duration;
}

} // namespace

AudioStreamer* AudioStreamerCreate(AudioDeviceTag* device, TimeStamp buffering) {
    auto internal = std::make_unique<StreamInternal>();
    internal->music = std::make_unique<sf::Music>();
    internal->device = device;
    internal->buffering = buffering;
    internal->startTime = AudioGetTime();
    StreamInternal* handle = internal.get();
    {
        std::lock_guard<std::mutex> lock(gAllocationMutex);
        gStreams.emplace_back(std::move(internal));
    }
    return reinterpret_cast<AudioStreamer*>(handle);
}

void AudioStreamerDestroy(AudioStreamer* streamer) {
    if (streamer == nullptr) {
        return;
    }
    StreamInternal* internal = reinterpret_cast<StreamInternal*>(streamer);
    internal->music.reset();
    std::lock_guard<std::mutex> lock(gAllocationMutex);
    gStreams.erase(std::remove_if(gStreams.begin(), gStreams.end(),
                                  [internal](const std::unique_ptr<StreamInternal>& ptr) {
                                      return ptr.get() == internal;
                                  }),
                   gStreams.end());
}

void AudioStreamerSetName(AudioStreamer* streamer, const char* name) {
    if (streamer == nullptr) {
        return;
    }
    auto* internal = reinterpret_cast<StreamInternal*>(streamer);
    internal->name = (name != nullptr) ? name : "";
}

void AudioStreamerSetAttribs(AudioStreamer* streamer, AudioAttribs* attribs) {
    if (streamer == nullptr) {
        return;
    }
    reinterpret_cast<StreamInternal*>(streamer)->attribs = attribs;
}

void AudioStreamerSetFadeAttribs(AudioStreamer* streamer, AudioAttribs* attribs) {
    if (streamer == nullptr) {
        return;
    }
    reinterpret_cast<StreamInternal*>(streamer)->fadeAttribs = attribs;
}

void AudioStreamerSetPriority(AudioStreamer* streamer, Int priority) {
    if (streamer == nullptr) {
        return;
    }
    reinterpret_cast<StreamInternal*>(streamer)->priority = priority;
}

int AudioStreamerOpen(AudioStreamer* streamer, const char* filename) {
    if (streamer == nullptr) {
        return FALSE;
    }
    auto* internal = reinterpret_cast<StreamInternal*>(streamer);
    if (!internal->music) {
        internal->music = std::make_unique<sf::Music>();
    }
    internal->music->stop();
    const std::string path = NormalizePath(filename);
    if (!internal->music->openFromFile(path)) {
        internal->open = false;
        internal->duration = 0;
        return FALSE;
    }
    internal->duration = static_cast<TimeStamp>(
        internal->music->getDuration().asMilliseconds());
    internal->open = true;
    internal->startTime = AudioGetTime();
    ApplyVolume(*internal);
    internal->music->setLoop(internal->loop);
    return TRUE;
}

int AudioStreamerStart(AudioStreamer* streamer) {
    if (streamer == nullptr) {
        return FALSE;
    }
    auto* internal = reinterpret_cast<StreamInternal*>(streamer);
    if (!internal->open || !internal->music) {
        return FALSE;
    }
    internal->startTime = AudioGetTime();
    ApplyVolume(*internal);
    internal->music->play();
    return TRUE;
}

void AudioStreamerStop(AudioStreamer* streamer) {
    if (streamer == nullptr) {
        return;
    }
    auto* internal = reinterpret_cast<StreamInternal*>(streamer);
    if (internal->music) {
        internal->music->stop();
    }
    internal->startTime = AudioGetTime();
}

void AudioStreamerClose(AudioStreamer* streamer) {
    if (streamer == nullptr) {
        return;
    }
    auto* internal = reinterpret_cast<StreamInternal*>(streamer);
    AudioStreamerStop(streamer);
    internal->music = std::make_unique<sf::Music>();
    internal->open = false;
    internal->duration = 0;
}

int AudioStreamerActive(AudioStreamer* streamer) {
    if (streamer == nullptr) {
        return FALSE;
    }
    auto* internal = reinterpret_cast<StreamInternal*>(streamer);
    if (!internal->music) {
        return FALSE;
    }
    const auto status = internal->music->getStatus();
    return status == sf::SoundSource::Playing || status == sf::SoundSource::Paused;
}

TimeStamp AudioStreamerEndTimeStamp(AudioStreamer* streamer) {
    if (streamer == nullptr) {
        return AudioGetTime();
    }
    const auto* internal = reinterpret_cast<StreamInternal*>(streamer);
    return ComputeEndTime(*internal);
}

void AudioStreamerSetVolume(AudioStreamer* streamer, Int volume) {
    if (streamer == nullptr) {
        return;
    }
    auto* internal = reinterpret_cast<StreamInternal*>(streamer);
    internal->volume = std::clamp(volume, AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    ApplyVolume(*internal);
}

void AudioStreamerSetMaxVolume(AudioStreamer* streamer, Int volume) {
    if (streamer == nullptr) {
        return;
    }
    auto* internal = reinterpret_cast<StreamInternal*>(streamer);
    internal->maxVolume = std::clamp(volume, AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    internal->volume = std::clamp(internal->volume, AUDIO_VOLUME_MIN, internal->maxVolume);
    ApplyVolume(*internal);
}

void AudioStreamerPause(AudioStreamer* streamer) {
    if (streamer == nullptr) {
        return;
    }
    auto* internal = reinterpret_cast<StreamInternal*>(streamer);
    if (internal->music && internal->music->getStatus() == sf::SoundSource::Playing) {
        internal->music->pause();
    }
}

void AudioStreamerResume(AudioStreamer* streamer) {
    if (streamer == nullptr) {
        return;
    }
    auto* internal = reinterpret_cast<StreamInternal*>(streamer);
    if (internal->music && internal->music->getStatus() == sf::SoundSource::Paused) {
        ApplyVolume(*internal);
        internal->music->play();
    }
}
