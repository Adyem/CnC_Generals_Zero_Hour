#include "CncSimulation/SnapshotCodec.hpp"

#include <cstring>
#include <limits>

#include "errno.hpp"

namespace cnc
{
namespace
{
constexpr ft_size_t header_size = 16U;
constexpr ft_size_t entry_size = 17U;

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

bool checked_wire_size(ft_size_t count, ft_size_t *size_out) noexcept
{
    if (size_out == nullptr || count > WorldSnapshotCodec::max_entries) return false;
    if (count > (std::numeric_limits<ft_size_t>::max() - header_size) / entry_size)
        return false;
    *size_out = header_size + count * entry_size;
    return true;
}
}

Error WorldSnapshotCodec::encode(const WorldSnapshot &snapshot,
                                 std::vector<uint8_t> *bytes_out) noexcept
{
    if (bytes_out == nullptr) return FT_ERR_INVALID_POINTER;
    const ft_size_t count = static_cast<ft_size_t>(snapshot.entries.size());
    ft_size_t wire_size = 0U;
    if (snapshot.schema_version != wire_schema_version ||
        !checked_wire_size(count, &wire_size) || count > std::numeric_limits<uint32_t>::max())
        return FT_ERR_INVALID_ARGUMENT;
    try
    {
        bytes_out->clear();
        bytes_out->reserve(static_cast<std::vector<uint8_t>::size_type>(wire_size));
        append_u32(*bytes_out, wire_schema_version);
        append_u64(*bytes_out, snapshot.tick.value);
        append_u32(*bytes_out, static_cast<uint32_t>(count));
        for (const WorldSnapshotEntry &entry : snapshot.entries)
        {
            if (!entry.entity.is_valid() ||
                (entry.alive != FT_TRUE && entry.alive != FT_FALSE))
            {
                bytes_out->clear();
                return FT_ERR_INVALID_ARGUMENT;
            }
            append_u64(*bytes_out, entry.entity.value);
            uint64_t encoded_value = 0U;
            static_assert(sizeof(encoded_value) == sizeof(entry.value), "integer width mismatch");
            std::memcpy(&encoded_value, &entry.value, sizeof(encoded_value));
            append_u64(*bytes_out, encoded_value);
            bytes_out->push_back(entry.alive == FT_TRUE ? 1U : 0U);
        }
    }
    catch (...)
    {
        bytes_out->clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error WorldSnapshotCodec::decode(const uint8_t *bytes, ft_size_t byte_count,
                                 WorldSnapshot *snapshot_out) noexcept
{
    if (bytes == nullptr || snapshot_out == nullptr) return FT_ERR_INVALID_POINTER;
    snapshot_out->entries.clear();
    if (byte_count < header_size) return FT_ERR_INVALID_ARGUMENT;
    const uint32_t schema = read_u32(bytes);
    const uint64_t tick = read_u64(bytes + 4U);
    const uint32_t count = read_u32(bytes + 12U);
    ft_size_t expected_size = 0U;
    if (schema != wire_schema_version || !checked_wire_size(static_cast<ft_size_t>(count), &expected_size) ||
        expected_size != byte_count)
        return FT_ERR_CONFIGURATION;
    try
    {
        snapshot_out->entries.reserve(static_cast<std::vector<WorldSnapshotEntry>::size_type>(count));
        snapshot_out->schema_version = wire_schema_version;
        snapshot_out->tick = SimulationTick{tick};
        for (uint32_t index = 0U; index < count; ++index)
        {
            const ft_size_t offset = header_size + static_cast<ft_size_t>(index) * entry_size;
            const uint64_t entity_value = read_u64(bytes + offset);
            const uint64_t encoded_value = read_u64(bytes + offset + 8U);
            const uint8_t alive = bytes[offset + 16U];
            if (entity_value == 0U || (alive != 0U && alive != 1U))
            {
                snapshot_out->entries.clear();
                return FT_ERR_CONFIGURATION;
            }
            int64_t value = 0;
            std::memcpy(&value, &encoded_value, sizeof(value));
            snapshot_out->entries.push_back(
                WorldSnapshotEntry{EntityId{entity_value}, value, alive == 1U ? FT_TRUE : FT_FALSE});
        }
    }
    catch (...)
    {
        snapshot_out->entries.clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}
}
