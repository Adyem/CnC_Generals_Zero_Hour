
#include <cstddef>

using GHTTPRequest = int;
using GHTTPResult = int;
using GHTTPBool = int;

constexpr GHTTPBool GHTTPTrue = 1;
constexpr GHTTPBool GHTTPFalse = 0;
constexpr GHTTPResult GHTTPSuccess = 0;
constexpr GHTTPResult GHTTPError = 1;

using GHTTPCallback = GHTTPBool (*)(GHTTPRequest, GHTTPResult, char *, int, void *);

inline GHTTPRequest ghttpGet(const char *, GHTTPBool, GHTTPCallback callback, void *param)
{
        if (callback != nullptr) {
                callback(0, GHTTPError, nullptr, 0, param);
        }
        return 0;
}

inline GHTTPRequest ghttpHead(const char *, GHTTPBool, GHTTPCallback callback, void *param)
{
        if (callback != nullptr) {
                callback(0, GHTTPError, nullptr, 0, param);
        }
        return 0;
}

inline const char *ghttpGetHeaders(GHTTPRequest)
{
        return "";
}

inline void ghttpSetProxy(const char *) {}

inline void ghttpThink(void) {}

inline void ghttpStartup(void) {}

inline void ghttpCleanup(void) {}
