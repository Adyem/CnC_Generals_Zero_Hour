#include "win_compat.h"

#ifndef _WIN32

#include <chrono>
#include <cwchar>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <system_error>

#include <dlfcn.h>
#include <unistd.h>

namespace {

thread_local unsigned long g_lastError = ERROR_SUCCESS;

} // namespace

unsigned long GetLastError() { return g_lastError; }

void SetLastError(unsigned long error) { g_lastError = error; }

HMODULE LoadLibraryA(const char* fileName) {
    void* handle = dlopen(fileName, RTLD_NOW | RTLD_GLOBAL);
    if (handle == nullptr) {
        g_lastError = ERROR_CALL_NOT_IMPLEMENTED;
    }
    return reinterpret_cast<HMODULE>(handle);
}

FARPROC GetProcAddressA(HMODULE handle, const char* procName) {
    if (handle == nullptr) {
        g_lastError = ERROR_CALL_NOT_IMPLEMENTED;
        return nullptr;
    }
    void* symbol = dlsym(handle, procName);
    if (symbol == nullptr) {
        g_lastError = ERROR_CALL_NOT_IMPLEMENTED;
        return nullptr;
    }
    return reinterpret_cast<FARPROC>(symbol);
}

BOOL FreeLibrary(HMODULE handle) {
    if (handle == nullptr) {
        return FALSE;
    }
    return dlclose(handle) == 0 ? TRUE : FALSE;
}

int LoadStringA(HINSTANCE, unsigned int, char*, int) {
    g_lastError = ERROR_CALL_NOT_IMPLEMENTED;
    return 0;
}

HRSRC FindResourceA(HINSTANCE, const char*, const char*) {
    g_lastError = ERROR_CALL_NOT_IMPLEMENTED;
    return nullptr;
}

HGLOBAL LoadResource(HINSTANCE, HRSRC) {
    g_lastError = ERROR_CALL_NOT_IMPLEMENTED;
    return nullptr;
}

void* LockResource(HGLOBAL) { return nullptr; }

DWORD SizeofResource(HINSTANCE, HRSRC) { return 0; }

unsigned long GetTickCount() {
    using namespace std::chrono;
    auto now = steady_clock::now().time_since_epoch();
    return static_cast<unsigned long>(duration_cast<milliseconds>(now).count());
}

int FindExecutableA(const char* file, const char* directory, char* result) {
    if (result == nullptr) {
        return 1;
    }

    std::string path;
    if (file != nullptr) {
        path = file;
        if (!path.empty() && access(path.c_str(), X_OK) == 0) {
            std::snprintf(result, MAX_PATH, "%s", path.c_str());
            return 0;
        }
    }

    if (directory != nullptr && file != nullptr) {
        std::string combined = std::string(directory) + "/" + file;
        if (access(combined.c_str(), X_OK) == 0) {
            std::snprintf(result, MAX_PATH, "%s", combined.c_str());
            return 0;
        }
    }

    const char* pathEnv = std::getenv("PATH");
    if (pathEnv != nullptr && file != nullptr) {
        std::string current;
        for (const char* iter = pathEnv; ; ++iter) {
            if (*iter == ':' || *iter == '\0') {
                if (!current.empty()) {
                    std::string candidate = current + "/" + file;
                    if (access(candidate.c_str(), X_OK) == 0) {
                        std::snprintf(result, MAX_PATH, "%s", candidate.c_str());
                        return 0;
                    }
                }
                current.clear();
                if (*iter == '\0') {
                    break;
                }
            } else {
                current.push_back(*iter);
            }
        }
    }

    if (result != nullptr) {
        result[0] = '\0';
    }
    g_lastError = ERROR_CALL_NOT_IMPLEMENTED;
    return 2;
}

unsigned long FormatMessageA(unsigned long,
                             const void*,
                             unsigned long messageId,
                             unsigned long,
                             char* buffer,
                             unsigned long size,
                             va_list*) {
    if (buffer == nullptr || size == 0) {
        return 0;
    }

    std::error_code ec(static_cast<int>(messageId), std::generic_category());
    std::string message = ec.message();
    if (message.empty()) {
        message = "Unknown error";
    }

    std::snprintf(buffer, size, "%s (error %lu)", message.c_str(), messageId);
    return static_cast<unsigned long>(std::strlen(buffer));
}

unsigned long FormatMessageW(unsigned long flags,
                             const void* source,
                             unsigned long messageId,
                             unsigned long languageId,
                             wchar_t* buffer,
                             unsigned long size,
                             va_list* arguments) {
    if (buffer == nullptr || size == 0) {
        return 0;
    }

    char temp[1024];
    unsigned long written = FormatMessageA(flags, source, messageId, languageId, temp, sizeof(temp), arguments);
    if (written == 0) {
        return 0;
    }

    std::mbstate_t state{};
    const char* src = temp;
    wchar_t* dst = buffer;
    size_t remaining = size - 1;
    while (remaining > 0) {
        wchar_t wc;
        size_t res = std::mbrtowc(&wc, src, std::strlen(src), &state);
        if (res == static_cast<size_t>(-1) || res == static_cast<size_t>(-2)) {
            break;
        }
        if (res == 0) {
            break;
        }
        *dst++ = wc;
        src += res;
        --remaining;
    }
    *dst = L'\0';
    return static_cast<unsigned long>(dst - buffer);
}

void LocalFree(void* memory) { std::free(memory); }

void OutputDebugStringA(const char* message) {
    if (message != nullptr) {
        std::fprintf(stderr, "%s", message);
    }
}

HMODULE GetModuleHandleA(const char*) { return nullptr; }

unsigned long GetModuleFileNameA(HMODULE, char* buffer, unsigned long size) {
    if (buffer == nullptr || size == 0) {
        return 0;
    }
    buffer[0] = '\0';
    return 0;
}

#endif // !_WIN32
