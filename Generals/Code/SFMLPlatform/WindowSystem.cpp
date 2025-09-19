#include "WindowSystem.h"

#include "SfmlKeyboardBridge.h"
#include "SfmlMouseBridge.h"

#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowHandle.hpp>
#include <SFML/Window/WindowStyle.hpp>
#include <cstdint>
#include <algorithm>

#if defined(SFML_SYSTEM_WINDOWS)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <SFML/OpenGL.hpp>
#elif defined(SFML_SYSTEM_LINUX)
#include <SFML/OpenGL.hpp>
#include <X11/Xlib.h>
#include <GL/glx.h>
#endif

namespace sfml_platform {

namespace {

#if defined(SFML_SYSTEM_LINUX)
Display* acquireX11Display() {
    static Display* display = nullptr;
    if (display == nullptr) {
        display = XOpenDisplay(nullptr);
    }
    return display;
}

void destroyX11Window(void* window) {
    if (window == nullptr) {
        return;
    }

    Display* display = acquireX11Display();
    if (display == nullptr) {
        return;
    }

    ::Window nativeWindow = static_cast<::Window>(reinterpret_cast<std::uintptr_t>(window));
    XDestroyWindow(display, nativeWindow);
    XFlush(display);
}
#elif defined(SFML_SYSTEM_WINDOWS)
void destroyWin32Window(void* window) {
    if (window == nullptr) {
        return;
    }

    HWND hwnd = static_cast<HWND>(window);
    PostMessage(hwnd, WM_CLOSE, 0, 0);
}
#endif

sf::VideoMode pickVideoMode(unsigned int width, unsigned int height, bool fullscreen) {
    if (!fullscreen) {
        return sf::VideoMode(width, height);
    }

    const auto modes = sf::VideoMode::getFullscreenModes();
    auto it = std::find_if(modes.begin(), modes.end(), [width, height](const sf::VideoMode& mode) {
        return mode.width == width && mode.height == height;
    });

    if (it != modes.end()) {
        return *it;
    }

    return modes.empty() ? sf::VideoMode(width, height) : modes.front();
}

} // namespace

WindowSystem::WindowSystem() = default;
WindowSystem::~WindowSystem() = default;

bool WindowSystem::initialize(const WindowConfig& config) {
    m_config = config;

    const auto style = config.fullscreen ? sf::Style::Fullscreen : sf::Style::Default;
    const auto videoMode = pickVideoMode(config.width, config.height, config.fullscreen);

    m_window.create(videoMode, config.title, style);
    configureRenderSettings(config);
    m_window.setActive(true);
    m_running = m_window.isOpen();

    return m_running;
}

void WindowSystem::configureRenderSettings(const WindowConfig& config) {
    m_window.setVerticalSyncEnabled(config.verticalSync);

    if (config.frameLimit > 0) {
        m_window.setFramerateLimit(config.frameLimit);
    } else {
        m_window.setFramerateLimit(0);
    }
}

void WindowSystem::run(const UpdateHandler& update, const RenderHandler& render, const EventHandler& onEvent) {
    sf::Clock frameClock;

    while (m_running && m_window.isOpen()) {
        sf::Event event{};
        while (m_window.pollEvent(event)) {
            if (auto* keyboard = GetActiveKeyboardBridge()) {
                keyboard->handleEvent(event);
            }

            if (auto* mouse = GetActiveMouseBridge()) {
                mouse->handleEvent(event);

                if (event.type == sf::Event::LostFocus) {
                    mouse->lostFocus(TRUE);
                } else if (event.type == sf::Event::GainedFocus) {
                    mouse->lostFocus(FALSE);
                    mouse->refreshCursor();
                } else if (event.type == sf::Event::MouseEntered) {
                    mouse->refreshCursor();
                }
            }

            if (event.type == sf::Event::Closed) {
                m_window.close();
            }

            if (onEvent) {
                onEvent(event);
            }
        }

        const float delta = frameClock.restart().asSeconds();

        if (update) {
            update(m_window, delta);
        }

        if (render) {
            render(m_window);
        } else {
            m_window.clear();
            m_window.display();
        }

        m_running = m_window.isOpen();
    }
}

void WindowSystem::shutdown() {
    if (m_window.isOpen()) {
        m_window.close();
    }
    m_running = false;
}

sf::RenderWindow& WindowSystem::window() {
    return m_window;
}

const sf::RenderWindow& WindowSystem::window() const {
    return m_window;
}

NativeWindowHandle WindowSystem::nativeHandle() const {
    NativeWindowHandle handle{};

    const_cast<sf::RenderWindow&>(m_window).setActive(true);

    const sf::WindowHandle systemHandle = m_window.getSystemHandle();
#if defined(SFML_SYSTEM_WINDOWS) || defined(SFML_SYSTEM_MACOS) || defined(SFML_SYSTEM_IOS)
    handle.window = reinterpret_cast<void*>(systemHandle);
#else
    handle.window = reinterpret_cast<void*>(static_cast<std::uintptr_t>(systemHandle));
#endif

#if defined(SFML_SYSTEM_LINUX)
    handle.display = acquireX11Display();
    handle.context = reinterpret_cast<void*>(glXGetCurrentContext());
    handle.backBuffer = reinterpret_cast<void*>(glXGetCurrentDrawable());
    handle.destroyWindow = destroyX11Window;
#elif defined(SFML_SYSTEM_WINDOWS)
    handle.context = reinterpret_cast<void*>(wglGetCurrentContext());
    handle.backBuffer = reinterpret_cast<void*>(wglGetCurrentDC());
    handle.destroyWindow = destroyWin32Window;
#else
    handle.display = nullptr;
#endif

    return handle;
}

} // namespace sfml_platform
