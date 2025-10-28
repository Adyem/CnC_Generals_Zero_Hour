
#include "WindowConfig.h"

#include <SFML/Graphics.hpp>
#include <functional>

namespace sfml_platform {

struct NativeWindowHandle {
    void* display = nullptr;
    void* window = nullptr;
    void* context = nullptr;
    void* backBuffer = nullptr;
    void* backBufferDepth = nullptr;
    void (*destroyWindow)(void*) = nullptr;
};

class WindowSystem {
public:
    using EventHandler = std::function<void(const sf::Event&)>;
    using UpdateHandler = std::function<void(sf::RenderWindow&, float)>;
    using RenderHandler = std::function<void(sf::RenderWindow&)>;

    WindowSystem();
    ~WindowSystem();

    bool initialize(const WindowConfig& config);
    void run(const UpdateHandler& update, const RenderHandler& render, const EventHandler& onEvent);
    void shutdown();

    sf::RenderWindow& window();
    const sf::RenderWindow& window() const;

    NativeWindowHandle nativeHandle() const;

private:
    void configureRenderSettings(const WindowConfig& config);

    WindowConfig m_config;
    sf::RenderWindow m_window;
    bool m_running = false;
};

void SetActiveWindowSystem(WindowSystem* system);
WindowSystem* GetActiveWindowSystem();

} // namespace sfml_platform
