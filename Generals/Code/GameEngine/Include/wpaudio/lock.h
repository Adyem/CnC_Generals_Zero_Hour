
#include "wpaudio/altypes.h"

#include <atomic>
#include <mutex>

struct Lock
{
    std::recursive_mutex mutex;
    std::atomic<int> count{0};
};

inline void LockInitPrimitive(Lock* lock)
{
    if (lock != nullptr)
    {
        lock->count.store(0, std::memory_order_relaxed);
    }
}

inline void LockAcquirePrimitive(Lock* lock)
{
    if (lock != nullptr)
    {
        lock->mutex.lock();
        lock->count.fetch_add(1, std::memory_order_relaxed);
    }
}

inline void LockReleasePrimitive(Lock* lock)
{
    if (lock != nullptr)
    {
        lock->count.fetch_sub(1, std::memory_order_relaxed);
        lock->mutex.unlock();
    }
}

inline bool LockIsLockedPrimitive(const Lock* lock)
{
    return lock != nullptr && lock->count.load(std::memory_order_relaxed) > 0;
}

#define LOCK_INIT(lockPtr) LockInitPrimitive(const_cast<Lock*>(lockPtr))
#define LOCK_ACQUIRE(lockPtr) LockAcquirePrimitive(const_cast<Lock*>(lockPtr))
#define LOCK_RELEASE(lockPtr) LockReleasePrimitive(const_cast<Lock*>(lockPtr))
#define LOCKED(lockPtr) LockIsLockedPrimitive(lockPtr)
#define NotLocked(lockPtr) (!LockIsLockedPrimitive(lockPtr))

