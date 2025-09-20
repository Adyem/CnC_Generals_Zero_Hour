#pragma once

#include <cassert>
#include <cstdio>

#if defined(NDEBUG)
#define DBG_CODE(expr) do { (void)sizeof(expr); } while (false)
#else
#define DBG_CODE(expr) do { expr; } while (false)
#endif

#define DBG_DECLARE_TYPE(Type)
#define DBG_TYPE()
#define DBG_SET_TYPE(ptr, Type) do { (void)(ptr); } while (false)
#define DBG_INVALIDATE_TYPE(ptr) do { (void)(ptr); } while (false)

#define DBG_ASSERT(expr) assert((expr))
#define DBG_ASSERT_PTR(ptr) assert((ptr) != nullptr)
#define DBG_ASSERT_TYPE(ptr, Type) assert((ptr) != nullptr)

#define DBG_MSGASSERT(expr, args)                                                    \
    do {                                                                             \
        if (!(expr)) {                                                               \
            std::printf args;                                                        \
            assert((expr));                                                          \
        }                                                                            \
    } while (false)

#define DBGPRINTF(args) do { std::printf args; } while (false)
#define DBG_Function(name) do { (void)(name); } while (false)

#define DBG_ASSERT_MSG(expr, args)                                                   \
    do {                                                                             \
        if (!(expr)) {                                                               \
            std::printf args;                                                        \
            assert((expr));                                                          \
        }                                                                            \
    } while (false)

