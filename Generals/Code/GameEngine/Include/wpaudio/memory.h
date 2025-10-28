
#include "wpaudio/altypes.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

struct MemoryItem
{
    MemoryItem* next = nullptr;
};

struct AudioMemoryPool
{
    MemoryItem* next = nullptr;
    std::size_t itemSize = 0;
    std::size_t itemCount = 0;
    std::size_t itemsOut = 0;
};

inline void* AudioMemAlloc(std::size_t size)
{
    return std::malloc(size);
}

inline void* AudioMemAllocZero(std::size_t size)
{
    void* ptr = std::calloc(1u, size);
    return ptr;
}

inline void AudioMemFree(void* ptr)
{
    std::free(ptr);
}

inline void AudioMemCopy(void* dst, const void* src, std::size_t size)
{
    if (dst != nullptr && src != nullptr)
    {
        std::memcpy(dst, src, size);
    }
}

inline void AudioMemSet(void* dst, int value, std::size_t size)
{
    if (dst != nullptr)
    {
        std::memset(dst, value, size);
    }
}

template <typename T>
inline T* AudioMemAllocStruct()
{
    return reinterpret_cast<T*>(AudioMemAllocZero(sizeof(T)));
}

#define ALLOC_STRUCT(var, Type) Type* var = AudioMemAllocStruct<Type>()
#define MEM_ALLOC_STRUCT(var, Type, flags) ALLOC_STRUCT(var, Type)
#define MEM_Free(ptr) AudioMemFree(ptr)

AudioMemoryPool* MemoryPoolCreate(std::size_t items, std::size_t size);
void MemoryPoolDestroy(AudioMemoryPool* pool);
void* MemoryPoolGetItem(AudioMemoryPool* pool);
void MemoryPoolReturnItem(AudioMemoryPool* pool, void* data);
int MemoryPoolCount(AudioMemoryPool* pool);

void AudioAddSlash(char* string);
int AudioHasPath(const char* string);

