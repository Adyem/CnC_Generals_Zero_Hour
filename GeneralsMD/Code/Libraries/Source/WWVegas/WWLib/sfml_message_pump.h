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

#ifndef GENERALSMD_CODE_LIBRARIES_SOURCE_WWVEGAS_WWLIB_SFML_MESSAGE_PUMP_H
#define GENERALSMD_CODE_LIBRARIES_SOURCE_WWVEGAS_WWLIB_SFML_MESSAGE_PUMP_H


#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Window.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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

#endif // GENERALSMD_CODE_LIBRARIES_SOURCE_WWVEGAS_WWLIB_SFML_MESSAGE_PUMP_H
