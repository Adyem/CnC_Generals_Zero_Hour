#pragma once

#include <cstdint>
#include <vector>

#include "CncRuntime/Types.hpp"

namespace cnc
{

struct RenderCommand
{
    uint32_t resource_id = 0U;
    int32_t x = 0;
    int32_t y = 0;
    uint32_t width = 0U;
    uint32_t height = 0U;
};

class HeadlessRenderer final
{
public:
    Error initialize() noexcept;
    Error begin_frame() noexcept;
    Error submit(const RenderCommand &command) noexcept;
    Error end_frame() noexcept;
    Error shutdown() noexcept;
    Bool is_initialized() const noexcept;
    uint64_t frame_count() const noexcept;
    Size submitted_command_count() const noexcept;

private:
    std::vector<RenderCommand> _commands;
    uint64_t _frame_count = 0U;
    Bool _initialized = FT_FALSE;
    Bool _frame_open = FT_FALSE;
};

}
