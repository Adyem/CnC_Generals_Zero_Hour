#pragma once

#include "wpaudio/altypes.h"
#include "wpaudio/list.h"
#include "wpaudio/lock.h"

#include <cstddef>
#include <cstdint>
#include <limits>

struct File;

constexpr int vSTM_MAX_ACCESSORS = 2;
constexpr int vSTM_ACCESS_ID_IN = 0;
constexpr int vSTM_ACCESS_ID_OUT = 1;

constexpr int vSTM_ACCESS_MODE_UPDATE = 0;
constexpr int vSTM_ACCESS_MAX_MODES = 1;

constexpr UnsignedInt mSTM_STREAM_FAKE = 1u << 0;
constexpr UnsignedInt mSTM_STREAM_RESET_DONE = 1u << 1;
constexpr UnsignedInt mSTM_ACCESS_TOP_OF_START = 1u << 0;
constexpr UnsignedInt mSTM_PROFILE_ACTIVE = 1u << 0;

constexpr UnsignedInt W_UINT_MAX = std::numeric_limits<UnsignedInt>::max();

inline std::size_t AlignUpBy(std::size_t value, std::size_t alignment)
{
    if (alignment == 0)
    {
        return value;
    }
    const std::size_t mask = alignment - 1;
    return (value + mask) & ~mask;
}

#define ALIGN_UPBY(value, alignment) AlignUpBy(static_cast<std::size_t>(value), static_cast<std::size_t>(alignment))

inline void W_FlagsSet(UnsignedInt& flags, UnsignedInt mask)
{
    flags |= mask;
}

inline void W_FlagsClear(UnsignedInt& flags, UnsignedInt mask)
{
    flags &= ~mask;
}

struct STM_STREAM;
struct STM_SBUFFER;
struct STM_ACCESS;
struct STM_PROFILE;

struct STM_REGION
{
    UnsignedByte* Data = nullptr;
    UnsignedInt Bytes = 0;
};

struct STM_SBUFFER
{
    ListNode nd{};
    STM_STREAM* stream = nullptr;
    UnsignedByte* buffer_memory = nullptr;
    STM_REGION data_region{};
    STM_REGION region[vSTM_MAX_ACCESSORS]{};
};

struct STM_ACCESS
{
    STM_STREAM* stream = nullptr;
    STM_SBUFFER* SBuffer = nullptr;
    STM_SBUFFER* start_buffer = nullptr;
    STM_REGION Block{};
    Lock in_service{};
    Lock ref{};
    UnsignedInt bytes_in = 0;
    UnsignedInt bytes_out = 0;
    UnsignedInt flags = 0;
    int mode = vSTM_ACCESS_MODE_UPDATE;
    int id = 0;
    UnsignedInt access_pos = 0;
    UnsignedInt position = 0;
    int last_error = 0;
};

struct STM_STREAM
{
    ListHead buffer_list{};
    Lock ref{};
    STM_ACCESS access[vSTM_MAX_ACCESSORS]{};
    int buffers = 0;
    UnsignedInt bytes = 0;
    UnsignedInt flags = 0;
};

struct STM_PROFILE
{
    UnsignedInt bytes = 0;
    UnsignedInt rate = 0;
    UnsignedInt update_interval = 0;
    TimeStamp last_update = 0;
    UnsignedInt flags = 0;
};

void STM_stream_reset(STM_STREAM* stm);
STM_STREAM* STM_StreamCreate(void);
void STM_StreamDestroy(STM_STREAM* stm);
void STM_StreamDestroyBuffers(STM_STREAM* stm);
void STM_StreamReset(STM_STREAM* stm);
void STM_StreamAddSBuffer(STM_STREAM* stm, STM_SBUFFER* sbuf);
int STM_StreamCreateSBuffers(STM_STREAM* stm, int numBuffers, int bufferSize, int align);
STM_ACCESS* STM_StreamAcquireAccess(STM_STREAM* stm, int accessId);
UnsignedInt STM_StreamTotalBytesIn(STM_STREAM* stm);
UnsignedInt STM_StreamTotalBytesTillFull(STM_STREAM* stm);
UnsignedInt STM_StreamTotalBytes(STM_STREAM* stm);
int STM_StreamFull(STM_STREAM* stm);
STM_SBUFFER* STM_SBufferCreate(int bytes, int align);
void STM_SBufferDestroy(STM_SBUFFER* sbuf);
void STM_SBufferRemove(STM_SBUFFER* sbuf);
STM_SBUFFER* STM_SBufferNext(STM_SBUFFER* sbuf);
void STM_AccessRelease(STM_ACCESS* access);
int STM_AccessSetMode(STM_ACCESS* access, int mode);
int STM_AccessMode(STM_ACCESS* access);
int STM_AccessID(STM_ACCESS* access);
int STM_AccessTransfer(STM_ACCESS* access, void* data, int bytes);
int STM_AccessFileTransfer(STM_ACCESS* access, File* file, int bytes, int* transferred);
int STM_AccessGetError(STM_ACCESS* access);
int STM_AccessUpdate(STM_ACCESS* access);
int STM_AccessAdvance(STM_ACCESS* access, int bytesToAdvance);
void STM_AccessReturnToStart(STM_ACCESS* access);
int STM_AccessNextBlock(STM_ACCESS* access);
int STM_AccessGetBlock(STM_ACCESS* access);
UnsignedInt STM_AccessTotalBytes(STM_ACCESS* access);
UnsignedInt STM_AccessPosition(STM_ACCESS* access);
STM_ACCESS* STM_AccessUpStreamAccessor(STM_ACCESS* access);
void STM_ProfileStart(STM_PROFILE* pf);
void STM_ProfileStop(STM_PROFILE* pf);
void STM_ProfileUpdate(STM_PROFILE* pf, int bytes);
int STM_ProfileResult(STM_PROFILE* pf);
int STM_ProfileActive(STM_PROFILE* pf);

