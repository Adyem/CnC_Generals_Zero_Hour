#include "WindowSystem.h"
#include "SfmlAudioManager.h"
#include "SfmlKeyboardBridge.h"
#include "SfmlMouseBridge.h"

#include <Common/CriticalSection.h>
#include <Common/StackDump.h>
#include <GameClient/Keyboard.h>
#include <GameClient/Mouse.h>
#include <Common/GameEngine.h>
#include <Common/GameAudio.h>
#include <Win32Device/Common/Win32GameEngine.h>
#include <W3DDevice/GameClient/RenderBackend.h>

#include <Main/WinMain.h>
#include <Main/EntryPointLifecycle.h>

#include <SFML/Graphics/RenderWindow.hpp>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <limits>
#include <optional>
#include <system_error>
#include <string>
#include <vector>

#ifdef _WIN32
#include <eh.h>
#include <windows.h>
#endif

using sfml_platform::WindowConfig;
using sfml_platform::WindowSystem;

namespace {

constexpr unsigned int kDefaultWidth = 1280;
constexpr unsigned int kDefaultHeight = 720;

enum class BackendOption {
    Direct3D8,
    OpenGL,
    Bgfx,
};

struct ParsedArguments {
    WindowConfig config;
    BackendOption backend = BackendOption::Direct3D8;
    unsigned int bitDepth = 32;
    bool helpRequested = false;
    bool useLegacyMilesAudio = false;
};

CriticalSection critSec1;
CriticalSection critSec2;
CriticalSection critSec3;
CriticalSection critSec4;
CriticalSection critSec5;

WindowSystem* g_activeWindowSystem = nullptr;

class SfmlGameEngine : public Win32GameEngine {
public:
    SfmlGameEngine() = default;
    ~SfmlGameEngine() override = default;
};

GameEngine* CreateSfmlGameEngine() { return NEW SfmlGameEngine; }

std::vector<std::string> toArgumentVector(int argc, char** argv) {
    std::vector<std::string> args;
    args.reserve(static_cast<std::size_t>(argc));
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i] ? argv[i] : "");
    }
    return args;
}

AudioManager* CreateSfmlAudioManager() { return NEW SfmlAudioManager; }

void printHelp() {
    std::cout << "Command & Conquer Generals (SFML bootstrap)\n"
              << "Options:\n"
              << "  -h, --help           Show this message\n"
              << "  -w, --width <value>  Set the window width (default " << kDefaultWidth << ")\n"
              << "  -H, --height <value> Set the window height (default " << kDefaultHeight << ")\n"
              << "  -f, --fullscreen     Start in fullscreen mode\n"
              << "      --windowed       Force windowed mode\n"
              << "      --vsync          Enable vertical sync (default)\n"
              << "      --no-vsync       Disable vertical sync\n"
              << "      --fps <value>    Limit frames per second (0 = uncapped)\n"
              << "      --opengl         Use the OpenGL renderer\n"
              << "      --bgfx           Use the bgfx renderer\n"
              << "      --d3d            Use the Direct3D 8 renderer (default)\n"
              << "      --bitdepth <n>   Set color bit depth (default 32)\n";
              << "      --legacy-miles-audio Use legacy Miles audio backend\n";
}

std::optional<unsigned int> parseUnsigned(const std::string& value) {
    if (value.empty()) {
        return std::nullopt;
    }

    try {
        unsigned long parsed = std::stoul(value);
        if (parsed > std::numeric_limits<unsigned int>::max()) {
            return std::nullopt;
        }
        return static_cast<unsigned int>(parsed);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

void processSfmlEvents() {
    if (g_activeWindowSystem == nullptr) {
        return;
    }

    sf::RenderWindow& window = g_activeWindowSystem->window();
    sf::Event event{};
    while (window.isOpen() && window.pollEvent(event)) {
        if (auto* keyboard = GetActiveKeyboardBridge()) {
            keyboard->handleEvent(event);
        }
        if (auto* mouse = GetActiveMouseBridge()) {
            mouse->handleEvent(event);
        }

        switch (event.type) {
            case sf::Event::Closed:
                window.close();
                if (TheGameEngine) {
                    TheGameEngine->checkAbnormalQuitting();
                    TheGameEngine->reset();
                    TheGameEngine->setQuitting(TRUE);
                }
                break;

            case sf::Event::LostFocus:
                if (TheKeyboard) {
                    TheKeyboard->resetKeys();
                }
                if (auto* mouse = GetActiveMouseBridge()) {
                    mouse->lostFocus(TRUE);
                }
                if (TheGameEngine) {
                    TheGameEngine->setIsActive(FALSE);
                }
#ifdef _WIN32
                GetRenderBackend().HandleFocusChange(false);
#endif
                if (TheAudio) {
                    TheAudio->loseFocus();
                }
                break;

            case sf::Event::GainedFocus:
                if (TheKeyboard) {
                    TheKeyboard->resetKeys();
                }
                if (auto* mouse = GetActiveMouseBridge()) {
                    mouse->lostFocus(FALSE);
                    mouse->refreshCursor();
                }
                if (TheGameEngine) {
                    TheGameEngine->setIsActive(TRUE);
                }
#ifdef _WIN32
                GetRenderBackend().HandleFocusChange(true);
#endif
                if (TheAudio) {
                    TheAudio->regainFocus();
                }
                break;

            case sf::Event::MouseEntered:
                if (auto* mouse = GetActiveMouseBridge()) {
                    mouse->refreshCursor();
                }
                break;

            default:
                break;
        }
    }

    if (!window.isOpen()) {
        if (TheGameEngine) {
            TheGameEngine->setQuitting(TRUE);
        }
        return;
    }

    if (auto* mouse = GetActiveMouseBridge()) {
        mouse->refreshCursor();
    }
}

ParsedArguments parseArguments(const std::vector<std::string>& arguments) {
    ParsedArguments result;
    result.config.width = kDefaultWidth;
    result.config.height = kDefaultHeight;

    for (std::size_t i = 1; i < arguments.size(); ++i) {
        const std::string& argument = arguments[i];

        auto consumeValue = [&](const char* longName, const char* shortName) -> std::optional<std::string> {
            if (argument == longName || (shortName != nullptr && argument == shortName)) {
                if (i + 1 < arguments.size()) {
                    return arguments[++i];
                }
                std::cerr << "Missing value for option '" << argument << "'\n";
                return std::nullopt;
            }
            return std::nullopt;
        };

        if (argument == "-h" || argument == "--help") {
            result.helpRequested = true;
            return result;
        }

        if (auto value = consumeValue("--width", "-w")) {
            if (auto parsed = parseUnsigned(*value)) {
                result.config.width = *parsed;
            } else {
                std::cerr << "Invalid width value: '" << *value << "'\n";
            }
            continue;
        }

        if (auto value = consumeValue("--height", "-H")) {
            if (auto parsed = parseUnsigned(*value)) {
                result.config.height = *parsed;
            } else {
                std::cerr << "Invalid height value: '" << *value << "'\n";
            }
            continue;
        }

        if (auto value = consumeValue("--fps", nullptr)) {
            if (auto parsed = parseUnsigned(*value)) {
                result.config.frameLimit = *parsed;
            } else {
                std::cerr << "Invalid FPS limit: '" << *value << "'\n";
            }
            continue;
        }

        if (auto value = consumeValue("--bitdepth", "-bpp")) {
            if (auto parsed = parseUnsigned(*value)) {
                result.bitDepth = *parsed;
            } else {
                std::cerr << "Invalid bit depth: '" << *value << "'\n";
            }
            continue;
        }

        if (argument == "-f" || argument == "--fullscreen") {
            result.config.fullscreen = true;
            continue;
        }

        if (argument == "--windowed" || argument == "-win" || argument == "-windowed" || argument == "-nowin") {
            result.config.fullscreen = false;
            continue;
        }

        if (argument == "--vsync") {
            result.config.verticalSync = true;
            continue;
        }

        if (argument == "--no-vsync" || argument == "-novsync") {
            result.config.verticalSync = false;
            continue;
        }

        if (argument == "--opengl" || argument == "-opengl") {
            result.backend = BackendOption::OpenGL;
            continue;
        }

        if (argument == "--bgfx") {
            result.backend = BackendOption::Bgfx;
            continue;
        }

        if (argument == "--d3d" || argument == "--direct3d" || argument == "-d3d") {
            result.backend = BackendOption::Direct3D8;
            continue;
        }

        if (argument == "--legacy-miles-audio") {
            result.useLegacyMilesAudio = true;
            continue;
        }

        std::cerr << "Unrecognized option: '" << argument << "'\n";
    }

    return result;
}

std::vector<char*> buildArgv(const std::vector<std::string>& arguments) {
    std::vector<char*> argvPointers;
    argvPointers.reserve(arguments.size());
    for (const std::string& argument : arguments) {
        argvPointers.push_back(const_cast<char*>(argument.c_str()));
    }
    return argvPointers;
}

void assignCriticalSections() {
    TheAsciiStringCriticalSection = &critSec1;
    TheUnicodeStringCriticalSection = &critSec2;
    TheDmaCriticalSection = &critSec3;
    TheMemoryPoolCriticalSection = &critSec4;
    TheDebugLogCriticalSection = &critSec5;
}

void clearCriticalSections() {
    TheAsciiStringCriticalSection = NULL;
    TheUnicodeStringCriticalSection = NULL;
    TheDmaCriticalSection = NULL;
    TheMemoryPoolCriticalSection = NULL;
    TheDebugLogCriticalSection = NULL;
}

GraphicsBackend toGraphicsBackend(BackendOption backend) {
    switch (backend) {
        case BackendOption::OpenGL:
            return GRAPHICS_BACKEND_OPENGL;
        case BackendOption::Bgfx:
            return GRAPHICS_BACKEND_BGFX;
        case BackendOption::Direct3D8:
        default:
            return GRAPHICS_BACKEND_DIRECT3D8;
    }
}

void setWorkingDirectory(const std::vector<std::string>& arguments) {
    if (arguments.empty()) {
        return;
    }

    std::error_code ec;
    std::filesystem::path executable = std::filesystem::absolute(arguments.front(), ec);
    if (ec) {
        return;
    }

    const auto parent = executable.parent_path();
    if (!parent.empty()) {
        std::filesystem::current_path(parent, ec);
    }
}

}  // namespace

int main(int argc, char** argv) {
    const auto arguments = toArgumentVector(argc, argv);
    ParsedArguments parsed = parseArguments(arguments);

    if (parsed.helpRequested) {
        printHelp();
        return 0;
    }

    setWorkingDirectory(arguments);

    WindowSystem windowSystem;
    if (!windowSystem.initialize(parsed.config)) {
        std::cerr << "Failed to create SFML window" << std::endl;
        return 1;
    }

#ifdef _WIN32
    ApplicationHInstance = GetModuleHandle(nullptr);
    ApplicationHWnd = reinterpret_cast<HWND>(windowSystem.window().getSystemHandle());
#else
    ApplicationHInstance = nullptr;
    ApplicationHWnd = nullptr;
#endif
    ApplicationIsWindowed = !parsed.config.fullscreen;
    gInitialEngineActiveState = windowSystem.window().hasFocus() ? TRUE : FALSE;
    g_activeWindowSystem = &windowSystem;

    SetWindowsMessagePumpOverride(processSfmlEvents);

    assignCriticalSections();

    EntryPointConfig entryPointConfig{toGraphicsBackend(parsed.backend)};
    EntryPointScope entryPointScope(entryPointConfig);

    std::vector<char*> argvPointers = buildArgv(arguments);

#ifdef _WIN32
    _set_se_translator(DumpExceptionInfo);
#endif

    SetKeyboardFactoryOverride(sfml_platform::CreateSfmlKeyboard);
    SetMouseFactoryOverride(sfml_platform::CreateSfmlMouse);
    SetGameEngineFactoryOverride(CreateSfmlGameEngine);

    if (parsed.useLegacyMilesAudio) {
        SetAudioManagerFactoryOverride(NULL);
    } else {
        SetAudioManagerFactoryOverride(CreateSfmlAudioManager);
    }

    try {
        GameMain(static_cast<int>(argvPointers.size()), argvPointers.data());
    } catch (...) {
        std::cerr << "Unhandled exception while running GameMain" << std::endl;
    }

    SetWindowsMessagePumpOverride(NULL);
    g_activeWindowSystem = nullptr;

    clearCriticalSections();

    windowSystem.shutdown();

    return 0;
}

#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    return main(__argc, __argv);
}
#endif
