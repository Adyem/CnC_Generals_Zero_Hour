
#include <string>

namespace sfml_platform {

struct WindowConfig {
    unsigned int width = 1280;
    unsigned int height = 720;
    bool fullscreen = false;
    bool verticalSync = true;
    unsigned int frameLimit = 0;
    std::string title = "Command & Conquer Generals";
};

} // namespace sfml_platform
