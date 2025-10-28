/*
**      Command & Conquer Generals(tm)
**      Copyright 2025 Electronic Arts Inc.
**
**      This program is free software: you can redistribute it and/or modify
**      it under the terms of the GNU General Public License as published by
**      the Free Software Foundation, either version 3 of the License, or
**      (at your option) any later version.
**
**      This program is distributed in the hope that it will be useful,
**      but WITHOUT ANY WARRANTY; without even the implied warranty of
**      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**      GNU General Public License for more details.
**
**      You should have received a copy of the GNU General Public License
**      along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GENERALS_CODE_LIBRARIES_SOURCE_WWVEGAS_WWLIB_SFML_MESSAGE_PUMP_H
#define GENERALS_CODE_LIBRARIES_SOURCE_WWVEGAS_WWLIB_SFML_MESSAGE_PUMP_H


#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#if __has_include(<SFML/Window/ContextSettings.hpp>)
#define WWLIB_HAS_SFML 1
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Window.hpp>
#else
#define WWLIB_HAS_SFML 0

namespace sf {

using Int32 = std::int32_t;
using Uint32 = std::uint32_t;

struct ContextSettings {
};

struct VideoMode {
        unsigned int width = 0;
        unsigned int height = 0;
        unsigned int bitsPerPixel = 32;

        constexpr VideoMode() = default;
        constexpr VideoMode(unsigned int w, unsigned int h, unsigned int bpp = 32)
                : width(w), height(h), bitsPerPixel(bpp) {}
};

struct Vector2u {
        unsigned int x = 0;
        unsigned int y = 0;
};

struct Vector2i {
        int x = 0;
        int y = 0;

        constexpr Vector2i() = default;
        constexpr Vector2i(int x_value, int y_value) : x(x_value), y(y_value) {}
};

struct Event {
        enum EventType {
                Closed,
                Resized,
                LostFocus,
                GainedFocus,
                KeyPressed,
                KeyReleased,
                MouseButtonPressed,
                MouseButtonReleased,
                MouseMoved,
                Count,
        };

        struct KeyEvent {
                int code = 0;
                bool alt = false;
                bool control = false;
                bool shift = false;
        };

        struct SizeEvent {
                unsigned int width = 0;
                unsigned int height = 0;
        };

        struct MouseMoveEvent {
                int x = 0;
                int y = 0;
        };

        struct MouseButtonEvent {
                int button = 0;
                int x = 0;
                int y = 0;
        };

        EventType type = Closed;
        KeyEvent key {};
        SizeEvent size {};
        MouseMoveEvent mouseMove {};
        MouseButtonEvent mouseButton {};
};

struct Keyboard {
        enum Key {
                Unknown = -1,
        };
};

struct Mouse {
        enum Button {
                Left,
                Right,
                Middle,
                XButton1,
                XButton2,
        };

        static Vector2i getPosition(const class Window &) { return {}; }
        static void setPosition(const Vector2i &, const class Window &) {}
};

struct Style {
        static constexpr Uint32 Default = 0;
};

class Window {
public:
        Window() = default;
        Window(const VideoMode &, const std::string &, Uint32, const ContextSettings &) {}

        bool hasFocus() const { return true; }
        void close() {}
        bool pollEvent(Event &) { return false; }
        Vector2u getSize() const { return {}; }
        Vector2i getPosition() const { return {}; }
};

} // namespace sf

#endif

namespace WWLib {

struct AcceleratorKey {
        sf::Keyboard::Key key = sf::Keyboard::Unknown;
        bool control = false;
        bool alt = false;
        bool shift = false;

        bool Matches(const sf::Event::KeyEvent &key_event) const;
};

class SfmlMessagePump {
public:
        using AcceleratorCallback = std::function<void()>;
        using DialogHook = std::function<bool(const sf::Event &)>;
        using EventListener = std::function<void(const sf::Event &)>;
        using Interceptor = std::function<bool(const sf::Event &)>;

        using AcceleratorId = std::uint64_t;
        using DialogHookId = std::uint64_t;
        using EventListenerId = std::uint64_t;
        using InterceptorId = std::uint64_t;

        struct WindowOptions {
                sf::VideoMode video_mode {800, 600, 32};
                std::string title {"WWLib"};
                sf::Uint32 style = sf::Style::Default;
                sf::ContextSettings context {};
        };

        SfmlMessagePump();
        explicit SfmlMessagePump(const WindowOptions &options);
        explicit SfmlMessagePump(std::unique_ptr<sf::Window> window);

        sf::Window &GetWindow();
        const sf::Window &GetWindow() const;

        void PumpEvents();

        AcceleratorId RegisterAccelerator(const AcceleratorKey &key, AcceleratorCallback callback);
        void UnregisterAccelerator(AcceleratorId id);

        DialogHookId AddDialogHook(DialogHook hook);
        void RemoveDialogHook(DialogHookId id);

        InterceptorId PushInterceptor(Interceptor interceptor);
        void RemoveInterceptor(InterceptorId id);

        EventListenerId AddEventListener(EventListener listener);
        void RemoveEventListener(EventListenerId id);

        void SetDialogFocus(bool has_focus);
        bool HasDialogFocus() const;

        bool HasFocus() const;

private:
        struct AcceleratorEntry {
                AcceleratorKey key;
                AcceleratorCallback callback;
        };

        struct DialogEntry {
                DialogHookId id = 0;
                DialogHook hook;
        };

        struct InterceptorEntry {
                InterceptorId id = 0;
                Interceptor hook;
        };

        struct ListenerEntry {
                EventListenerId id = 0;
                EventListener listener;
        };

        void EnsureWindow() const;
        bool DispatchAccelerators(const sf::Event &event);
        bool DispatchDialogHooks(const sf::Event &event);
        bool DispatchInterceptors(const sf::Event &event);
        void NotifyListeners(const sf::Event &event);

        std::unique_ptr<sf::Window> m_window;
        bool m_dialog_focus_override = false;
        bool m_has_focus = false;

        std::uint64_t m_next_accelerator_id = 1;
        std::uint64_t m_next_dialog_id = 1;
        std::uint64_t m_next_interceptor_id = 1;
        std::uint64_t m_next_listener_id = 1;

        std::unordered_map<AcceleratorId, AcceleratorEntry> m_accelerators;
        std::vector<DialogEntry> m_dialog_hooks;
        std::vector<InterceptorEntry> m_interceptors;
        std::vector<ListenerEntry> m_listeners;
};

} // namespace WWLib

#endif // GENERALS_CODE_LIBRARIES_SOURCE_WWVEGAS_WWLIB_SFML_MESSAGE_PUMP_H
