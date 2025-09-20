#include "SfmlAudioManager.h"

#include "Common/AudioAffect.h"
#include "Common/AudioEventInfo.h"
#include "Common/AudioHandleSpecialValues.h"
#include "Common/AudioRequest.h"
#include "Common/AudioSettings.h"
#include "Common/FileSystem.h"
#include "Common/GameCommon.h"
#include "Common/GameSounds.h"
#include "Common/GlobalData.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "GameClient/View.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/TerrainLogic.h"
#include "Lib/BaseType.h"

#include <vlc/vlc.h>

#include <SFML/Audio/Listener.hpp>
#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/SoundSource.hpp>
#include <SFML/Audio/SoundStream.hpp>

#include <algorithm>
#include <cmath>
#include <condition_variable>
#include <cstdio>
#include <deque>
#include <limits>
#include <mutex>
#include <string>
#include <vector>

namespace {
constexpr const char* kSfmlProviderName = "SFML";
constexpr UnsignedInt kSfmlProviderIndex = 0;
constexpr UnsignedInt kDefaultSpeakerType = 0;

inline float clampVolume(float value) {
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > 1.0f) {
        return 1.0f;
    }
    return value;
}

inline float toSfmlVolume(float value) {
    return clampVolume(value) * 100.0f;
}

inline bool equalsIgnoreCase(const AsciiString& lhs, const AsciiString& rhs) {
    return lhs.compareNoCase(rhs) == 0;
}

class SfmlVideoAudioStream : public sf::SoundStream {
public:
    SfmlVideoAudioStream() : m_running(false) {}

    bool initializeStream(unsigned int channelCount, unsigned int sampleRate) {
        if (channelCount == 0 || sampleRate == 0) {
            return false;
        }

        m_running = true;
        initialize(channelCount, sampleRate);
        return true;
    }

    void enqueueSamples(const sf::Int16* samples, std::size_t count) {
        if (!samples || count == 0) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        m_pending.emplace_back(samples, samples + count);
        m_condition.notify_one();
    }

    void stopStream() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_running = false;
        }
        m_condition.notify_all();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pending.clear();
        m_current.clear();
    }

protected:
    bool onGetData(Chunk& data) override {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this]() { return !m_pending.empty() || !m_running; });

        if (!m_running && m_pending.empty()) {
            data.samples = nullptr;
            data.sampleCount = 0;
            return false;
        }

        m_current = std::move(m_pending.front());
        m_pending.pop_front();
        data.samples = m_current.data();
        data.sampleCount = m_current.size();
        return true;
    }

    void onSeek(sf::Time) override {
        clear();
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::deque<std::vector<sf::Int16>> m_pending;
    std::vector<sf::Int16> m_current;
    bool m_running;
};

class SfmlVlcAudioBridge : public VideoSoundBridge {
public:
    SfmlVlcAudioBridge()
        : m_mediaPlayer(nullptr), m_channelCount(0), m_sampleRate(0) {}

    ~SfmlVlcAudioBridge() override { detach(); }

    Bool initialize() override { return TRUE; }

    Bool attach(void* videoHandle) override {
        detach();

        if (!videoHandle) {
            return FALSE;
        }

        m_mediaPlayer = static_cast<libvlc_media_player_t*>(videoHandle);
        libvlc_audio_set_callbacks(m_mediaPlayer, &SfmlVlcAudioBridge::handlePlay,
                                   &SfmlVlcAudioBridge::handlePause, &SfmlVlcAudioBridge::handleResume,
                                   &SfmlVlcAudioBridge::handleFlush, &SfmlVlcAudioBridge::handleDrain, this);
        libvlc_audio_set_format_callbacks(m_mediaPlayer, &SfmlVlcAudioBridge::handleSetup,
                                          &SfmlVlcAudioBridge::handleCleanup);
        return TRUE;
    }

    void detach() override {
        if (m_mediaPlayer) {
            libvlc_audio_set_callbacks(m_mediaPlayer, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
            libvlc_audio_set_format_callbacks(m_mediaPlayer, nullptr, nullptr);
        }

        if (m_stream) {
            m_stream->stopStream();
            m_stream->stop();
            m_stream.reset();
        }

        m_mediaPlayer = nullptr;
        m_channelCount = 0;
        m_sampleRate = 0;
    }

    void onFrameDecoded(void*) override {}

private:
    static unsigned handleSetup(void** opaque, char* format, unsigned* rate, unsigned* channels) {
        if (!opaque || !*opaque || !format || !rate || !channels) {
            return 0;
        }

        auto* self = static_cast<SfmlVlcAudioBridge*>(*opaque);
        *opaque = self;
        std::memcpy(format, "S16N", 4);
        self->m_channelCount = *channels;
        self->m_sampleRate = *rate;
        self->m_stream.reset(new SfmlVideoAudioStream());
        if (!self->m_stream->initializeStream(self->m_channelCount, self->m_sampleRate)) {
            self->m_stream.reset();
            return 0;
        }
        self->m_stream->play();
        return 1;
    }

    static void handleCleanup(void* opaque) {
        auto* self = static_cast<SfmlVlcAudioBridge*>(opaque);
        if (!self) {
            return;
        }
        if (self->m_stream) {
            self->m_stream->stopStream();
            self->m_stream->stop();
            self->m_stream.reset();
        }
        self->m_channelCount = 0;
        self->m_sampleRate = 0;
    }

    static void handlePlay(void* opaque, const void* samples, unsigned count, int64_t) {
        auto* self = static_cast<SfmlVlcAudioBridge*>(opaque);
        if (!self || !self->m_stream || !samples || self->m_channelCount == 0) {
            return;
        }

        const sf::Int16* pcm = static_cast<const sf::Int16*>(samples);
        const std::size_t totalSamples = static_cast<std::size_t>(count) * self->m_channelCount;
        self->m_stream->enqueueSamples(pcm, totalSamples);
    }

    static void handlePause(void* opaque, int64_t) {
        auto* self = static_cast<SfmlVlcAudioBridge*>(opaque);
        if (self && self->m_stream) {
            self->m_stream->pause();
        }
    }

    static void handleResume(void* opaque, int64_t) {
        auto* self = static_cast<SfmlVlcAudioBridge*>(opaque);
        if (self && self->m_stream) {
            self->m_stream->play();
        }
    }

    static void handleFlush(void* opaque, int64_t) {
        auto* self = static_cast<SfmlVlcAudioBridge*>(opaque);
        if (self && self->m_stream) {
            self->m_stream->clear();
        }
    }

    static void handleDrain(void* opaque) {
        auto* self = static_cast<SfmlVlcAudioBridge*>(opaque);
        if (self && self->m_stream) {
            self->m_stream->clear();
        }
    }

    libvlc_media_player_t* m_mediaPlayer;
    std::unique_ptr<SfmlVideoAudioStream> m_stream;
    unsigned m_channelCount;
    unsigned m_sampleRate;
};

} // namespace

struct SfmlAudioManager::ActiveSound {
    AudioHandle handle;
    ChannelType type;
    std::unique_ptr<AudioEventRTS> event;
    std::unique_ptr<sf::Sound> sound;
    std::unique_ptr<sf::Music> stream;
    AsciiString filename;
    Bool paused;
    Bool volumeOverridden;
    Real overriddenVolume;
    std::deque<std::shared_ptr<sf::SoundBuffer>> pendingBuffers;
    std::shared_ptr<sf::SoundBuffer> currentBuffer;
    std::shared_ptr<sf::SoundBuffer> loopingBuffer;
    Bool loopMain;

    ActiveSound()
        : handle(0),
          type(ChannelType::Sample2D),
          paused(FALSE),
          volumeOverridden(FALSE),
          overriddenVolume(0.0f),
          loopMain(FALSE) {}

    Bool isStream() const { return type == ChannelType::Stream; }
    Bool is3D() const { return type == ChannelType::Sample3D; }
    Bool isMusic() const {
        return isStream() && event && event->getAudioEventInfo() && event->getAudioEventInfo()->m_soundType == AT_Music;
    }

    sf::SoundSource* soundSource() const {
        if (stream) {
            return stream.get();
        }
        return sound.get();
    }
};

SfmlAudioManager::SfmlAudioManager()
    : m_selectedProvider(kSfmlProviderIndex),
      m_speakerType(kDefaultSpeakerType),
      m_max2DSamples(0),
      m_max3DSamples(0),
      m_maxStreams(0),
      m_deviceOpen(FALSE),
      m_ambientPaused(FALSE),
      m_currentMusicHandle(0),
      m_musicCompleted(TRUE) {}

SfmlAudioManager::~SfmlAudioManager() { closeDevice(); }

#if defined(_DEBUG) || defined(_INTERNAL)
void SfmlAudioManager::audioDebugDisplay(DebugDisplayInterface* dd, void*, FILE* fp) {
    if (!dd && !fp) {
        return;
    }

    const UnsignedInt playing2D = std::count_if(m_activeSounds.begin(), m_activeSounds.end(), [](const auto& pair) {
        return pair.second && pair.second->type == ChannelType::Sample2D;
    });
    const UnsignedInt playing3D = std::count_if(m_activeSounds.begin(), m_activeSounds.end(), [](const auto& pair) {
        return pair.second && pair.second->type == ChannelType::Sample3D;
    });
    const UnsignedInt playingStreams = std::count_if(m_activeSounds.begin(), m_activeSounds.end(), [](const auto& pair) {
        return pair.second && pair.second->type == ChannelType::Stream;
    });

    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), "SFML Audio - 2D:%u 3D:%u Streams:%u\n", playing2D, playing3D, playingStreams);

    if (dd) {
        dd->printf(buffer);
    }

    if (fp) {
        std::fprintf(fp, "%s", buffer);
    }
}
#endif

void SfmlAudioManager::init() {
    AudioManager::init();
    openDevice();
}

void SfmlAudioManager::postProcessLoad() {
    AudioManager::postProcessLoad();
}

void SfmlAudioManager::reset() {
    AudioManager::reset();
    stopAudio(AudioAffect_All);
    m_currentMusicHandle = 0;
    m_currentMusicName = AsciiString::TheEmptyString;
    m_musicCompleted = TRUE;
}

void SfmlAudioManager::update() {
    AudioManager::update();
    setDeviceListenerPosition();
    processRequestList();
    updateActiveSounds();
    updateMusicState();
}

void SfmlAudioManager::stopAudio(AudioAffect which) { stopCategory(which); }

void SfmlAudioManager::pauseAudio(AudioAffect which) { pauseCategory(which, TRUE); }

void SfmlAudioManager::resumeAudio(AudioAffect which) { pauseCategory(which, FALSE); }

void SfmlAudioManager::pauseAmbient(Bool shouldPause) {
    if (m_ambientPaused == shouldPause) {
        return;
    }

    m_ambientPaused = shouldPause;
    for (auto& entry : m_activeSounds) {
        if (!entry.second) {
            continue;
        }
        auto* active = entry.second.get();
        if (!active->event || !active->event->isPositionalAudio()) {
            continue;
        }

        if (shouldPause) {
            pauseAudioEvent(active->handle);
        } else {
            resumeAudioEvent(active->handle);
        }
    }
}

void SfmlAudioManager::stopAllAmbientsBy(Object* obj) {
    if (!obj) {
        return;
    }

    UnsignedInt id = obj->getID();
    for (auto& entry : m_activeSounds) {
        if (!entry.second || !entry.second->event) {
            continue;
        }
        if (entry.second->event->getObjectID() == id) {
            stopAudioEvent(entry.first);
        }
    }
}

void SfmlAudioManager::stopAllAmbientsBy(Drawable* draw) {
    if (!draw) {
        return;
    }

    DrawableID id = draw->getID();
    for (auto& entry : m_activeSounds) {
        if (!entry.second || !entry.second->event) {
            continue;
        }
        if (entry.second->event->getDrawableID() == id) {
            stopAudioEvent(entry.first);
        }
    }
}

void SfmlAudioManager::killAudioEventImmediately(AudioHandle audioEvent) { stopAudioEvent(audioEvent); }

void SfmlAudioManager::nextMusicTrack() {
    AsciiString current = m_currentMusicName;
    AsciiString next = nextTrackName(current);
    if (next.isEmpty()) {
        return;
    }

    removeAudioEvent(AHSV_StopTheMusic);
    AudioEventRTS newTrack(next);
    newTrack.setPlayingHandle(allocateNewHandle());
    newTrack.generateFilename();
    newTrack.generatePlayInfo();
    friend_forcePlayAudioEventRTS(&newTrack);
}

void SfmlAudioManager::prevMusicTrack() {
    AsciiString current = m_currentMusicName;
    AsciiString prev = prevTrackName(current);
    if (prev.isEmpty()) {
        return;
    }

    removeAudioEvent(AHSV_StopTheMusic);
    AudioEventRTS newTrack(prev);
    newTrack.setPlayingHandle(allocateNewHandle());
    newTrack.generateFilename();
    newTrack.generatePlayInfo();
    friend_forcePlayAudioEventRTS(&newTrack);
}

Bool SfmlAudioManager::isMusicPlaying() const {
    for (const auto& entry : m_activeSounds) {
        if (entry.second && entry.second->isMusic()) {
            const auto& active = *entry.second;
            if (active.stream && active.stream->getStatus() == sf::SoundSource::Playing) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

Bool SfmlAudioManager::hasMusicTrackCompleted(const AsciiString& trackName, Int numberOfTimes) const {
    if (trackName != m_currentMusicName) {
        return FALSE;
    }
    if (!m_musicCompleted) {
        return FALSE;
    }
    return numberOfTimes <= 0 ? TRUE : FALSE;
}

AsciiString SfmlAudioManager::getMusicTrackName() const { return m_currentMusicName; }

void SfmlAudioManager::openDevice() {
    if (m_deviceOpen) {
        return;
    }

    const AudioSettings* settings = getAudioSettings();
    m_max2DSamples = settings ? settings->m_sampleCount2D : 32;
    m_max3DSamples = settings ? settings->m_sampleCount3D : 32;
    m_maxStreams = settings ? settings->m_streamCount : 8;
    m_deviceOpen = TRUE;
}

void SfmlAudioManager::closeDevice() {
    if (!m_deviceOpen) {
        return;
    }

    stopAudio(AudioAffect_All);
    m_bufferCache.clear();
    m_deviceOpen = FALSE;
}

void* SfmlAudioManager::getDevice() { return nullptr; }

AudioDeviceTag* SfmlAudioManager::device() const {
    return const_cast<AudioDeviceTag*>(&m_device);
}

void SfmlAudioManager::notifyOfAudioCompletion(UnsignedInt audioCompleted, UnsignedInt) {
    finishActiveSound(static_cast<AudioHandle>(audioCompleted), TRUE);
}

UnsignedInt SfmlAudioManager::getProviderCount() const { return 1; }

AsciiString SfmlAudioManager::getProviderName(UnsignedInt providerNum) const {
    if (providerNum == kSfmlProviderIndex) {
        return AsciiString(kSfmlProviderName);
    }
    return AsciiString::TheEmptyString;
}

UnsignedInt SfmlAudioManager::getProviderIndex(AsciiString providerName) const {
    return equalsIgnoreCase(providerName, AsciiString(kSfmlProviderName)) ? kSfmlProviderIndex : PROVIDER_ERROR;
}

void SfmlAudioManager::selectProvider(UnsignedInt providerNdx) {
    if (providerNdx == kSfmlProviderIndex) {
        m_selectedProvider = providerNdx;
    }
}

void SfmlAudioManager::unselectProvider() { m_selectedProvider = kSfmlProviderIndex; }

UnsignedInt SfmlAudioManager::getSelectedProvider() const { return m_selectedProvider; }

void SfmlAudioManager::setSpeakerType(UnsignedInt speakerType) { m_speakerType = speakerType; }

UnsignedInt SfmlAudioManager::getSpeakerType() { return m_speakerType; }

UnsignedInt SfmlAudioManager::getNum2DSamples() const { return m_max2DSamples; }

UnsignedInt SfmlAudioManager::getNum3DSamples() const { return m_max3DSamples; }

UnsignedInt SfmlAudioManager::getNumStreams() const { return m_maxStreams; }

Bool SfmlAudioManager::doesViolateLimit(AudioEventRTS* event) const {
    if (!event || !event->getAudioEventInfo()) {
        return FALSE;
    }

    Int limit = event->getAudioEventInfo()->m_limit;
    if (limit <= 0) {
        return FALSE;
    }

    Int playing = 0;
    for (const auto& entry : m_activeSounds) {
        if (!entry.second || !entry.second->event) {
            continue;
        }
        if (entry.second->event->getEventName() == event->getEventName()) {
            ++playing;
            if (playing >= limit) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

Bool SfmlAudioManager::isPlayingLowerPriority(AudioEventRTS*) const { return FALSE; }

Bool SfmlAudioManager::isPlayingAlready(AudioEventRTS* event) const {
    if (!event) {
        return FALSE;
    }

    for (const auto& entry : m_activeSounds) {
        if (!entry.second || !entry.second->event) {
            continue;
        }
        if (entry.second->event->getEventName() == event->getEventName()) {
            return TRUE;
        }
    }

    return FALSE;
}

Bool SfmlAudioManager::isObjectPlayingVoice(UnsignedInt objID) const {
    for (const auto& entry : m_activeSounds) {
        if (!entry.second || !entry.second->event) {
            continue;
        }
        AudioEventRTS* evt = entry.second->event.get();
        if (!evt->getAudioEventInfo()) {
            continue;
        }
        if (evt->getAudioEventInfo()->m_type & ST_VOICE) {
            if (evt->getObjectID() == objID) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

void SfmlAudioManager::adjustVolumeOfPlayingAudio(AsciiString eventName, Real newVolume) {
    for (auto& entry : m_activeSounds) {
        if (!entry.second || !entry.second->event) {
            continue;
        }
        if (entry.second->event->getEventName() != eventName) {
            continue;
        }

        if (newVolume >= 0.0f) {
            entry.second->event->setVolume(newVolume);
            entry.second->volumeOverridden = TRUE;
            entry.second->overriddenVolume = newVolume;
        } else {
            entry.second->event->setVolume(-1.0f);
            entry.second->volumeOverridden = FALSE;
        }
        applyVolume(*entry.second);
    }
}

void SfmlAudioManager::removePlayingAudio(AsciiString eventName) {
    auto handles = collectHandlesMatching(eventName);
    for (AudioHandle handle : handles) {
        stopAudioEvent(handle);
    }
}

void SfmlAudioManager::removeAllDisabledAudio() {
    std::vector<AudioHandle> handlesToRemove;
    for (const auto& entry : m_activeSounds) {
        const auto& active = entry.second;
        if (!active || !active->event) {
            continue;
        }

        for (const auto& adjusted : m_adjustedVolumes) {
            if (adjusted.first == active->event->getEventName() && adjusted.second <= 0.0f) {
                handlesToRemove.push_back(entry.first);
                break;
            }
        }
    }

    for (AudioHandle handle : handlesToRemove) {
        stopAudioEvent(handle);
    }
}

void SfmlAudioManager::friend_forcePlayAudioEventRTS(const AudioEventRTS* eventToPlay) {
    if (!eventToPlay) {
        return;
    }

    AudioEventRTS* copy = NEW AudioEventRTS(*eventToPlay);
    copy->setPlayingHandle(allocateNewHandle());
    copy->generateFilename();
    copy->generatePlayInfo();
    playAudioEvent(copy);
}

void SfmlAudioManager::setPreferredProvider(AsciiString providerNdx) { m_preferredProvider = providerNdx; }

void SfmlAudioManager::setPreferredSpeaker(AsciiString speakerType) { m_preferredSpeaker = speakerType; }

Real SfmlAudioManager::getFileLengthMS(AsciiString strToLoad) const {
    if (strToLoad.isEmpty()) {
        return 0.0f;
    }

    auto it = m_bufferCache.find(strToLoad.str());
    if (it != m_bufferCache.end()) {
        if (auto buffer = it->second.lock()) {
            return buffer->getDuration().asSeconds() * 1000.0f;
        }
    }

    sf::SoundBuffer tempBuffer;
    if (tempBuffer.loadFromFile(strToLoad.str())) {
        return tempBuffer.getDuration().asSeconds() * 1000.0f;
    }

    sf::Music music;
    if (music.openFromFile(strToLoad.str())) {
        return music.getDuration().asSeconds() * 1000.0f;
    }

    return 0.0f;
}

void SfmlAudioManager::closeAnySamplesUsingFile(const void*) {}

std::unique_ptr<VideoSoundBridge> SfmlAudioManager::createVideoSoundBridge()
{
    return std::unique_ptr<VideoSoundBridge>(new SfmlVlcAudioBridge());
}

void SfmlAudioManager::processRequestList() {
    for (auto it = m_audioRequests.begin(); it != m_audioRequests.end();) {
        AudioRequest* request = *it;
        it = m_audioRequests.erase(it);
        if (!request) {
            continue;
        }
        processRequest(request);
        request->deleteInstance();
    }
}

void SfmlAudioManager::setDeviceListenerPosition() {
    const Coord3D& pos = m_listenerPosition;
    const Coord3D& dir = m_listenerOrientation;

    sf::Listener::setPosition(static_cast<float>(pos.x), static_cast<float>(pos.y), static_cast<float>(pos.z));
    sf::Listener::setDirection(static_cast<float>(dir.x), static_cast<float>(dir.y), static_cast<float>(dir.z));
    sf::Listener::setUpVector(0.0f, 0.0f, 1.0f);
}

void SfmlAudioManager::processRequest(AudioRequest* req) {
    switch (req->m_request) {
        case AR_Play:
            playAudioEvent(req->m_pendingEvent);
            break;
        case AR_Pause:
            pauseAudioEvent(req->m_handleToInteractOn);
            break;
        case AR_Stop:
            stopAudioEvent(req->m_handleToInteractOn);
            break;
    }
}

void SfmlAudioManager::playAudioEvent(AudioEventRTS* event) {
    if (!event) {
        return;
    }

    std::unique_ptr<AudioEventRTS> owned(event);
    if (!owned->getAudioEventInfo()) {
        getInfoForAudioEvent(owned.get());
    }

    const AudioEventInfo* info = owned->getAudioEventInfo();
    if (!info) {
        releaseAudioEventRTS(owned.release());
        return;
    }

    owned->generateFilename();
    owned->generatePlayInfo();

    AsciiString filename = owned->getFilename();
    if (filename.isEmpty()) {
        releaseAudioEventRTS(owned.release());
        return;
    }

    auto active = std::make_unique<ActiveSound>();
    active->handle = owned->getPlayingHandle();
    active->event = std::move(owned);
    active->filename = filename;

    if (info->m_soundType == AT_Music || info->m_soundType == AT_Streaming) {
        auto music = std::make_unique<sf::Music>();
        if (!music->openFromFile(filename.str())) {
            releaseAudioEventRTS(active->event.release());
            return;
        }
        music->setLoop(BitTest(info->m_control, AC_LOOP));
        music->setPitch(static_cast<float>(active->event->getPitchShift()));
        music->play();
        active->type = ChannelType::Stream;
        active->stream = std::move(music);
        m_sound->notifyOf2DSampleStart();
    } else {
        auto sound = std::make_unique<sf::Sound>();
        sound->setPitch(static_cast<float>(active->event->getPitchShift()));

        if (active->event->isPositionalAudio()) {
            active->type = ChannelType::Sample3D;
            sound->setRelativeToListener(false);
            const Coord3D* pos = active->event->getCurrentPosition();
            if (pos) {
                sound->setPosition(static_cast<float>(pos->x), static_cast<float>(pos->y), static_cast<float>(pos->z));
            }
            sound->setMinDistance(static_cast<float>(info->m_minDistance));
            sound->setAttenuation(1.0f);
            m_sound->notifyOf3DSampleStart();
        } else {
            active->type = ChannelType::Sample2D;
            sound->setRelativeToListener(true);
            m_sound->notifyOf2DSampleStart();
        }

        const Bool loopMain = BitTest(info->m_control, AC_LOOP);
        active->loopMain = loopMain;

        AsciiString attackName = active->event->getAttackFilename();
        if (!attackName.isEmpty()) {
            if (auto attackBuffer = loadBuffer(attackName)) {
                active->pendingBuffers.push_back(attackBuffer);
            }
        }

        auto mainBuffer = loadBuffer(filename);
        if (!mainBuffer) {
            releaseAudioEventRTS(active->event.release());
            return;
        }
        active->loopingBuffer = mainBuffer;
        active->pendingBuffers.push_back(mainBuffer);

        if (!loopMain) {
            AsciiString decayName = active->event->getDecayFilename();
            if (!decayName.isEmpty()) {
                if (auto decayBuffer = loadBuffer(decayName)) {
                    active->pendingBuffers.push_back(decayBuffer);
                }
            }
        }

        active->sound = std::move(sound);
        if (!startNextBuffer(*active)) {
            releaseAudioEventRTS(active->event.release());
            return;
        }
    }

    applyVolume(*active);
    m_activeSounds[active->handle] = std::move(active);
}

void SfmlAudioManager::stopAudioEvent(AudioHandle handle) {
    if (handle == AHSV_StopTheMusic || handle == AHSV_StopTheMusicFade) {
        stopCategory(AudioAffect_Music);
        return;
    }

    auto it = m_activeSounds.find(handle);
    if (it == m_activeSounds.end()) {
        return;
    }

    auto& active = it->second;
    if (active->stream) {
        active->stream->stop();
    }
    if (active->sound) {
        active->sound->stop();
    }

    finishActiveSound(handle, TRUE);
}

void SfmlAudioManager::pauseAudioEvent(AudioHandle handle) {
    ActiveSound* active = findActiveSound(handle);
    if (!active) {
        return;
    }

    if (active->stream) {
        active->stream->pause();
    } else if (active->sound) {
        active->sound->pause();
    }
    active->paused = TRUE;
}

void SfmlAudioManager::resumeAudioEvent(AudioHandle handle) {
    ActiveSound* active = findActiveSound(handle);
    if (!active) {
        return;
    }

    if (active->stream) {
        active->stream->play();
    } else if (active->sound) {
        active->sound->play();
    }
    active->paused = FALSE;
}

void SfmlAudioManager::updateActiveSounds() {
    std::vector<AudioHandle> finished;
    for (auto& entry : m_activeSounds) {
        auto* active = entry.second.get();
        if (!active) {
            continue;
        }

        if (active->is3D()) {
            applySpatialization(*active);
        } else if (active->sound) {
            applyStereoBalance(*active);
        }

        if (m_volumeHasChanged) {
            applyVolume(*active);
        }

        sf::SoundSource* source = active->soundSource();
        if (!source) {
            continue;
        }

        if (source->getStatus() == sf::SoundSource::Stopped && !active->paused) {
            if (active->sound && startNextBuffer(*active)) {
                continue;
            }
            finished.push_back(entry.first);
        }
    }

    if (m_volumeHasChanged) {
        m_volumeHasChanged = FALSE;
    }

    for (AudioHandle handle : finished) {
        finishActiveSound(handle, TRUE);
    }

    purgeExpiredBuffers();
}

void SfmlAudioManager::applyVolume(ActiveSound& active) {
    Real volume = getEffectiveVolume(active.event.get());
    if (active.volumeOverridden) {
        volume = active.overriddenVolume;
    }

    sf::SoundSource* source = active.soundSource();
    if (source) {
        source->setVolume(toSfmlVolume(static_cast<float>(volume)));
    }
}

void SfmlAudioManager::applySpatialization(ActiveSound& active) {
    if (!active.sound || !active.event) {
        return;
    }

    const Coord3D* pos = active.event->getCurrentPosition();
    if (!pos) {
        return;
    }

    active.sound->setPosition(static_cast<float>(pos->x), static_cast<float>(pos->y), static_cast<float>(pos->z));
}

void SfmlAudioManager::applyStereoBalance(ActiveSound& active) {
    if (!active.sound) {
        return;
    }

    active.sound->setRelativeToListener(true);
    const Coord3D* pos = active.event ? active.event->getCurrentPosition() : nullptr;
    if (pos) {
        active.sound->setPosition(static_cast<float>(pos->x), static_cast<float>(pos->y), static_cast<float>(pos->z));
    } else {
        active.sound->setPosition(0.0f, 0.0f, 0.0f);
    }
}

bool SfmlAudioManager::startNextBuffer(ActiveSound& active) {
    if (!active.sound) {
        return false;
    }

    if (active.pendingBuffers.empty()) {
        return false;
    }

    auto nextBuffer = active.pendingBuffers.front();
    active.pendingBuffers.pop_front();
    if (!nextBuffer) {
        return false;
    }

    active.currentBuffer = nextBuffer;
    active.sound->setBuffer(*nextBuffer);
    active.sound->setLoop(active.loopMain && nextBuffer == active.loopingBuffer);

    if (active.is3D()) {
        applySpatialization(active);
    } else {
        applyStereoBalance(active);
    }

    applyVolume(active);
    active.sound->play();
    return true;
}

void SfmlAudioManager::finishActiveSound(AudioHandle handle, Bool notifyCompletion) {
    auto it = m_activeSounds.find(handle);
    if (it == m_activeSounds.end()) {
        return;
    }

    auto active = std::move(it->second);
    m_activeSounds.erase(it);

    if (!active) {
        return;
    }

    active->pendingBuffers.clear();
    active->currentBuffer.reset();
    active->loopingBuffer.reset();

    if (active->type == ChannelType::Sample3D) {
        m_sound->notifyOf3DSampleCompletion();
    } else {
        m_sound->notifyOf2DSampleCompletion();
    }

    if (active->isMusic()) {
        if (notifyCompletion) {
            m_musicCompleted = TRUE;
        }
        m_currentMusicName = active->event ? active->event->getEventName() : AsciiString::TheEmptyString;
        m_currentMusicHandle = 0;
    }

    releaseAudioEventRTS(active->event.release());
}

SfmlAudioManager::ActiveSound* SfmlAudioManager::findActiveSound(AudioHandle handle) {
    auto it = m_activeSounds.find(handle);
    if (it == m_activeSounds.end()) {
        return nullptr;
    }
    return it->second.get();
}

std::vector<AudioHandle> SfmlAudioManager::collectHandlesMatching(const AsciiString& eventName) const {
    std::vector<AudioHandle> handles;
    for (const auto& entry : m_activeSounds) {
        if (!entry.second || !entry.second->event) {
            continue;
        }
        if (entry.second->event->getEventName() == eventName) {
            handles.push_back(entry.first);
        }
    }
    return handles;
}

Real SfmlAudioManager::getEffectiveVolume(AudioEventRTS* event) const {
    if (!event || !event->getAudioEventInfo()) {
        return 0.0f;
    }

    Real volume = event->getVolume() * event->getVolumeShift();
    const AudioEventInfo* info = event->getAudioEventInfo();

    if (info->m_soundType == AT_Music) {
        volume *= m_musicVolume;
    } else if (info->m_soundType == AT_Streaming) {
        volume *= m_speechVolume;
    } else {
        if (event->isPositionalAudio()) {
            volume *= m_sound3DVolume;
            Coord3D distance = m_listenerPosition;
            const Coord3D* pos = event->getCurrentPosition();
            if (pos) {
                distance.sub(pos);
                Real objDistance = distance.length();
                Real minDistance = info->m_minDistance;
                Real maxDistance = info->m_maxDistance;
                if (BitTest(info->m_type, ST_GLOBAL)) {
                    minDistance = getAudioSettings()->m_globalMinRange;
                    maxDistance = getAudioSettings()->m_globalMaxRange;
                }
                if (objDistance > minDistance && minDistance > 0.0f) {
                    volume *= 1.0f / (objDistance / minDistance);
                }
                if (objDistance >= maxDistance && maxDistance > 0.0f) {
                    volume = 0.0f;
                }
            }
        } else {
            volume *= m_soundVolume;
        }
    }

    return volume;
}

std::shared_ptr<sf::SoundBuffer> SfmlAudioManager::loadBuffer(const AsciiString& filename) {
    auto key = std::string(filename.str());
    auto it = m_bufferCache.find(key);
    if (it != m_bufferCache.end()) {
        if (auto existing = it->second.lock()) {
            return existing;
        }
    }

    auto buffer = std::make_shared<sf::SoundBuffer>();
    if (!buffer->loadFromFile(key)) {
        return nullptr;
    }

    m_bufferCache[key] = buffer;
    return buffer;
}

void SfmlAudioManager::purgeExpiredBuffers() {
    for (auto it = m_bufferCache.begin(); it != m_bufferCache.end();) {
        if (it->second.expired()) {
            it = m_bufferCache.erase(it);
        } else {
            ++it;
        }
    }
}

void SfmlAudioManager::stopCategory(AudioAffect affect) {
    for (auto it = m_activeSounds.begin(); it != m_activeSounds.end();) {
        ActiveSound* active = it->second.get();
        if (!active || !active->event || !active->event->getAudioEventInfo()) {
            it = m_activeSounds.erase(it);
            continue;
        }

        Bool shouldStop = FALSE;
        AudioType type = active->event->getAudioEventInfo()->m_soundType;

        if (BitTest(affect, AudioAffect_Music) && type == AT_Music) {
            shouldStop = TRUE;
        }
        if (BitTest(affect, AudioAffect_Speech) && type == AT_Streaming) {
            shouldStop = TRUE;
        }
        if (BitTest(affect, AudioAffect_Sound) && type == AT_SoundEffect && !active->event->isPositionalAudio()) {
            shouldStop = TRUE;
        }
        if (BitTest(affect, AudioAffect_Sound3D) && active->event->isPositionalAudio()) {
            shouldStop = TRUE;
        }

        if (shouldStop) {
            AudioHandle handle = it->first;
            ++it;
            stopAudioEvent(handle);
        } else {
            ++it;
        }
    }
}

void SfmlAudioManager::pauseCategory(AudioAffect affect, Bool pause) {
    for (auto& entry : m_activeSounds) {
        ActiveSound* active = entry.second.get();
        if (!active || !active->event || !active->event->getAudioEventInfo()) {
            continue;
        }

        Bool shouldAffect = FALSE;
        AudioType type = active->event->getAudioEventInfo()->m_soundType;

        if (BitTest(affect, AudioAffect_Music) && type == AT_Music) {
            shouldAffect = TRUE;
        }
        if (BitTest(affect, AudioAffect_Speech) && type == AT_Streaming) {
            shouldAffect = TRUE;
        }
        if (BitTest(affect, AudioAffect_Sound) && type == AT_SoundEffect && !active->event->isPositionalAudio()) {
            shouldAffect = TRUE;
        }
        if (BitTest(affect, AudioAffect_Sound3D) && active->event->isPositionalAudio()) {
            shouldAffect = TRUE;
        }

        if (!shouldAffect) {
            continue;
        }

        if (pause) {
            pauseAudioEvent(entry.first);
        } else {
            resumeAudioEvent(entry.first);
        }
    }
}

void SfmlAudioManager::updateMusicState() {
    if (!m_currentMusicHandle) {
        for (const auto& entry : m_activeSounds) {
            if (entry.second && entry.second->isMusic()) {
                m_currentMusicHandle = entry.first;
                m_currentMusicName = entry.second->event ? entry.second->event->getEventName() : AsciiString::TheEmptyString;
                m_musicCompleted = FALSE;
                break;
            }
        }
        return;
    }

    ActiveSound* active = findActiveSound(m_currentMusicHandle);
    if (!active || !active->isMusic()) {
        m_musicCompleted = TRUE;
        m_currentMusicHandle = 0;
    }
}

