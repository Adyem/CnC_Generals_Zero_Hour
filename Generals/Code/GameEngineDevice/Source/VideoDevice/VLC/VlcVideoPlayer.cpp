/*
**      Command & Conquer Generals(tm)
**      Copyright 2025 Electronic Arts Inc.
**
**      This program is free software: you can redistribute it and/or modify
**      it under the terms of the GNU General Public License as published by
**      the Free Software Foundation, either version 3 of the License, or
**      (at your option) any later version.
**
**      This program is distributed in the hope that it will be useful,
**      but WITHOUT ANY WARRANTY; without even the implied warranty of
**      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**      GNU General Public License for more details.
**
**      You should have received a copy of the GNU General Public License
**      along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//                                                                                                                             /
//
//  (c) 2001-2003 Electronic Arts Inc.                                                                                         /
//
//                                                                                                                             /
//
////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//         Includes
//----------------------------------------------------------------------------

#include "Lib/BaseType.h"
#include "VideoDevice/VLC/VlcVideoPlayer.h"
#include "Common/AudioAffect.h"
#include "Common/GameAudio.h"
#include "Common/GameMemory.h"
#include "Common/GlobalData.h"
#include "Common/Registry.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <climits>

#if WW3D_VLC_AVAILABLE
#include <vlc/libvlc_media.h>
#endif

#if WW3D_VLC_AVAILABLE

//----------------------------------------------------------------------------
//         Defines
//----------------------------------------------------------------------------

#define VIDEO_LANG_PATH_FORMAT "Data/%s/Movies/%s"
#define VIDEO_PATH             "Data/Movies"
#define VIDEO_EXT              "mp4"

namespace
{
inline Bool hasExtension(const AsciiString& fileName)
{
        const char* raw = fileName.str();
        if (!raw)
        {
                return FALSE;
        }
        return std::strchr(raw, '.') != NULL;
}

inline std::string normalizePath(const char* path)
{
        if (!path)
        {
                return std::string();
        }

        std::string result(path);
        std::replace(result.begin(), result.end(), '\\', '/');
        return result;
}
}

#ifndef _MAX_PATH
#ifdef PATH_MAX
#define _MAX_PATH PATH_MAX
#else
#define _MAX_PATH 4096
#endif
#endif

//----------------------------------------------------------------------------
//         VlcVideoStream
//----------------------------------------------------------------------------

VlcVideoStream::VlcVideoStream()
:       m_instance(NULL),
        m_mediaPlayer(NULL),
        m_frameAvailable(FALSE),
        m_width(0),
        m_height(0),
        m_pitch(0),
        m_frameRate(0.0f),
        m_frameCounter(0),
        m_totalFrames(0),
        m_duration(0),
        m_playbackInitialized(FALSE)
{
}

VlcVideoStream::~VlcVideoStream()
{
        shutdown();
}

void VlcVideoStream::shutdown()
{
        if (m_player)
        {
                if (VlcVideoPlayer* vlcPlayer = dynamic_cast<VlcVideoPlayer*>(m_player))
                {
                        if (VideoSoundBridge* bridge = vlcPlayer->getAudioBridge())
                        {
                                bridge->detach();
                        }
                }
        }

        std::lock_guard<std::mutex> lock(m_frameMutex);

        if (m_mediaPlayer)
        {
                libvlc_media_player_stop(m_mediaPlayer);
                libvlc_media_player_release(m_mediaPlayer);
                m_mediaPlayer = NULL;
        }

        m_frameBuffer.clear();
        m_backBuffer.clear();
        m_frameAvailable = FALSE;
        m_frameCounter = 0;
        m_totalFrames = 0;
        m_width = 0;
        m_height = 0;
        m_pitch = 0;
        m_frameRate = 0.0f;
        m_duration = 0;
        m_playbackInitialized = FALSE;
}

Bool VlcVideoStream::initialize(libvlc_instance_t* instance, const AsciiString& sourcePath)
{
        shutdown();

        if (!instance || sourcePath.isEmpty())
        {
                return FALSE;
        }

        m_instance = instance;

        const std::string normalizedPath = normalizePath(sourcePath.str());
        if (normalizedPath.empty())
        {
                return FALSE;
        }

        libvlc_media_t* media = libvlc_media_new_path(m_instance, normalizedPath.c_str());
        if (!media)
        {
                return FALSE;
        }

        libvlc_media_parse(media);

        unsigned int detectedWidth = 0;
        unsigned int detectedHeight = 0;
        Real detectedFrameRate = 0.0f;

        libvlc_media_track_t** tracks = NULL;
        int trackCount = libvlc_media_tracks_get(media, &tracks);
        if (trackCount > 0 && tracks)
        {
                for (int i = 0; i < trackCount; ++i)
                {
                        libvlc_media_track_t* track = tracks[i];
                        if (!track || track->i_type != libvlc_track_video || track->video == NULL)
                        {
                                continue;
                        }

                        if (track->video->i_width > 0)
                        {
                                detectedWidth = track->video->i_width;
                        }
                        if (track->video->i_height > 0)
                        {
                                detectedHeight = track->video->i_height;
                        }
                        if (track->video->i_frame_rate_den > 0 && track->video->i_frame_rate_num > 0)
                        {
                                detectedFrameRate = static_cast<Real>(track->video->i_frame_rate_num) /
                                        static_cast<Real>(track->video->i_frame_rate_den);
                        }

                        break;
                }
        }

        if (tracks && trackCount > 0)
        {
                libvlc_media_tracks_release(tracks, trackCount);
        }

        m_duration = libvlc_media_get_duration(media);

        m_mediaPlayer = libvlc_media_player_new_from_media(media);
        libvlc_media_release(media);
        if (!m_mediaPlayer)
        {
                return FALSE;
        }

        if (detectedWidth == 0 || detectedHeight == 0)
        {
                unsigned int width = libvlc_video_get_width(m_mediaPlayer);
                unsigned int height = libvlc_video_get_height(m_mediaPlayer);
                if (width > 0)
                {
                        detectedWidth = width;
                }
                if (height > 0)
                {
                        detectedHeight = height;
                }
        }

        if (detectedWidth == 0)
        {
                detectedWidth = 640;
        }
        if (detectedHeight == 0)
        {
                detectedHeight = 480;
        }

        if (detectedFrameRate <= 0.0f)
        {
                detectedFrameRate = 30.0f;
        }

        m_width = detectedWidth;
        m_height = detectedHeight;
        m_pitch = m_width * 4;
        m_frameRate = detectedFrameRate;
        m_frameBuffer.assign(m_pitch * m_height, 0);
        m_backBuffer.assign(m_pitch * m_height, 0);
        m_frameAvailable = FALSE;
        m_frameCounter = 0;
        m_totalFrames = 0;
        m_playbackInitialized = FALSE;

        if (m_duration > 0 && m_frameRate > 0.0f)
        {
                m_totalFrames = static_cast<Int>((static_cast<double>(m_duration) / 1000.0) * m_frameRate);
        }

        libvlc_video_set_callbacks(m_mediaPlayer, &VlcVideoStream::lockCallback,
                &VlcVideoStream::unlockCallback, &VlcVideoStream::displayCallback, this);
        libvlc_video_set_format(m_mediaPlayer, "RGBA", m_width, m_height, m_pitch);

        return TRUE;
}

void VlcVideoStream::ensurePlaybackStarted()
{
        if (!m_mediaPlayer || m_playbackInitialized)
        {
                return;
        }

        if (libvlc_media_player_play(m_mediaPlayer) == 0)
        {
                m_playbackInitialized = TRUE;
        }
}

Int VlcVideoStream::calculateFrameIndex() const
{
        if (!m_mediaPlayer || m_frameRate <= 0.0f)
        {
                return 0;
        }

        libvlc_time_t current = libvlc_media_player_get_time(m_mediaPlayer);
        if (current < 0)
        {
                current = 0;
        }

        const double seconds = static_cast<double>(current) / 1000.0;
        Int index = static_cast<Int>(seconds * static_cast<double>(m_frameRate));

        if (m_totalFrames > 0)
        {
                index %= m_totalFrames;
        }

        return index;
}

void* VlcVideoStream::lockCallback(void* opaque, void** planes)
{
        VlcVideoStream* self = static_cast<VlcVideoStream*>(opaque);
        if (!self)
        {
                return NULL;
        }

        std::lock_guard<std::mutex> lock(self->m_frameMutex);
        if (self->m_backBuffer.empty())
        {
                *planes = NULL;
        }
        else
        {
                *planes = self->m_backBuffer.data();
        }

        return self->m_backBuffer.empty() ? NULL : self->m_backBuffer.data();
}

void VlcVideoStream::unlockCallback(void*, void*, void* const*)
{
}

void VlcVideoStream::displayCallback(void* opaque, void* picture)
{
        VlcVideoStream* self = static_cast<VlcVideoStream*>(opaque);
        if (!self)
        {
                return;
        }

        self->handleDisplay(picture);
}

void VlcVideoStream::handleDisplay(void* picture)
{
        std::lock_guard<std::mutex> lock(m_frameMutex);

        if (m_backBuffer.empty() || m_frameBuffer.empty() || picture == NULL)
        {
                return;
        }

        std::memcpy(m_frameBuffer.data(), m_backBuffer.data(), m_backBuffer.size());
        m_frameAvailable = TRUE;
        m_frameCounter = calculateFrameIndex();
}

void VlcVideoStream::update(void)
{
        VideoStream::update();
}

Bool VlcVideoStream::isFrameReady(void)
{
        std::lock_guard<std::mutex> lock(m_frameMutex);
        return m_frameAvailable;
}

void VlcVideoStream::frameDecompress(void)
{
        ensurePlaybackStarted();
}

void VlcVideoStream::frameRender(VideoBuffer* buffer)
{
        if (!buffer)
        {
                return;
        }

        std::lock_guard<std::mutex> lock(m_frameMutex);
        if (!m_frameAvailable || m_frameBuffer.empty())
        {
                return;
        }

        void* destination = buffer->lock();
        if (!destination)
        {
                m_frameAvailable = FALSE;
                return;
        }

        const unsigned char* src = m_frameBuffer.data();
        unsigned char* dst = static_cast<unsigned char*>(destination);
        const UnsignedInt bufferHeight = std::min(buffer->height(), m_height);
        const UnsignedInt bufferWidth = std::min(buffer->width(), m_width);
        const UnsignedInt destPitch = buffer->pitch();

        switch (buffer->format())
        {
        case VideoBuffer::TYPE_X8R8G8B8:
        {
                const UnsignedInt copyWidth = bufferWidth * 4;
                for (UnsignedInt y = 0; y < bufferHeight; ++y)
                {
                        const unsigned char* srcRow = src + y * m_pitch;
                        unsigned char* dstRow = dst + y * destPitch;
                        std::memcpy(dstRow, srcRow, copyWidth);
                }
                break;
        }

        case VideoBuffer::TYPE_R8G8B8:
        {
                for (UnsignedInt y = 0; y < bufferHeight; ++y)
                {
                        const unsigned char* srcRow = src + y * m_pitch;
                        unsigned char* dstRow = dst + y * destPitch;
                        for (UnsignedInt x = 0; x < bufferWidth; ++x)
                        {
                                const unsigned char* pixel = srcRow + x * 4;
                                unsigned char* out = dstRow + x * 3;
                                out[0] = pixel[0];
                                out[1] = pixel[1];
                                out[2] = pixel[2];
                        }
                }
                break;
        }

        case VideoBuffer::TYPE_R5G6B5:
        {
                for (UnsignedInt y = 0; y < bufferHeight; ++y)
                {
                        const unsigned char* srcRow = src + y * m_pitch;
                        UnsignedShort* dstRow = reinterpret_cast<UnsignedShort*>(dst + y * destPitch);
                        for (UnsignedInt x = 0; x < bufferWidth; ++x)
                        {
                                const unsigned char* pixel = srcRow + x * 4;
                                UnsignedShort r = static_cast<UnsignedShort>(pixel[0] >> 3);
                                UnsignedShort g = static_cast<UnsignedShort>(pixel[1] >> 2);
                                UnsignedShort b = static_cast<UnsignedShort>(pixel[2] >> 3);
                                dstRow[x] = static_cast<UnsignedShort>((r << 11) | (g << 5) | b);
                        }
                }
                break;
        }

        case VideoBuffer::TYPE_X1R5G5B5:
        {
                for (UnsignedInt y = 0; y < bufferHeight; ++y)
                {
                        const unsigned char* srcRow = src + y * m_pitch;
                        UnsignedShort* dstRow = reinterpret_cast<UnsignedShort*>(dst + y * destPitch);
                        for (UnsignedInt x = 0; x < bufferWidth; ++x)
                        {
                                const unsigned char* pixel = srcRow + x * 4;
                                UnsignedShort r = static_cast<UnsignedShort>(pixel[0] >> 3);
                                UnsignedShort g = static_cast<UnsignedShort>(pixel[1] >> 3);
                                UnsignedShort b = static_cast<UnsignedShort>(pixel[2] >> 3);
                                dstRow[x] = static_cast<UnsignedShort>(0x8000 | (r << 10) | (g << 5) | b);
                        }
                }
                break;
        }

        default:
                break;
        }

        buffer->unlock();
        m_frameAvailable = FALSE;
}

void VlcVideoStream::frameNext(void)
{
        // Playback advances asynchronously under libVLC, so frameNext is a no-op.
}

Int VlcVideoStream::frameIndex(void)
{
        std::lock_guard<std::mutex> lock(m_frameMutex);
        return m_frameCounter;
}

Int VlcVideoStream::frameCount(void)
{
        return m_totalFrames;
}

void VlcVideoStream::frameGoto(Int index)
{
        if (!m_mediaPlayer || m_frameRate <= 0.0f)
        {
                return;
        }

        if (index < 0)
        {
                index = 0;
        }

        if (m_totalFrames > 0)
        {
                index = index % m_totalFrames;
        }

        const double seconds = static_cast<double>(index) / static_cast<double>(m_frameRate);
        libvlc_time_t target = static_cast<libvlc_time_t>(seconds * 1000.0);
        if (target < 0)
        {
                target = 0;
        }

        libvlc_media_player_set_time(m_mediaPlayer, target);
}

Int VlcVideoStream::height(void)
{
        return static_cast<Int>(m_height);
}

Int VlcVideoStream::width(void)
{
        return static_cast<Int>(m_width);
}

void VlcVideoStream::close(void)
{
        shutdown();
        delete this;
}

//----------------------------------------------------------------------------
//         VlcVideoPlayer
//----------------------------------------------------------------------------

VlcVideoPlayer::VlcVideoPlayer()
:       m_instance(NULL)
{
}

VlcVideoPlayer::~VlcVideoPlayer()
{
        deinit();
}

void VlcVideoPlayer::init(void)
{
        VideoPlayer::init();

        if (!m_instance)
        {
                const char* args[] = { "--no-video-title-show" };
                const int argCount = static_cast<int>(sizeof(args) / sizeof(args[0]));
                m_instance = libvlc_new(argCount, args);
                if (!m_instance)
                {
                        return;
                }
        }

        initializeAudioBridge();
}

void VlcVideoPlayer::initializeAudioBridge()
{
        if (!TheAudio)
        {
                return;
        }

        if (!m_audioBridge)
        {
                m_audioBridge = TheAudio->createVideoSoundBridge();
        }

        if (m_audioBridge && !m_audioBridge->initialize())
        {
                m_audioBridge.reset();
        }
}

void VlcVideoPlayer::reset(void)
{
        VideoPlayer::reset();
}

void VlcVideoPlayer::update(void)
{
        VideoPlayer::update();
}

void VlcVideoPlayer::deinit(void)
{
        closeAllStreams();

        if (m_audioBridge)
        {
                m_audioBridge->detach();
                m_audioBridge.reset();
        }

        if (m_instance)
        {
                libvlc_release(m_instance);
                m_instance = NULL;
        }

        VideoPlayer::deinit();
}

void VlcVideoPlayer::loseFocus(void)
{
        VideoPlayer::loseFocus();
}

void VlcVideoPlayer::regainFocus(void)
{
        VideoPlayer::regainFocus();
}

AsciiString VlcVideoPlayer::resolveMoviePath(const Video* video) const
{
        if (!video)
        {
                return AsciiString::TheEmptyString;
        }

        if (video->m_filename.isEmpty())
        {
                return AsciiString::TheEmptyString;
        }

        if (hasExtension(video->m_filename))
        {
                return video->m_filename;
        }

        char buffer[_MAX_PATH];
        std::snprintf(buffer, sizeof(buffer), "%s.%s", video->m_filename.str(), VIDEO_EXT);
        return AsciiString(buffer);
}

VideoStreamInterface* VlcVideoPlayer::createStream(const AsciiString& resolvedPath)
{
        if (!m_instance || resolvedPath.isEmpty())
        {
                return NULL;
        }

        VlcVideoStream* stream = NEW VlcVideoStream;
        if (!stream)
        {
                return NULL;
        }

        if (!stream->initialize(m_instance, resolvedPath))
        {
                delete stream;
                return NULL;
        }

        stream->m_next = m_firstStream;
        stream->m_player = this;
        m_firstStream = stream;

        if (m_audioBridge)
        {
                if (!m_audioBridge->attach(stream->mediaPlayer()))
                {
                        m_audioBridge->detach();
                        m_audioBridge.reset();
                }
        }

        stream->ensurePlaybackStarted();
        return stream;
}

VideoStreamInterface* VlcVideoPlayer::open(AsciiString movieTitle)
{
        const Video* video = getVideo(movieTitle);
        if (!video)
        {
                return NULL;
        }

        const AsciiString fileName = resolveMoviePath(video);
        if (fileName.isEmpty())
        {
                return NULL;
        }

        char localized[_MAX_PATH];
        std::snprintf(localized, sizeof(localized), VIDEO_LANG_PATH_FORMAT, GetRegistryLanguage().str(), fileName.str());

        char defaultPath[_MAX_PATH];
        std::snprintf(defaultPath, sizeof(defaultPath), "%s/%s", VIDEO_PATH, fileName.str());

        VideoStreamInterface* stream = NULL;

        if (TheGlobalData && TheGlobalData->m_modDir.isNotEmpty())
        {
                const char* modRoot = TheGlobalData->m_modDir.str();
                const size_t rootLen = std::strlen(modRoot);
                const bool needsSlash = (rootLen > 0) && (modRoot[rootLen - 1] != '/' && modRoot[rootLen - 1] != '\\');

                char modPath[_MAX_PATH];
                std::snprintf(modPath, sizeof(modPath), needsSlash ? "%s/Data/Movies/%s" : "%sData/Movies/%s", modRoot, fileName.str());
                stream = createStream(AsciiString(modPath));
                if (stream)
                {
                        return stream;
                }
        }

        stream = createStream(AsciiString(localized));
        if (stream)
        {
                return stream;
        }

        return createStream(AsciiString(defaultPath));
}

VideoStreamInterface* VlcVideoPlayer::load(AsciiString movieTitle)
{
        return open(movieTitle);
}

void VlcVideoPlayer::notifyVideoPlayerOfNewProvider(Bool nowHasValid)
{
        if (!nowHasValid)
        {
                if (m_audioBridge)
                {
                        m_audioBridge->detach();
                        m_audioBridge.reset();
                }
        }
        else
        {
                initializeAudioBridge();
        }
}

#else // WW3D_VLC_AVAILABLE

VlcVideoStream::VlcVideoStream()
:       m_instance(NULL),
        m_mediaPlayer(NULL),
        m_frameAvailable(FALSE),
        m_width(0),
        m_height(0),
        m_pitch(0),
        m_frameRate(0.0f),
        m_frameCounter(0),
        m_totalFrames(0),
        m_duration(0),
        m_playbackInitialized(FALSE)
{
}

VlcVideoStream::~VlcVideoStream() = default;

void VlcVideoStream::shutdown()
{
        std::lock_guard<std::mutex> lock(m_frameMutex);
        m_frameBuffer.clear();
        m_backBuffer.clear();
        m_frameAvailable = FALSE;
        m_frameCounter = 0;
        m_totalFrames = 0;
        m_width = 0;
        m_height = 0;
        m_pitch = 0;
        m_frameRate = 0.0f;
        m_duration = 0;
        m_playbackInitialized = FALSE;
        m_instance = NULL;
        m_mediaPlayer = NULL;
}

Bool VlcVideoStream::initialize([[maybe_unused]] libvlc_instance_t* instance,
        [[maybe_unused]] const AsciiString& sourcePath)
{
        shutdown();
        return FALSE;
}

void VlcVideoStream::update(void)
{
}

Bool VlcVideoStream::isFrameReady(void)
{
        return FALSE;
}

void VlcVideoStream::frameDecompress(void)
{
}

void VlcVideoStream::frameRender([[maybe_unused]] VideoBuffer* buffer)
{
}

void VlcVideoStream::frameNext(void)
{
}

Int VlcVideoStream::frameIndex(void)
{
        return 0;
}

Int VlcVideoStream::frameCount(void)
{
        return 0;
}

void VlcVideoStream::frameGoto([[maybe_unused]] Int index)
{
}

Int VlcVideoStream::height(void)
{
        return 0;
}

Int VlcVideoStream::width(void)
{
        return 0;
}

void VlcVideoStream::close(void)
{
        shutdown();
}

void VlcVideoStream::ensurePlaybackStarted()
{
}

Int VlcVideoStream::calculateFrameIndex() const
{
        return 0;
}

void* VlcVideoStream::lockCallback([[maybe_unused]] void* opaque,
        [[maybe_unused]] void** planes)
{
        if (planes)
        {
                *planes = NULL;
        }
        return NULL;
}

void VlcVideoStream::unlockCallback([[maybe_unused]] void* opaque,
        [[maybe_unused]] void* picture,
        [[maybe_unused]] void* const* planes)
{
}

void VlcVideoStream::displayCallback([[maybe_unused]] void* opaque,
        [[maybe_unused]] void* picture)
{
}

void VlcVideoStream::handleDisplay([[maybe_unused]] void* picture)
{
}

VlcVideoPlayer::VlcVideoPlayer()
:       m_instance(NULL)
{
}

VlcVideoPlayer::~VlcVideoPlayer() = default;

void VlcVideoPlayer::init(void)
{
        VideoPlayer::init();
        m_audioBridge.reset();
}

void VlcVideoPlayer::initializeAudioBridge()
{
        m_audioBridge.reset();
}

void VlcVideoPlayer::reset(void)
{
        VideoPlayer::reset();
}

void VlcVideoPlayer::update(void)
{
        VideoPlayer::update();
}

void VlcVideoPlayer::deinit(void)
{
        VideoPlayer::deinit();
        m_audioBridge.reset();
        m_instance = NULL;
}

void VlcVideoPlayer::loseFocus(void)
{
        VideoPlayer::loseFocus();
}

void VlcVideoPlayer::regainFocus(void)
{
        VideoPlayer::regainFocus();
}

AsciiString VlcVideoPlayer::resolveMoviePath([[maybe_unused]] const Video* video) const
{
        return AsciiString();
}

VideoStreamInterface* VlcVideoPlayer::createStream([[maybe_unused]] const AsciiString& resolvedPath)
{
        return nullptr;
}

VideoStreamInterface* VlcVideoPlayer::open([[maybe_unused]] AsciiString movieTitle)
{
        return nullptr;
}

VideoStreamInterface* VlcVideoPlayer::load([[maybe_unused]] AsciiString movieTitle)
{
        return nullptr;
}

void VlcVideoPlayer::notifyVideoPlayerOfNewProvider([[maybe_unused]] Bool nowHasValid)
{
}

#endif // WW3D_VLC_AVAILABLE
