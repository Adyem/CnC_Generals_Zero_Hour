#pragma once

#include <cstdint>
#include <vector>

#include "CncSimulation/SpatialIndex.hpp"

namespace cnc
{

struct GridCell
{
    uint32_t x = 0U;
    uint32_t y = 0U;
};

class GridPathfinder final
{
public:
    static constexpr ft_size_t max_cells = static_cast<ft_size_t>(1U << 20U);

    Error initialize(uint32_t width, uint32_t height) noexcept;
    Error shutdown() noexcept;
    Error set_blocked(uint32_t x, uint32_t y, Bool blocked) noexcept;
    Error is_blocked(uint32_t x, uint32_t y, Bool *blocked_out) const noexcept;
    Error find_path(GridCell start, GridCell goal,
                    std::vector<GridCell> *path_out) const noexcept;
    uint32_t width() const noexcept;
    uint32_t height() const noexcept;

private:
    uint32_t _width = 0U;
    uint32_t _height = 0U;
    std::vector<Bool> _blocked;
    Bool _initialized = FT_FALSE;

    ft_size_t index(uint32_t x, uint32_t y) const noexcept;
    Bool valid(uint32_t x, uint32_t y) const noexcept;
};

}
