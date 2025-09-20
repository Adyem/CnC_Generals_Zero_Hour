# WPAudio DirectSound and waveOut Usage

## Driver bootstrap and `AudioSystem` integration
- `AUD_DSoundDriver` registers a DirectSound-backed `AudioSystemMaster` whose vtable exposes the driver entry points (`audioLoad`, `audioUnload`, `audioOpen`, `audioClose`, and the channel callbacks) that `AudioSystem` instances call during device discovery and teardown.【F:Generals/Code/Libraries/Source/WPAudio/AUD_DSoundDriver.cpp†L243-L254】
- `audioLoad` starts the Miles ASI helpers, instantiates the DirectSound device, raises the cooperative level, and spins up the real-time mixer thread that will service every audio device.【F:Generals/Code/Libraries/Source/WPAudio/AUD_DSoundDriver.cpp†L1333-L1368】
- When a device opens, the driver seeds poll intervals, binds the service thread to the new device, creates the DirectSound primary buffer, and kicks it into a looping play state so secondary buffers can mix into it.【F:Generals/Code/Libraries/Source/WPAudio/AUD_DSoundDriver.cpp†L1405-L1457】
- Each `audioOpenChannel` call allocates an `AUD_DRV_CHAN` structure, points it at the owning `AudioChannel`, preconfigures frame timing, and prepares to build a DirectSound secondary buffer when playback begins.【F:Generals/Code/Libraries/Source/WPAudio/AUD_DSoundDriver.cpp†L1486-L1500】

## Channel lifetime, buffering, and callbacks
- `AUD_DRV_CHAN` tracks the DirectSound buffer interface alongside source pointers, write cursors, decoded transfer state, and the timing used by the mixer to decide when to refill audio data.【F:Generals/Code/Libraries/Source/WPAudio/AUD_DSoundDriver.cpp†L169-L210】
- `AUD_service_device` runs on the driver thread; it walks every `AudioChannel` on the device, updates service timers, advances buffers for voices that are still playing, and invokes the driver stop path when a channel falls idle.【F:Generals/Code/Libraries/Source/WPAudio/AUD_DSoundDriver.cpp†L660-L714】
- Device helpers in `AUD_Device.cpp` enforce the channel cap, create `AudioChannel` instances, and add them to the device before the driver-specific `openChannel` hook runs.【F:Generals/Code/Libraries/Source/WPAudio/AUD_Device.cpp†L1075-L1098】
- `audioChannelCreate` wires new channels back to the driver, seeds default attributes, and immediately delegates to `openChannel`, so higher-level systems always talk to the driver through the `AudioChannel` abstraction.【F:Generals/Code/Libraries/Source/WPAudio/AUD_Channel.cpp†L106-L149】

## Streaming pipeline
- `AudioStreamer` installs driver callbacks (`streamNextFrame`, `streamSampleDone`, `streamStop`) that recycle buffers out of the streaming ring, update frame sizes, and stop channels once the stream flags drop.【F:Generals/Code/Libraries/Source/WPAudio/AUD_Streamer.cpp†L142-L218】
- `streamFill` coordinates with the background streaming thread to pull more data from the source file, loop when requested, and feed bytes into the shared ring buffer that `AudioChannel` instances read from.【F:Generals/Code/Libraries/Source/WPAudio/AUD_Streamer.cpp†L225-L279】
- The dedicated streamer thread (`service_streams`) wakes at fixed intervals, walks every active `AudioStreamer`, and calls `service_stream` so queued buffers stay primed.【F:Generals/Code/Libraries/Source/WPAudio/AUD_Streamer.cpp†L286-L321】【F:Generals/Code/Libraries/Source/WPAudio/AUD_Streamer.cpp†L374-L390】

## WWAudio bridge and Miles handle management
- `WWAudioClass::Open_2D_Device` toggles the Miles preference to favor DirectSound, opens the driver, checks for emulation, and falls back to waveOut when necessary before reassigning handles to active sounds.【F:Generals/Code/Libraries/Source/WWVegas/WWAudio/WWAudio.cpp†L320-L367】
- `Allocate_2D_Handles` refreshes the pool by asking Miles for every available `HSAMPLE`, storing them locally, and clearing the user-data slots so WWAudio can attach owners later.【F:Generals/Code/Libraries/Source/WWVegas/WWAudio/WWAudio.cpp†L1473-L1504】
- `Get_2D_Sample` and its 3D counterpart search the handle pool for unused entries, fall back to priority-based stealing, and rely on Miles user data to identify which `AudibleSound` currently owns each handle.【F:Generals/Code/Libraries/Source/WWVegas/WWAudio/WWAudio.cpp†L1520-L1569】
- `Sound2DHandleClass` wraps the Miles 2D sample API for playback control, translating pan, volume, loop count, seek position, user data, and playback rate requests directly into `AIL_set_*` or `AIL_sample_*` calls.【F:Generals/Code/Libraries/Source/WWVegas/WWAudio/sound2dhandle.cpp†L147-L352】
- `SoundStreamHandleClass` manages paired Miles stream/sample handles so streamed content can start, stop, adjust attributes, and still expose sample user data for WWAudio’s bookkeeping.【F:Generals/Code/Libraries/Source/WWVegas/WWAudio/soundstreamhandle.cpp†L96-L316】
