#include "win_compat.h"

#ifndef _WIN32

#include <chrono>
#include <ctime>
#include <cwchar>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <functional>
#include <string>
#include <system_error>
#include <csignal>
#include <cerrno>

#include <sys/stat.h>
#include <fcntl.h>

#include <dlfcn.h>
#include <unistd.h>

namespace cnc::windows
{
namespace
{
thread_local DWORD g_lastError = ERROR_SUCCESS;

int resolveDefaultMessageBoxResponse(UINT type)
{
    if (type & MB_ABORTRETRYIGNORE)
    {
        return IDIGNORE;
    }
    if (type & MB_RETRYCANCEL)
    {
        return IDCANCEL;
    }
    if (type & MB_YESNOCANCEL)
    {
        return IDYES;
    }
    if (type & MB_YESNO)
    {
        return IDYES;
    }
    return IDOK;
}

std::string wideToUtf8(LPCWSTR text)
{
    if (text == nullptr)
    {
        return std::string{};
    }

    std::mbstate_t state{};
    const wchar_t* source = text;
    std::size_t required = std::wcsrtombs(nullptr, &source, 0, &state);
    if (required == static_cast<std::size_t>(-1))
    {
        return std::string{};
    }

    std::string buffer(required, '\0');
    source = text;
    state = std::mbstate_t{};
    std::wcsrtombs(buffer.data(), &source, buffer.size(), &state);
    return buffer;
}

FILETIME unixTimeToFileTime(std::time_t value)
{
    constexpr long long WINDOWS_TICK = 10000000LL;
    constexpr long long SEC_TO_UNIX_EPOCH = 11644473600LL;

    long long total = (static_cast<long long>(value) + SEC_TO_UNIX_EPOCH) * WINDOWS_TICK;
    FILETIME result{};
    result.dwLowDateTime = static_cast<DWORD>(total & 0xffffffffu);
    result.dwHighDateTime = static_cast<DWORD>((total >> 32) & 0xffffffffu);
    return result;
}
} // namespace

DWORD GetLastError()
{
    return g_lastError;
}

void SetLastError(DWORD error)
{
    g_lastError = error;
}

DWORD GetCurrentThreadId()
{
    auto id = std::this_thread::get_id();
    auto hash = std::hash<std::thread::id>{}(id);
    return static_cast<DWORD>(hash);
}

void DebugBreak()
{
#ifdef SIGTRAP
    std::raise(SIGTRAP);
#else
    std::abort();
#endif
}

BOOL ShowWindow(HWND, int)
{
    return TRUE_VALUE;
}

BOOL SetWindowPos(HWND,
                  HWND,
                  int,
                  int,
                  int,
                  int,
                  UINT)
{
    return TRUE_VALUE;
}

int MessageBoxA(HWND,
                LPCSTR text,
                LPCSTR caption,
                UINT type)
{
    const char* resolvedCaption = caption != nullptr ? caption : "";
    const char* resolvedText = text != nullptr ? text : "";
    std::fprintf(stderr, "[MessageBox] %s: %s\n", resolvedCaption, resolvedText);
    return resolveDefaultMessageBoxResponse(type);
}

int MessageBoxW(HWND window,
                LPCWSTR text,
                LPCWSTR caption,
                UINT type)
{
    std::string convertedText = wideToUtf8(text);
    std::string convertedCaption = wideToUtf8(caption);
    return MessageBoxA(window, convertedText.c_str(), convertedCaption.c_str(), type);
}

HMODULE LoadLibraryA(const char* fileName)
{
    void* handle = dlopen(fileName, RTLD_NOW | RTLD_GLOBAL);
    if (handle == nullptr)
    {
        g_lastError = ERROR_CALL_NOT_IMPLEMENTED;
    }
    return reinterpret_cast<HMODULE>(handle);
}

FARPROC GetProcAddressA(HMODULE handle, const char* procName)
{
    if (handle == nullptr)
    {
        g_lastError = ERROR_CALL_NOT_IMPLEMENTED;
        return nullptr;
    }
    void* symbol = dlsym(handle, procName);
    if (symbol == nullptr)
    {
        g_lastError = ERROR_CALL_NOT_IMPLEMENTED;
        return nullptr;
    }
    return reinterpret_cast<FARPROC>(symbol);
}

BOOL FreeLibrary(HMODULE handle)
{
    if (handle == nullptr)
    {
        return FALSE;
    }
    return dlclose(handle) == 0 ? TRUE : FALSE;
}

int LoadStringA(HINSTANCE, unsigned int, char*, int)
{
    g_lastError = ERROR_CALL_NOT_IMPLEMENTED;
    return 0;
}

HRSRC FindResourceA(HINSTANCE, const char*, const char*)
{
    g_lastError = ERROR_CALL_NOT_IMPLEMENTED;
    return nullptr;
}

HGLOBAL LoadResource(HINSTANCE, HRSRC)
{
    g_lastError = ERROR_CALL_NOT_IMPLEMENTED;
    return nullptr;
}

void* LockResource(HGLOBAL)
{
    return nullptr;
}

DWORD SizeofResource(HINSTANCE, HRSRC)
{
    return 0;
}

DWORD GetTickCount()
{
    using namespace std::chrono;
    auto now = steady_clock::now().time_since_epoch();
    return static_cast<DWORD>(duration_cast<milliseconds>(now).count());
}

DWORD GetFileAttributesA(const char* fileName)
{
    if (fileName == nullptr)
    {
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return INVALID_FILE_ATTRIBUTES;
    }

    struct stat info{};
    if (stat(fileName, &info) != 0)
    {
        SetLastError(static_cast<DWORD>(errno));
        return INVALID_FILE_ATTRIBUTES;
    }

    DWORD attributes = 0;
    if (S_ISDIR(info.st_mode))
    {
        attributes |= FILE_ATTRIBUTE_DIRECTORY;
    }
    if ((info.st_mode & S_IWUSR) == 0)
    {
        attributes |= FILE_ATTRIBUTE_READONLY;
    }

    SetLastError(ERROR_SUCCESS);
    return attributes;
}

BOOL DeleteFileA(const char* fileName)
{
    if (fileName == nullptr)
    {
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE_VALUE;
    }

    if (std::remove(fileName) == 0)
    {
        SetLastError(ERROR_SUCCESS);
        return TRUE_VALUE;
    }

    SetLastError(static_cast<DWORD>(errno));
    return FALSE_VALUE;
}

BOOL GetFileTime(HANDLE file,
                 FILETIME* creationTime,
                 FILETIME* lastAccessTime,
                 FILETIME* lastWriteTime)
{
    if (file == nullptr)
    {
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE_VALUE;
    }

    FILE* stream = static_cast<FILE*>(file);
    int descriptor = fileno(stream);
    if (descriptor == -1)
    {
        SetLastError(static_cast<DWORD>(errno));
        return FALSE_VALUE;
    }

    struct stat info{};
    if (fstat(descriptor, &info) != 0)
    {
        SetLastError(static_cast<DWORD>(errno));
        return FALSE_VALUE;
    }

    if (creationTime != nullptr)
    {
        *creationTime = unixTimeToFileTime(info.st_ctime);
    }
    if (lastAccessTime != nullptr)
    {
        *lastAccessTime = unixTimeToFileTime(info.st_atime);
    }
    if (lastWriteTime != nullptr)
    {
        *lastWriteTime = unixTimeToFileTime(info.st_mtime);
    }

    SetLastError(ERROR_SUCCESS);
    return TRUE_VALUE;
}

DWORD GetFileVersionInfoSizeA(const char*, DWORD*)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return 0;
}

BOOL GetFileVersionInfoA(const char*, DWORD, DWORD, void*)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE_VALUE;
}

BOOL VerQueryValueA(const void*, const char*, void**, unsigned int*)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE_VALUE;
}

HGLOBAL GlobalAlloc(UINT flags, SIZE_T bytes)
{
    void* memory = nullptr;
    if (flags & GMEM_ZEROINIT)
    {
        memory = std::calloc(1, bytes);
    }
    else
    {
        memory = std::malloc(bytes);
    }

    if (memory == nullptr)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return nullptr;
    }

    SetLastError(ERROR_SUCCESS);
    return memory;
}

void* GlobalLock(HGLOBAL memory)
{
    return memory;
}

BOOL GlobalUnlock(HGLOBAL)
{
    return TRUE_VALUE;
}

HGLOBAL GlobalFree(HGLOBAL memory)
{
    std::free(memory);
    return nullptr;
}

int FindExecutableA(const char* file, const char* directory, char* result)
{
    if (result == nullptr)
    {
        return 1;
    }

    std::string path;
    if (file != nullptr)
    {
        path = file;
        if (!path.empty() && access(path.c_str(), X_OK) == 0)
        {
            std::snprintf(result, MAX_PATH, "%s", path.c_str());
            return 0;
        }
    }

    if (directory != nullptr && file != nullptr)
    {
        std::string combined = std::string(directory) + "/" + file;
        if (access(combined.c_str(), X_OK) == 0)
        {
            std::snprintf(result, MAX_PATH, "%s", combined.c_str());
            return 0;
        }
    }

    const char* pathEnv = std::getenv("PATH");
    if (pathEnv != nullptr && file != nullptr)
    {
        std::string current;
        for (const char* iter = pathEnv;; ++iter)
        {
            if (*iter == ':' || *iter == '\0')
            {
                if (!current.empty())
                {
                    std::string candidate = current + "/" + file;
                    if (access(candidate.c_str(), X_OK) == 0)
                    {
                        std::snprintf(result, MAX_PATH, "%s", candidate.c_str());
                        return 0;
                    }
                }
                current.clear();
                if (*iter == '\0')
                {
                    break;
                }
            }
            else
            {
                current.push_back(*iter);
            }
        }
    }

    if (result != nullptr)
    {
        result[0] = '\0';
    }
    g_lastError = ERROR_CALL_NOT_IMPLEMENTED;
    return 2;
}

DWORD FormatMessageA(DWORD,
                     const void*,
                     DWORD messageId,
                     DWORD,
                     char* buffer,
                     DWORD size,
                     va_list*)
{
    if (buffer == nullptr || size == 0)
    {
        return 0;
    }

    std::error_code ec(static_cast<int>(messageId), std::generic_category());
    std::string message = ec.message();
    if (message.empty())
    {
        message = "Unknown error";
    }

    std::snprintf(buffer, size, "%s (error %u)", message.c_str(), messageId);
    return static_cast<DWORD>(std::strlen(buffer));
}

DWORD FormatMessageW(DWORD flags,
                     const void* source,
                     DWORD messageId,
                     DWORD languageId,
                     wchar_t* buffer,
                     DWORD size,
                     va_list* arguments)
{
    if (buffer == nullptr || size == 0)
    {
        return 0;
    }

    char temp[1024];
    DWORD written = FormatMessageA(flags, source, messageId, languageId, temp, sizeof(temp), arguments);
    if (written == 0)
    {
        return 0;
    }

    std::mbstate_t state{};
    const char* src = temp;
    wchar_t* dst = buffer;
    size_t remaining = size - 1;
    while (remaining > 0)
    {
        wchar_t wc;
        size_t res = std::mbrtowc(&wc, src, std::strlen(src), &state);
        if (res == static_cast<size_t>(-1) || res == static_cast<size_t>(-2))
        {
            break;
        }
        if (res == 0)
        {
            break;
        }
        *dst++ = wc;
        src += res;
        --remaining;
    }
    *dst = L'\0';
    return static_cast<DWORD>(dst - buffer);
}

void LocalFree(void* memory)
{
    std::free(memory);
}

void OutputDebugStringA(LPCSTR message)
{
    if (message != nullptr)
    {
        std::fprintf(stderr, "%s", message);
    }
}

HMODULE GetModuleHandleA(LPCSTR)
{
    return nullptr;
}

DWORD GetModuleFileNameA(HMODULE, LPSTR buffer, DWORD size)
{
    if (buffer == nullptr || size == 0)
    {
        return 0;
    }

    ssize_t len = readlink("/proc/self/exe", buffer, size - 1);
    if (len == -1)
    {
        buffer[0] = '\0';
        return 0;
    }
    buffer[len] = '\0';
    return static_cast<DWORD>(len);
}

} // namespace cnc::windows

#endif // !_WIN32
