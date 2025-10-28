
#include "Common/GameAudio.h"
#include "wpaudio/device.h"

#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>

namespace sf {
class Sound;
class SoundBuffer;
class Music;
}

class AudioRequest;
class Drawable;
class Object;

class SfmlAudioManager : public AudioManager {
public:
    SfmlAudioManager();
    ~SfmlAudioManager() override;

#if defined(_DEBUG) || defined(_INTERNAL)
    void audioDebugDisplay(DebugDisplayInterface* dd, void* userData, FILE* fp = nullptr) override;
#endif

    void init() override;
    void postProcessLoad() override;
    void reset() override;
    void update() override;

    void stopAudio(AudioAffect which) override;
    void pauseAudio(AudioAffect which) override;
    void resumeAudio(AudioAffect which) override;
    void pauseAmbient(Bool shouldPause) override;

    void stopAllAmbientsBy(Object* obj) override;
    void stopAllAmbientsBy(Drawable* draw) override;

    void killAudioEventImmediately(AudioHandle audioEvent) override;

    void nextMusicTrack() override;
    void prevMusicTrack() override;
    Bool isMusicPlaying() const override;
    Bool hasMusicTrackCompleted(const AsciiString& trackName, Int numberOfTimes) const override;
    AsciiString getMusicTrackName() const override;

    void openDevice() override;
    void closeDevice() override;
    void* getDevice() override;
    AudioDeviceTag* device() const override;

    void notifyOfAudioCompletion(UnsignedInt audioCompleted, UnsignedInt flags) override;

    UnsignedInt getProviderCount() const override;
    AsciiString getProviderName(UnsignedInt providerNum) const override;
    UnsignedInt getProviderIndex(AsciiString providerName) const override;
    void selectProvider(UnsignedInt providerNdx) override;
    void unselectProvider() override;
    UnsignedInt getSelectedProvider() const override;
    void setSpeakerType(UnsignedInt speakerType) override;
    UnsignedInt getSpeakerType() override;

    UnsignedInt getNum2DSamples() const override;
    UnsignedInt getNum3DSamples() const override;
    UnsignedInt getNumStreams() const override;

    Bool doesViolateLimit(AudioEventRTS* event) const override;
    Bool isPlayingLowerPriority(AudioEventRTS* event) const override;
    Bool isPlayingAlready(AudioEventRTS* event) const override;
    Bool isObjectPlayingVoice(UnsignedInt objID) const override;

    void adjustVolumeOfPlayingAudio(AsciiString eventName, Real newVolume) override;
    void removePlayingAudio(AsciiString eventName) override;
    void removeAllDisabledAudio() override;

    void friend_forcePlayAudioEventRTS(const AudioEventRTS* eventToPlay) override;

    void setPreferredProvider(AsciiString providerNdx) override;
    void setPreferredSpeaker(AsciiString speakerType) override;

    Real getFileLengthMS(AsciiString strToLoad) const override;

    void closeAnySamplesUsingFile(const void* fileToClose) override;

    std::unique_ptr<VideoSoundBridge> createVideoSoundBridge() override;

protected:
    void processRequestList() override;
    void setDeviceListenerPosition() override;

private:
    struct ActiveSound;
    enum class ChannelType {
        Sample2D,
        Sample3D,
        Stream
    };

    void processRequest(AudioRequest* req);
    void playAudioEvent(AudioEventRTS* event);
    void stopAudioEvent(AudioHandle handle);
    void pauseAudioEvent(AudioHandle handle);
    void resumeAudioEvent(AudioHandle handle);

    void updateActiveSounds();
    void applyVolume(ActiveSound& active);
    void applySpatialization(ActiveSound& active);
    void applyStereoBalance(ActiveSound& active);
    bool startNextBuffer(ActiveSound& active);
    void finishActiveSound(AudioHandle handle, Bool notifyCompletion);
    ActiveSound* findActiveSound(AudioHandle handle);
    std::vector<AudioHandle> collectHandlesMatching(const AsciiString& eventName) const;

    Real getEffectiveVolume(AudioEventRTS* event) const;

    std::shared_ptr<sf::SoundBuffer> loadBuffer(const AsciiString& filename);
    void purgeExpiredBuffers();

    void stopCategory(AudioAffect affect);
    void pauseCategory(AudioAffect affect, Bool pause);

    void updateMusicState();

    std::unordered_map<AudioHandle, std::unique_ptr<ActiveSound>> m_activeSounds;
    std::unordered_map<std::string, std::weak_ptr<sf::SoundBuffer>> m_bufferCache;

    UnsignedInt m_selectedProvider;
    UnsignedInt m_speakerType;
    UnsignedInt m_max2DSamples;
    UnsignedInt m_max3DSamples;
    UnsignedInt m_maxStreams;

    Bool m_deviceOpen;
    Bool m_ambientPaused;

    AsciiString m_preferredProvider;
    AsciiString m_preferredSpeaker;

    AudioHandle m_currentMusicHandle;
    AsciiString m_currentMusicName;
    Bool m_musicCompleted;

    AudioDeviceTag m_device;
};

