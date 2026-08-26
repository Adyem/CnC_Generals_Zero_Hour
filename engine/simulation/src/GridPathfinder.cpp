#include "CncSimulation/GridPathfinder.hpp"

#include <algorithm>

#include "errno.hpp"

namespace cnc
{

Error GridPathfinder::initialize(uint32_t width_value, uint32_t height_value) noexcept
{
    if (_initialized == FT_TRUE) return FT_ERR_ALREADY_INITIALISED;
    if (width_value == 0U || height_value == 0U ||
        static_cast<uint64_t>(width_value) * static_cast<uint64_t>(height_value) > max_cells)
        return FT_ERR_INVALID_ARGUMENT;
    try
    {
        _blocked.assign(static_cast<ft_size_t>(width_value) * height_value, FT_FALSE);
    }
    catch (...) { return FT_ERR_NO_MEMORY; }
    _width = width_value;
    _height = height_value;
    _initialized = FT_TRUE;
    return FT_ERR_SUCCESS;
}

Error GridPathfinder::shutdown() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_SUCCESS;
    _blocked.clear();
    _width = 0U;
    _height = 0U;
    _initialized = FT_FALSE;
    return FT_ERR_SUCCESS;
}

Bool GridPathfinder::valid(uint32_t x, uint32_t y) const noexcept
{
    return (x < _width && y < _height) ? FT_TRUE : FT_FALSE;
}

ft_size_t GridPathfinder::index(uint32_t x, uint32_t y) const noexcept
{
    return static_cast<ft_size_t>(y) * _width + x;
}

Error GridPathfinder::set_blocked(uint32_t x, uint32_t y, Bool blocked) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (valid(x, y) != FT_TRUE || (blocked != FT_TRUE && blocked != FT_FALSE))
        return FT_ERR_INVALID_ARGUMENT;
    _blocked[index(x, y)] = blocked;
    return FT_ERR_SUCCESS;
}

Error GridPathfinder::is_blocked(uint32_t x, uint32_t y, Bool *blocked_out) const noexcept
{
    if (blocked_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (valid(x, y) != FT_TRUE) return FT_ERR_INVALID_ARGUMENT;
    *blocked_out = _blocked[index(x, y)];
    return FT_ERR_SUCCESS;
}

Error GridPathfinder::find_path(GridCell start, GridCell goal,
                                std::vector<GridCell> *path_out) const noexcept
{
    if (path_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (valid(start.x, start.y) != FT_TRUE || valid(goal.x, goal.y) != FT_TRUE)
        return FT_ERR_INVALID_ARGUMENT;
    try
    {
        path_out->clear();
        if (_blocked[index(start.x, start.y)] == FT_TRUE ||
            _blocked[index(goal.x, goal.y)] == FT_TRUE)
            return FT_ERR_NOT_FOUND;
        const ft_size_t start_index = index(start.x, start.y);
        const ft_size_t goal_index = index(goal.x, goal.y);
        std::vector<ft_size_t> queue;
        std::vector<ft_size_t> parent(_blocked.size(), max_cells);
        queue.reserve(_blocked.size());
        queue.push_back(start_index);
        parent[start_index] = start_index;
        const int32_t directions[4][2] = {{0, -1}, {-1, 0}, {1, 0}, {0, 1}};
        for (ft_size_t head = 0U; head < queue.size() && parent[goal_index] == max_cells; ++head)
        {
            const ft_size_t current = queue[head];
            const uint32_t current_x = static_cast<uint32_t>(current % _width);
            const uint32_t current_y = static_cast<uint32_t>(current / _width);
            for (const auto &direction : directions)
            {
                const int64_t next_x = static_cast<int64_t>(current_x) + direction[0];
                const int64_t next_y = static_cast<int64_t>(current_y) + direction[1];
                if (next_x < 0 || next_y < 0 || next_x >= _width || next_y >= _height)
                    continue;
                const uint32_t x = static_cast<uint32_t>(next_x);
                const uint32_t y = static_cast<uint32_t>(next_y);
                const ft_size_t next = index(x, y);
                if (_blocked[next] == FT_TRUE || parent[next] != max_cells) continue;
                parent[next] = current;
                queue.push_back(next);
            }
        }
        if (parent[goal_index] == max_cells) return FT_ERR_NOT_FOUND;
        for (ft_size_t current = goal_index;; current = parent[current])
        {
            path_out->push_back(GridCell{
                static_cast<uint32_t>(current % _width), static_cast<uint32_t>(current / _width)});
            if (current == start_index) break;
        }
        std::reverse(path_out->begin(), path_out->end());
    }
    catch (...)
    {
        path_out->clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

uint32_t GridPathfinder::width() const noexcept { return _width; }
uint32_t GridPathfinder::height() const noexcept { return _height; }

}
