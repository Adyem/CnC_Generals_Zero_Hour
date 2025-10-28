
#include <cstddef>
#include <cstdint>
#include <memory>

namespace GameAudio
{

struct PCMBufferView
{
    const void *data = nullptr;
    std::size_t bytes = 0;
    std::uint32_t sampleRate = 0;
    std::uint32_t channelCount = 0;
    std::uint32_t bitsPerSample = 0;
};

struct VoiceAttributes
{
    float volume = 1.0f;
    float pan = 0.0f;
    float pitch = 1.0f;
    bool looping = false;
};

struct SpatialAttributes
{
    bool enabled = false;
    float position[3] = {0.0f, 0.0f, 0.0f};
    float velocity[3] = {0.0f, 0.0f, 0.0f};
    float forward[3] = {0.0f, 0.0f, 1.0f};
    float up[3] = {0.0f, 1.0f, 0.0f};
    float minDistance = 1.0f;
    float maxDistance = 10000.0f;
    float doppler = 1.0f;
};

struct ListenerAttributes
{
    float position[3] = {0.0f, 0.0f, 0.0f};
    float velocity[3] = {0.0f, 0.0f, 0.0f};
    float forward[3] = {0.0f, 0.0f, 1.0f};
    float up[3] = {0.0f, 1.0f, 0.0f};
};

class IStreamingSource
{
public:
    virtual ~IStreamingSource() = default;

    virtual std::size_t OnStreamRequest(std::size_t bytesRequested, PCMBufferView &outChunk) = 0;
    virtual void OnStreamRewind() = 0;
};

class IAudioVoice
{
public:
    virtual ~IAudioVoice() = default;

    virtual void Start(const PCMBufferView &buffer, bool queueForLoop) = 0;
    virtual void SubmitBuffer(const PCMBufferView &buffer, bool isLast) = 0;
    virtual void AttachStream(IStreamingSource *source) = 0;
    virtual void DetachStream() = 0;

    virtual void Stop() = 0;
    virtual void Pause(bool paused) = 0;
    virtual bool IsPlaying() const = 0;

    virtual void SetAttributes(const VoiceAttributes &attributes) = 0;
    virtual VoiceAttributes GetAttributes() const = 0;

    virtual void SetSpatialAttributes(const SpatialAttributes &attributes) = 0;
    virtual SpatialAttributes GetSpatialAttributes() const = 0;

    virtual void *GetUserData() const = 0;
    virtual void SetUserData(void *userdata) = 0;
};

struct DeviceCreateInfo
{
    std::uint32_t sampleRate = 44100;
    std::uint32_t channelCount = 2;
    std::uint32_t bitsPerSample = 16;
    std::uint32_t maxVoices = 32;
};

class IAudioDevice
{
public:
    virtual ~IAudioDevice() = default;

    virtual std::unique_ptr<IAudioVoice> CreateVoice(const VoiceAttributes &defaults,
                                                     const SpatialAttributes &spatialDefaults) = 0;
    virtual void DestroyVoice(IAudioVoice *voice) = 0;

    virtual void UpdateListener(const ListenerAttributes &listener) = 0;

    virtual void Service() = 0;

    virtual std::uint32_t MaxVoices() const = 0;
};

struct BackendInitInfo
{
    std::uint32_t preferredSampleRate = 44100;
    std::uint32_t preferredChannels = 2;
    std::uint32_t preferredBitsPerSample = 16;
};

class IAudioBackend
{
public:
    virtual ~IAudioBackend() = default;

    virtual bool Initialize(const BackendInitInfo &info) = 0;
    virtual void Shutdown() = 0;

    virtual std::unique_ptr<IAudioDevice> CreateDevice(const DeviceCreateInfo &info) = 0;

    virtual void Process() = 0;
};

} // namespace GameAudio
