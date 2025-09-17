#include "WindowSystem.h"

#include "SfmlKeyboardBridge.h"
#include "SfmlMouseBridge.h"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowStyle.hpp>
#include <SFML/System/Clock.hpp>
#include <algorithm>

namespace sfml_platform {

namespace {

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

} // namespace sfml_platform
