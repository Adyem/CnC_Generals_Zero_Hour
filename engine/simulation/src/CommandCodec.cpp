#include "CncSimulation/CommandCodec.hpp"

#include <cstring>
#include <limits>

#include "errno.hpp"

namespace cnc
{
namespace
{
constexpr ft_size_t header_size = 20U;
constexpr ft_size_t command_size = 24U;

void append_u32(std::vector<uint8_t> &bytes, uint32_t value)
{
    for (uint32_t shift = 0U; shift < 32U; shift += 8U)
        bytes.push_back(static_cast<uint8_t>((value >> shift) & 0xFFU));
}
void append_u64(std::vector<uint8_t> &bytes, uint64_t value)
{
    for (uint32_t shift = 0U; shift < 64U; shift += 8U)
        bytes.push_back(static_cast<uint8_t>((value >> shift) & 0xFFU));
}
uint32_t read_u32(const uint8_t *bytes)
{
    uint32_t value = 0U;
    for (uint32_t shift = 0U; shift < 32U; shift += 8U)
        value |= static_cast<uint32_t>(bytes[shift / 8U]) << shift;
    return value;
}
uint64_t read_u64(const uint8_t *bytes)
{
    uint64_t value = 0U;
    for (uint32_t shift = 0U; shift < 64U; shift += 8U)
        value |= static_cast<uint64_t>(bytes[shift / 8U]) << shift;
    return value;
}
bool checked_size(ft_size_t count, ft_size_t *size_out) noexcept
{
    if (size_out == nullptr || count > WorldCommandCodec::max_commands ||
        count > (std::numeric_limits<ft_size_t>::max() - header_size) / command_size)
        return false;
    *size_out = header_size + count * command_size;
    return true;
}
}

Error WorldCommandCodec::encode(const WorldCommandFrame &frame,
                                std::vector<uint8_t> *bytes_out) noexcept
{
    if (bytes_out == nullptr) return FT_ERR_INVALID_POINTER;
    const ft_size_t count = static_cast<ft_size_t>(frame.commands.size());
    ft_size_t wire_size = 0U;
    if (frame.schema_version != wire_schema_version || !checked_size(count, &wire_size) ||
        count > std::numeric_limits<uint32_t>::max())
        return FT_ERR_INVALID_ARGUMENT;
    try
    {
        bytes_out->clear();
        bytes_out->reserve(static_cast<std::vector<uint8_t>::size_type>(wire_size));
        append_u32(*bytes_out, wire_schema_version);
        append_u64(*bytes_out, frame.tick.value);
        append_u32(*bytes_out, static_cast<uint32_t>(count));
        append_u32(*bytes_out, 0U); // reserved flags, must remain zero for v1
        uint64_t previous_sequence = 0U;
        bool has_previous = false;
        for (const WorldCommand &command : frame.commands)
        {
            if (!command.entity.is_valid() ||
                (has_previous && command.sequence <= previous_sequence))
            {
                bytes_out->clear();
                return FT_ERR_INVALID_ARGUMENT;
            }
            previous_sequence = command.sequence;
            has_previous = true;
            append_u64(*bytes_out, command.entity.value);
            uint64_t encoded_delta = 0U;
            std::memcpy(&encoded_delta, &command.delta, sizeof(encoded_delta));
            append_u64(*bytes_out, encoded_delta);
            append_u64(*bytes_out, command.sequence);
        }
    }
    catch (...)
    {
        bytes_out->clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error WorldCommandCodec::decode(const uint8_t *bytes, ft_size_t byte_count,
                                WorldCommandFrame *frame_out) noexcept
{
    if (bytes == nullptr || frame_out == nullptr) return FT_ERR_INVALID_POINTER;
    frame_out->commands.clear();
    if (byte_count < header_size) return FT_ERR_INVALID_ARGUMENT;
    const uint32_t schema = read_u32(bytes);
    const uint64_t tick = read_u64(bytes + 4U);
    const uint32_t count = read_u32(bytes + 12U);
    if (read_u32(bytes + 16U) != 0U) return FT_ERR_CONFIGURATION;
    ft_size_t expected_size = 0U;
    if (schema != wire_schema_version ||
        !checked_size(static_cast<ft_size_t>(count), &expected_size) ||
        expected_size != byte_count)
        return FT_ERR_CONFIGURATION;
    try
    {
        frame_out->schema_version = wire_schema_version;
        frame_out->tick = SimulationTick{tick};
        frame_out->commands.reserve(static_cast<std::vector<WorldCommand>::size_type>(count));
        uint64_t previous_sequence = 0U;
        bool has_previous = false;
        for (uint32_t index = 0U; index < count; ++index)
        {
            const ft_size_t offset = header_size + static_cast<ft_size_t>(index) * command_size;
            const uint64_t entity = read_u64(bytes + offset);
            const uint64_t encoded_delta = read_u64(bytes + offset + 8U);
            const uint64_t sequence = read_u64(bytes + offset + 16U);
            if (entity == 0U || (has_previous && sequence <= previous_sequence))
            {
                frame_out->commands.clear();
                return FT_ERR_CONFIGURATION;
            }
            int64_t delta = 0;
            std::memcpy(&delta, &encoded_delta, sizeof(delta));
            frame_out->commands.push_back(WorldCommand{EntityId{entity}, delta, sequence});
            previous_sequence = sequence;
            has_previous = true;
        }
    }
    catch (...)
    {
        frame_out->commands.clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}
}
