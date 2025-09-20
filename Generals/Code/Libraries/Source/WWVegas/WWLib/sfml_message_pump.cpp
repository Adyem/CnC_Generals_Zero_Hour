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

#include "sfml_message_pump.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <utility>

namespace WWLib {

bool AcceleratorKey::Matches(const sf::Event::KeyEvent &key_event) const
{
        return key == key_event.code && control == key_event.control && alt == key_event.alt && shift == key_event.shift;
}

SfmlMessagePump::SfmlMessagePump() : SfmlMessagePump(WindowOptions{})
{
}

SfmlMessagePump::SfmlMessagePump(const WindowOptions &options)
        : SfmlMessagePump(std::make_unique<sf::Window>(options.video_mode, options.title, options.style, options.context))
{
}

SfmlMessagePump::SfmlMessagePump(std::unique_ptr<sf::Window> window)
        : m_window(std::move(window))
{
        EnsureWindow();
        m_has_focus = m_window->hasFocus();
}

sf::Window &SfmlMessagePump::GetWindow()
{
        EnsureWindow();
        return *m_window;
}

const sf::Window &SfmlMessagePump::GetWindow() const
{
        EnsureWindow();
        return *m_window;
}

void SfmlMessagePump::PumpEvents()
{
        EnsureWindow();

        sf::Event event;
        while (m_window->pollEvent(event)) {
                switch (event.type) {
                case sf::Event::Closed:
                        m_window->close();
                        break;
                case sf::Event::GainedFocus:
                        m_has_focus = true;
                        break;
                case sf::Event::LostFocus:
                        m_has_focus = false;
                        break;
                default:
                        break;
                }

                if (DispatchAccelerators(event)) {
                        continue;
                }

                if (DispatchDialogHooks(event)) {
                        continue;
                }

                if (DispatchInterceptors(event)) {
                        continue;
                }

                NotifyListeners(event);
        }
}

SfmlMessagePump::AcceleratorId SfmlMessagePump::RegisterAccelerator(const AcceleratorKey &key, AcceleratorCallback callback)
{
        AcceleratorId id = m_next_accelerator_id++;
        m_accelerators.emplace(id, AcceleratorEntry{key, std::move(callback)});
        return id;
}

void SfmlMessagePump::UnregisterAccelerator(AcceleratorId id)
{
        m_accelerators.erase(id);
}

SfmlMessagePump::DialogHookId SfmlMessagePump::AddDialogHook(DialogHook hook)
{
        DialogHookId id = m_next_dialog_id++;
        m_dialog_hooks.push_back(DialogEntry{id, std::move(hook)});
        return id;
}

void SfmlMessagePump::RemoveDialogHook(DialogHookId id)
{
        auto end = std::remove_if(m_dialog_hooks.begin(), m_dialog_hooks.end(), [id](const DialogEntry &entry) { return entry.id == id; });
        m_dialog_hooks.erase(end, m_dialog_hooks.end());
}

SfmlMessagePump::InterceptorId SfmlMessagePump::PushInterceptor(Interceptor interceptor)
{
        InterceptorId id = m_next_interceptor_id++;
        m_interceptors.push_back(InterceptorEntry{id, std::move(interceptor)});
        return id;
}

void SfmlMessagePump::RemoveInterceptor(InterceptorId id)
{
        auto end = std::remove_if(m_interceptors.begin(), m_interceptors.end(), [id](const InterceptorEntry &entry) { return entry.id == id; });
        m_interceptors.erase(end, m_interceptors.end());
}

SfmlMessagePump::EventListenerId SfmlMessagePump::AddEventListener(EventListener listener)
{
        EventListenerId id = m_next_listener_id++;
        m_listeners.push_back(ListenerEntry{id, std::move(listener)});
        return id;
}

void SfmlMessagePump::RemoveEventListener(EventListenerId id)
{
        auto end = std::remove_if(m_listeners.begin(), m_listeners.end(), [id](const ListenerEntry &entry) { return entry.id == id; });
        m_listeners.erase(end, m_listeners.end());
}

void SfmlMessagePump::SetDialogFocus(bool has_focus)
{
        m_dialog_focus_override = has_focus;
}

bool SfmlMessagePump::HasDialogFocus() const
{
        return m_dialog_focus_override || !m_dialog_hooks.empty();
}

bool SfmlMessagePump::HasFocus() const
{
        return m_has_focus;
}

void SfmlMessagePump::EnsureWindow() const
{
        if (!m_window) {
                throw std::runtime_error("SfmlMessagePump requires a valid sf::Window instance");
        }
}

bool SfmlMessagePump::DispatchAccelerators(const sf::Event &event)
{
        if (event.type != sf::Event::KeyPressed) {
                return false;
        }

        for (const auto &pair : m_accelerators) {
                const AcceleratorEntry &entry = pair.second;
                if (entry.key.Matches(event.key)) {
                        if (entry.callback) {
                                entry.callback();
                        }
                        return true;
                }
        }

        return false;
}

bool SfmlMessagePump::DispatchDialogHooks(const sf::Event &event)
{
        for (auto iterator = m_dialog_hooks.rbegin(); iterator != m_dialog_hooks.rend(); ++iterator) {
                if (iterator->hook && iterator->hook(event)) {
                        return true;
                }
        }

        return false;
}

bool SfmlMessagePump::DispatchInterceptors(const sf::Event &event)
{
        for (auto iterator = m_interceptors.rbegin(); iterator != m_interceptors.rend(); ++iterator) {
                if (iterator->hook && iterator->hook(event)) {
                        return true;
                }
        }

        return false;
}

void SfmlMessagePump::NotifyListeners(const sf::Event &event)
{
        for (const auto &entry : m_listeners) {
                if (entry.listener) {
                        entry.listener(event);
                }
        }
}

} // namespace WWLib

