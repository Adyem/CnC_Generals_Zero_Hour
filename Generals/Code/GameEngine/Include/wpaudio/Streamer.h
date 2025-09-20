#pragma once

#include "wpaudio/device.h"

struct AudioStreamer;

AudioStreamer* AudioStreamerCreate(AudioDeviceTag* device, TimeStamp buffering);
void AudioStreamerDestroy(AudioStreamer* streamer);
void AudioStreamerSetName(AudioStreamer* streamer, const char* name);
void AudioStreamerSetAttribs(AudioStreamer* streamer, AudioAttribs* attribs);
void AudioStreamerSetFadeAttribs(AudioStreamer* streamer, AudioAttribs* attribs);
void AudioStreamerSetPriority(AudioStreamer* streamer, Int priority);
int AudioStreamerOpen(AudioStreamer* streamer, const char* filename);
int AudioStreamerStart(AudioStreamer* streamer);
void AudioStreamerStop(AudioStreamer* streamer);
void AudioStreamerClose(AudioStreamer* streamer);
int AudioStreamerActive(AudioStreamer* streamer);
TimeStamp AudioStreamerEndTimeStamp(AudioStreamer* streamer);
void AudioStreamerSetVolume(AudioStreamer* streamer, Int volume);
void AudioStreamerSetMaxVolume(AudioStreamer* streamer, Int volume);
void AudioStreamerPause(AudioStreamer* streamer);
void AudioStreamerResume(AudioStreamer* streamer);
