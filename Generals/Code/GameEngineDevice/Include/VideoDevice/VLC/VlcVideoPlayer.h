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
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:    Generals
//
// File name:  GameClient/VlcVideoPlayer.h
//
// Created:    01/22/25
//
//----------------------------------------------------------------------------

#pragma once

#ifndef __VIDEODEVICE_VLCVIDEOPLAYER_H_
#define __VIDEODEVICE_VLCVIDEOPLAYER_H_

//----------------------------------------------------------------------------
//           Includes
//----------------------------------------------------------------------------

#include "GameClient/VideoPlayer.h"

#include <memory>
#include <mutex>
#include <vector>

#include <vlc/vlc.h>

class VideoSoundBridge;

//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------

class VlcVideoPlayer;

//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------

//===============================
// VlcVideoStream
//===============================

class VlcVideoStream : public VideoStream
{
        friend class VlcVideoPlayer;

        protected:
                VlcVideoStream();
                virtual ~VlcVideoStream();

                Bool initialize( libvlc_instance_t *instance, const AsciiString &sourcePath );

        public:
                virtual void update( void ) override;

                virtual Bool    isFrameReady( void ) override;
                virtual void    frameDecompress( void ) override;
                virtual void    frameRender( VideoBuffer *buffer ) override;
                virtual void    frameNext( void ) override;
                virtual Int             frameIndex( void ) override;
                virtual Int             frameCount( void ) override;
                virtual void    frameGoto( Int index ) override;
                virtual Int             height( void ) override;
                virtual Int             width( void ) override;
                virtual void    close( void ) override;

                libvlc_media_player_t *mediaPlayer() const { return m_mediaPlayer; }

        private:
                void shutdown();
                void ensurePlaybackStarted();
                Int calculateFrameIndex() const;

                static void *lockCallback( void *opaque, void **planes );
                static void unlockCallback( void *opaque, void *picture, void *const *planes );
                static void displayCallback( void *opaque, void *picture );

                void handleDisplay( void *picture );

                libvlc_instance_t              *m_instance;
                libvlc_media_player_t  *m_mediaPlayer;

                std::vector<unsigned char> m_frameBuffer;
                std::vector<unsigned char> m_backBuffer;

                mutable std::mutex     m_frameMutex;
                Bool                                   m_frameAvailable;
                UnsignedInt                     m_width;
                UnsignedInt                     m_height;
                UnsignedInt                     m_pitch;
                Real                                    m_frameRate;
                Int                                             m_frameCounter;
                Int                                             m_totalFrames;
                libvlc_time_t                  m_duration;
                Bool                                    m_playbackInitialized;
};

//===============================
// VlcVideoPlayer
//===============================

class VlcVideoPlayer : public VideoPlayer
{
        protected:
                VideoStreamInterface *createStream( const AsciiString &resolvedPath );

        public:
                VlcVideoPlayer();
                virtual ~VlcVideoPlayer();

                virtual void    init( void ) override;
                virtual void    reset( void ) override;
                virtual void    update( void ) override;
                virtual void    deinit( void ) override;

                virtual void    loseFocus( void ) override;
                virtual void    regainFocus( void ) override;

                virtual VideoStreamInterface*   open( AsciiString movieTitle ) override;
                virtual VideoStreamInterface*   load( AsciiString movieTitle ) override;

                virtual void notifyVideoPlayerOfNewProvider( Bool nowHasValid ) override;
                void initializeAudioBridge();

                VideoSoundBridge *getAudioBridge() const { return m_audioBridge.get(); }
                libvlc_instance_t *getInstance() const { return m_instance; }

        private:
                AsciiString resolveMoviePath( const Video *video ) const;

                libvlc_instance_t *m_instance;
                std::unique_ptr<VideoSoundBridge> m_audioBridge;
};

#endif // __VIDEODEVICE_VLCVIDEOPLAYER_H_
