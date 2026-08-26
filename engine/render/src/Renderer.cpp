#include "CncRender/Renderer.hpp"

#include "errno.hpp"

namespace cnc
{

const char *HeadlessRenderer::backend_name() const noexcept
{
    return "headless";
}

Error HeadlessRenderer::initialize() noexcept
{
    if (_initialized == FT_TRUE) return FT_ERR_ALREADY_INITIALISED;
    _initialized = FT_TRUE;
    return FT_ERR_SUCCESS;
}

Error HeadlessRenderer::begin_frame() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_INVALID_STATE;
    if (_frame_open == FT_TRUE) return FT_ERR_INVALID_OPERATION;
    _commands.clear();
    _last_snapshot.clear();
    _frame_open = FT_TRUE;
    return FT_ERR_SUCCESS;
}

Error HeadlessRenderer::submit(const RenderCommand &command) noexcept
{
    if (_initialized != FT_TRUE || _frame_open != FT_TRUE)
        return FT_ERR_INVALID_STATE;
    if (command.resource_id == 0U || command.width == 0U || command.height == 0U)
        return FT_ERR_INVALID_ARGUMENT;
    try
    {
        _commands.push_back(command);
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error HeadlessRenderer::present_snapshot(
    const std::vector<WorldSnapshotEntry> &snapshot) noexcept
{
    if (_initialized != FT_TRUE || _frame_open != FT_TRUE)
        return FT_ERR_INVALID_STATE;
    try
    {
        _last_snapshot = snapshot;
    }
    catch (...)
    {
        _last_snapshot.clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error HeadlessRenderer::end_frame() noexcept
{
    if (_initialized != FT_TRUE || _frame_open != FT_TRUE)
        return FT_ERR_INVALID_STATE;
    ++_frame_count;
    _frame_open = FT_FALSE;
    return FT_ERR_SUCCESS;
}

Error HeadlessRenderer::shutdown() noexcept
{
    _commands.clear();
    _last_snapshot.clear();
    _frame_open = FT_FALSE;
    _initialized = FT_FALSE;
    return FT_ERR_SUCCESS;
}

Bool HeadlessRenderer::is_initialized() const noexcept { return _initialized; }
uint64_t HeadlessRenderer::frame_count() const noexcept { return _frame_count; }
Size HeadlessRenderer::submitted_command_count() const noexcept
{
    return static_cast<Size>(_commands.size());
}

}
