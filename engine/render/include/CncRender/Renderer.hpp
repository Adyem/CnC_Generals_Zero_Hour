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

class Renderer
{
public:
    virtual ~Renderer() noexcept = default;
    virtual Error initialize() noexcept = 0;
    virtual const char *backend_name() const noexcept = 0;
    virtual Error begin_frame() noexcept = 0;
    virtual Error submit(const RenderCommand &command) noexcept = 0;
    virtual Error end_frame() noexcept = 0;
    virtual Error shutdown() noexcept = 0;
    virtual Bool is_initialized() const noexcept = 0;
    virtual uint64_t frame_count() const noexcept = 0;
    virtual Size submitted_command_count() const noexcept = 0;
};

class HeadlessRenderer final : public Renderer
{
public:
    Error initialize() noexcept override;
    const char *backend_name() const noexcept override;
    Error begin_frame() noexcept override;
    Error submit(const RenderCommand &command) noexcept override;
    Error end_frame() noexcept override;
    Error shutdown() noexcept override;
    Bool is_initialized() const noexcept override;
    uint64_t frame_count() const noexcept override;
    Size submitted_command_count() const noexcept override;

private:
    std::vector<RenderCommand> _commands;
    uint64_t _frame_count = 0U;
    Bool _initialized = FT_FALSE;
    Bool _frame_open = FT_FALSE;
};

}
