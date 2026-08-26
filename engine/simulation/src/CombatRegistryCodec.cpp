#include "CncSimulation/CombatRegistryCodec.hpp"

#include <cstring>
#include <limits>

#include "errno.hpp"

namespace cnc
{
namespace
{
constexpr ft_size_t header_size = 8U;
constexpr ft_size_t entry_size = 25U;
void append_u32(std::vector<uint8_t> &bytes, uint32_t value)
{ for (uint32_t shift = 0U; shift < 32U; shift += 8U) bytes.push_back(static_cast<uint8_t>((value >> shift) & 0xFFU)); }
void append_u64(std::vector<uint8_t> &bytes, uint64_t value)
{ for (uint32_t shift = 0U; shift < 64U; shift += 8U) bytes.push_back(static_cast<uint8_t>((value >> shift) & 0xFFU)); }
uint32_t read_u32(const uint8_t *bytes)
{ uint32_t value = 0U; for (uint32_t shift = 0U; shift < 32U; shift += 8U) value |= static_cast<uint32_t>(bytes[shift / 8U]) << shift; return value; }
uint64_t read_u64(const uint8_t *bytes)
{ uint64_t value = 0U; for (uint32_t shift = 0U; shift < 64U; shift += 8U) value |= static_cast<uint64_t>(bytes[shift / 8U]) << shift; return value; }
bool checked_size(ft_size_t count, ft_size_t *size_out) noexcept
{
    if (size_out == nullptr || count > CombatRegistryCodec::max_entries ||
        count > (std::numeric_limits<ft_size_t>::max() - header_size) / entry_size) return false;
    *size_out = header_size + count * entry_size;
    return true;
}
}

Error CombatRegistryCodec::encode(const CombatRegistrySnapshot &snapshot,
                                  std::vector<uint8_t> *bytes_out) noexcept
{
    if (bytes_out == nullptr) return FT_ERR_INVALID_POINTER;
    const ft_size_t count = static_cast<ft_size_t>(snapshot.health.size());
    ft_size_t wire_size = 0U;
    if (snapshot.schema_version != wire_schema_version || !checked_size(count, &wire_size))
        return FT_ERR_INVALID_ARGUMENT;
    try
    {
        bytes_out->clear();
        bytes_out->reserve(static_cast<std::vector<uint8_t>::size_type>(wire_size));
        append_u32(*bytes_out, wire_schema_version);
        append_u32(*bytes_out, static_cast<uint32_t>(count));
        for (const HealthState &state : snapshot.health)
        {
            if (!state.entity.is_valid() || state.maximum <= 0 || state.current < 0 ||
                state.current > state.maximum || state.alive != (state.current > 0 ? FT_TRUE : FT_FALSE))
            { bytes_out->clear(); return FT_ERR_INVALID_ARGUMENT; }
            append_u64(*bytes_out, state.entity.value);
            append_u64(*bytes_out, static_cast<uint64_t>(state.current));
            append_u64(*bytes_out, static_cast<uint64_t>(state.maximum));
            bytes_out->push_back(state.alive == FT_TRUE ? 1U : 0U);
        }
    }
    catch (...) { bytes_out->clear(); return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

Error CombatRegistryCodec::decode(const uint8_t *bytes, ft_size_t byte_count,
                                  CombatRegistrySnapshot *snapshot_out) noexcept
{
    if (bytes == nullptr || snapshot_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (byte_count < header_size) return FT_ERR_INVALID_ARGUMENT;
    const uint32_t schema = read_u32(bytes);
    const uint32_t count = read_u32(bytes + 4U);
    ft_size_t expected_size = 0U;
    if (schema != wire_schema_version || !checked_size(static_cast<ft_size_t>(count), &expected_size) ||
        expected_size != byte_count) return FT_ERR_CONFIGURATION;
    CombatRegistrySnapshot decoded;
    try
    {
        decoded.schema_version = wire_schema_version;
        decoded.health.reserve(count);
        for (uint32_t index = 0U; index < count; ++index)
        {
            const ft_size_t offset = header_size + static_cast<ft_size_t>(index) * entry_size;
            int64_t current = 0;
            int64_t maximum = 0;
            const uint64_t encoded_current = read_u64(bytes + offset + 8U);
            const uint64_t encoded_maximum = read_u64(bytes + offset + 16U);
            std::memcpy(&current, &encoded_current, sizeof(current));
            std::memcpy(&maximum, &encoded_maximum, sizeof(maximum));
            const uint8_t alive = bytes[offset + 24U];
            if (alive > 1U) return FT_ERR_CONFIGURATION;
            decoded.health.push_back(HealthState{
                EntityId{read_u64(bytes + offset)}, current, maximum,
                alive == 1U ? FT_TRUE : FT_FALSE});
        }
    }
    catch (...) { return FT_ERR_NO_MEMORY; }
    snapshot_out->schema_version = decoded.schema_version;
    snapshot_out->health.swap(decoded.health);
    return FT_ERR_SUCCESS;
}
}
